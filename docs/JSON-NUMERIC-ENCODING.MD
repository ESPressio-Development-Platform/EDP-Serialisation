# JSON Numeric Known-Type Encoding

This document defines the currently implemented V1 encoding execution surface: `Json` codec, `RootProfile::KnownTypeBody`, and `FieldProfile::Numeric`.

`Measure` and `Serialise` are implemented for this profile. JSON deserialisation, Typed Envelope execution, LocalisedText Fields, and CBOR remain later implementation slices and are not implied by this document.

## Public operations

```cpp
const auto measurement = ESPressio::Serialisation::Measure<
    ESPressio::Serialisation::Json
>(value);

const auto result = ESPressio::Serialisation::Serialise<
    ESPressio::Serialisation::Json
>(
    value,
    output,
    capacity
);
```

The root and Field profile template parameters default to `KnownTypeBody` and `Numeric`. Selecting an unimplemented codec/profile combination currently fails at compile time with a focused diagnostic rather than silently falling back to another representation.

`Measure` performs complete source validation and returns the exact representation size only on success. `Serialise` runs that same exact measurement before writing. Invalid source data, failed canonical adaptation, null output, or insufficient output capacity therefore produces `BytesWritten == 0`; capacity failure also reports the exact `RequiredBytes`.

The source value must not be concurrently mutated during a call. A canonical forward `TypeConversionAdapter` used by encoding must be observationally stable for an unchanged source because measurement and emission deliberately traverse the value separately to avoid retaining a second encoded object or hidden scratch buffer.

## Canonical JSON values

- `bool` emits `true` or `false`.
- fixed-width signed and unsigned integers emit canonical base-10 text with no leading `+` or unnecessary zeroes;
- certified enums emit their fixed-width numeric domain;
- finite binary32/binary64 values use C++20 `std::to_chars` shortest round-tripping decimal, with explicit canonical spellings `0` and `-0`;
- NaN and infinities fail before output;
- disengaged root/sequence `Optional<T>` emits `null`; engaged Optional emits its value;
- `std::array`, native fixed arrays, and `Bounded::Vector` emit JSON arrays;
- nested `SchemaType` values emit JSON objects;
- approved strong semantic Types first convert to their one direct canonical representation through `EDP-BoundedTypes::TypeConversionAdapter`.

## Numeric schema Fields

JSON object keys are strings by JSON syntax, but the semantic key is always the one-byte `System::FieldIdentifier`. Numeric Field keys therefore emit as canonical decimal strings from `"0"` through `"255"`.

Schema Fields always emit in ascending numeric `FieldIdentifier` order, independently of `FieldSet` declaration order. The implementation resolves bindings at compile time and does not retain a runtime schema registry or sort table.

Disengaged Optional object Fields are omitted entirely. Engaged Optional Fields occupy their normal numeric position and encode the contained value.

## String and UTF-8 rules

`Bounded::String<N>` is validated as complete UTF-8 before encoding. The encoder rejects malformed continuation sequences, overlong forms, UTF-16 surrogate code points, code points above U+10FFFF, incomplete sequences, and U+0000.

Canonical JSON output leaves valid non-ASCII UTF-8 bytes native. `"` and `\\` are escaped, the standard short escapes are used for backspace/tab/newline/form-feed/carriage-return, and other ASCII control bytes use lowercase `\\u00xx` spelling. No Unicode normalization or transliteration occurs.

## Bytes/Base64

Only `Bounded::Bytes<N>` uses the byte-string representation. JSON emits RFC 4648 standard Base64 with alphabet `A-Z a-z 0-9 + /`, mandatory `=` padding, no whitespace or wrapper prefix, and an empty byte sequence as `""`.

`std::array<std::uint8_t,N>` and `Bounded::Vector<std::uint8_t,N>` remain ordinary numeric JSON arrays.

## Strong semantic adaptation storage

Forward encoding must create a bounded temporary canonical surrogate which the existing pairwise `TypeConversionAdapter` can populate. Therefore a strong Type qualifies only when its canonical surrogate is nothrow default-constructible in addition to the already-established one-hop, bidirectional, success-aware, and noexcept conversion requirements.

This does not introduce another construction/conversion abstraction and does not add dynamic allocation.

## Resource behavior

The implementation has no DOM, no runtime Type/Field registry, no dynamic allocation, no hidden ownership, and no mutable global state. Measurement uses a zero-storage counting sink. Serialisation writes directly into caller-owned contiguous bytes only after successful preflight measurement. Byte-range writes reuse the EDP-Memory `ByteOperations` contract; the default is the stateless `EDP-Platform-Portable` provider and callers may substitute another conforming stateless provider through the `Serialise` template parameter.

The floating `std::to_chars` path has a material flash footprint on the current ESP32 GCC/libstdc++ toolchain because it pulls the toolchain Ryu tables. AI-AGENT-02 measurement of the small demonstration showed roughly 122-126 KiB additional flash when the first float encoding was instantiated, while RAM remained unchanged. This is recorded as implementation-size optimization issue `EDP-Serialisation#2`; it is not retained runtime state.

## Current non-scope

This checkpoint does not implement:

- `RootProfile::TypedEnvelope` execution;
- `FieldProfile::LocalisedText` execution;
- CBOR encoding or decoding;
- streaming/non-replayable input;
- dynamic object construction or codec autodetection.
