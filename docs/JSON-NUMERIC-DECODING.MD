# JSON Numeric Known-Type Decoding

This document defines V1 `Deserialise` for `Json`, `RootProfile::KnownTypeBody`, and `FieldProfile::Numeric`.

The decoder consumes one complete caller-owned contiguous JSON byte range and populates an existing compile-time target Type. It performs no dynamic allocation, constructs no DOM, and performs no runtime schema lookup. The same body decoder is now reused by the implemented Typed Envelope root profile. LocalisedText Fields, CBOR, and non-replayable streaming input remain later implementation slices.

## Public operation

```cpp
const auto result = ESPressio::Serialisation::Deserialise<
    ESPressio::Serialisation::Json
>(
    input,
    length,
    destination
);
```

The root/Field profiles default to `KnownTypeBody` / `Numeric`; strictness defaults to `StrictnessPolicy::Exact`; parser resources default to `DefaultParserLimits`. Unsupported compile-time codec/profile combinations fail with focused `static_assert` diagnostics.

`input` must be non-null. JSON whitespace may precede/follow the root value, but any other trailing byte produces `TrailingData`. `BytesConsumed` equals the supplied input length only on success and remains zero on failure.

## Transactional destination guarantee

Deserialisation is deliberately replayed in two passes:

1. validate the complete input against the target Type, capacities, parser limits and reverse adapters without mutating caller state;
2. replay the same immutable bytes and populate the destination only after validation succeeds.

Every ordinary validation failure therefore leaves the destination observably unchanged. Reverse `TypeConversionAdapter` implementations participating in canonical adaptation must be deterministic for an unchanged surrogate during one call: validation proves the adapter outcome before the population pass is allowed to mutate caller state.

No second full object, DOM or dynamic scratch tree is retained. Validation spends bounded CPU/replay work instead of proportional RAM.

## Schema objects and strictness

Numeric JSON schema keys are canonical decimal strings `"0"` through `"255"`. Input Field order is arbitrary; identity comes solely from `System::FieldIdentifier`.

A fixed 256-bit bitmap tracks schema Field presence and duplicates. Duplicate known or unknown numeric Field keys always fail. Required non-Optional Fields must be present. Optional object Fields may be absent or explicit `null`, both producing a disengaged destination Optional.

`StrictnessPolicy::Exact` rejects a structurally valid unknown Field with `UnknownField`. `IgnoreUnknownFields` validates and skips the complete unknown JSON value. Skipped objects also reject duplicate textual JSON member names, including escaped-equivalent spellings such as `"a"` and `"\\u0061"`; this is implemented by replay comparison rather than an unbounded key dictionary.

## Numeric conversion

- `bool` accepts only JSON `true`/`false`.
- integer/enum targets accept any syntactically valid JSON number whose mathematical value is exactly integral and in range; conversion never passes through `double`.
- signedness/range failures produce `NumericOutOfRange`; non-integral numeric input produces `TypeMismatch`.
- enum decode accepts every representable certified underlying value; named-enumerator membership remains application validation.
- `float`/`double` use C++20 `std::from_chars` for correctly rounded binary32/binary64 conversion after JSON grammar validation.
- overflowing finite numeric text fails with `NumericOutOfRange`; non-zero input which underflows completely to zero fails with `NumericUnderflow`; representable subnormals remain accepted.
- mathematical zero remains zero for arbitrarily large explicit exponents, and negative zero preserves its sign.

JSON itself has no NaN/Infinity tokens, so such spellings are malformed/type-incompatible rather than an alternate floating representation.

## String and Bytes decoding

`Bounded::String<N>` decodes JSON escapes and raw UTF-8 into canonical UTF-8 bytes. The decoder rejects malformed UTF-8, overlong sequences, unpaired surrogate escapes, values above U+10FFFF, U+0000, and capacity overflow. No Unicode normalization occurs.

`Bounded::Bytes<N>` accepts only strict standard padded RFC 4648 Base64 after JSON String escape decoding. Missing/excess padding, Base64URL characters, invalid lengths/characters, non-zero unused pad bits, data after a padded final quartet, or decoded capacity overflow fail. `std::array<uint8_t,N>` and `Bounded::Vector<uint8_t,N>` remain numeric JSON arrays rather than Base64 values.

## Sequences and Optional

`std::array<T,N>` and native `T[N]` require exactly `N` JSON elements. `Bounded::Vector<T,N>` accepts `0..N` elements and never truncates. Positional/root disengaged Optional uses JSON `null`.

Because population must create new logical values without hidden ownership, V1 qualification requires Optional elements and Bounded Vector elements to be nothrow default-constructible. Strong semantic Types used through `CanonicalRepresentation` must be either nothrow default-constructible or nothrow copy-constructible so reverse-adapter validation has temporary semantic storage; canonical surrogates remain nothrow default-constructible.

## Parser resource limits

`ParserLimits<MaximumNestingDepth, MaximumSkippedContainerItems>` bounds syntactic container nesting and the number of array/object items traversed solely while skipping unknown values. Known Type-owned String/Bytes/Vector capacities remain independently authoritative.

Unknown-object duplicate detection intentionally retains no dynamic key table. It replays earlier members and compares their decoded key bytes exactly, spending bounded CPU rather than nesting-proportional RAM.

## Current non-scope

This body decoder does not implement LocalisedText Field presentation/resolution, CBOR, streaming/non-replayable input, runtime object construction, or codec autodetection. Typed Envelope execution is implemented as a root wrapper which delegates its `value` member to this decoder.
