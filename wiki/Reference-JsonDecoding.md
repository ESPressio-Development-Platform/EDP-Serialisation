# Reference — JSON Decoding

**Source:** `src/serialisation/JsonDecoding.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header implements the allocation-free parser shared by Numeric and LocalisedText JSON `Deserialise` operations for Known-Type Body and Typed Envelope roots. No declaration in this file is a cross-repository compatibility surface; consumers use `Operations.hpp` through the repository umbrella.

## Outcome and parser state

### `JsonDecodingStatus`

Internal strongly typed outcome family. Values are: `Succeeded` (complete internal operation succeeded), `MalformedRepresentation` (invalid JSON grammar), `ResourceLimitExceeded` (compile-time parser limit exceeded), `UnknownField`, `DuplicateField`, `MissingRequiredField`, `TypeMismatch`, `NumericOutOfRange`, `NumericUnderflow`, `NonFiniteNumber`, `InvalidUtf8`, `InvalidBase64`, `CapacityExceeded`, `AdaptationFailed`, `UnsupportedEnvelopeVersion`, `TypeIdentifierMismatch`, `InvalidLanguageMetadata`, `FieldNameNotFound`, `FieldNameAmbiguous`, and `LocalisationFailure`. `Operations.hpp` maps these into the public `DeserialisationStatus` family.

### `JsonInputCursor`

Immutable pointer/length/position view over caller-owned input. Members `_input`, `_length`, and `_position` retain no ownership. Constructor binds the range. `IsAtEnd`, `Position`, `Length`, `Data`, and `Current` inspect state; `Advance()` / `Advance(count)` move only within already validated parser logic; `Seek(position)` supports bounded replay used by duplicate-key comparison.

### `JsonNumberToken`

Fixed-size description of one syntactically validated JSON number. `Start`/`End` delimit the complete token; integer/fraction offsets delimit mantissa digits; sign/exponent members retain mathematical layout without copying text. `IntegerDigits`, `FractionDigits`, and `MantissaDigits` derive counts. Exponent magnitude saturates through `IsExponentMagnitudeOverflow` rather than allocating arbitrary precision state.

### `JsonSkipState`

Retains only `ContainerItems`, the count of object/array items traversed solely because an unknown Field is being skipped. This state enforces `ParserLimits::MaximumSkippedContainerItems` and is reset for each validation/population traversal.

### `JsonDecodedStringReader`

Fixed-state incremental reader used to compare arbitrary unknown-object member names by their decoded Unicode/UTF-8 bytes. `Cursor`, `Started`, `Finished`, `Pending`, `PendingCount`, and `PendingIndex` permit exact escaped/raw equivalence checks with at most four pending UTF-8 bytes and no key dictionary.

## Lexical and String helpers

- `IsJsonWhitespace` — recognizes exactly JSON space, tab, line-feed and carriage-return.
- `SkipJsonWhitespace` — advances over contiguous JSON whitespace.
- `ConsumeJsonByte` — consumes one required syntax byte or returns malformed representation.
- `ConsumeJsonLiteral` — consumes exact lowercase `true`, `false`, or `null`-style literal text.
- `TryHexValue` — converts one hexadecimal escape digit.
- `ParseJsonHexQuad` — parses exactly four `\\u` hexadecimal digits.
- `EmitUtf8Scalar<TConsumer>` — canonical UTF-8 expansion of one validated Unicode scalar, optionally rejecting U+0000.
- `ParseJsonString<TConsumer>` — parses JSON String syntax, raw UTF-8, simple escapes, and surrogate pairs while streaming decoded UTF-8 bytes to a caller-supplied consumer. It retains no decoded String buffer.
- `QueueDecodedStringScalar` / `NextDecodedJsonStringByte` — incremental scalar/byte machinery for exact key comparison.
- `AreDecodedJsonStringsEqual` — compares two source String spellings by decoded UTF-8 sequence, so escaped-equivalent names compare equal.
- `ParseJsonNumericFieldKey` — accepts only quoted canonical decimal FieldIdentifier keys `"0".."255"`, rejecting leading-zero or non-decimal alternatives.

## Numeric helpers

- `ParseJsonNumberToken` — validates RFC 8259 number grammar and records token layout without converting through floating point.
- `JsonMantissaDigitAt` — reads one logical mantissa digit across integer/fraction spans.
- `ConvertJsonIntegerToken<TValue>` — evaluates exact integral mathematical value from decimal digits/exponent directly into a fixed-width target; rejects non-integral, signedness and range violations without `double` conversion.
- `IsJsonNumberZero` — proves whether every mantissa digit is zero.
- `JsonScientificExponent` — derives a bounded decimal scientific exponent used only to classify floating out-of-range as overflow versus underflow.
- `ConvertJsonFloatingToken<TValue>` — uses C++20 `std::from_chars` for correctly rounded binary32/binary64 after grammar validation; preserves signed zero and distinguishes overflow/non-zero-underflow.

## Parser-limit and unknown-value helpers

- `EnterJsonContainer<TParserLimits>` — checks syntactic nesting before entering an array/object.
- `AccountSkippedContainerItem<TParserLimits>` — bounds work spent only on unknown values.
- `SkipJsonValue<TParserLimits>` — recursively validates and skips arbitrary JSON values without retaining them.
- `SkipJsonArray<TParserLimits>` — validates/skips one unknown array while accounting every element.
- `SkipJsonObject<TParserLimits>` — validates/skips one unknown object and rejects duplicate decoded member names.
- `HasEarlierEquivalentJsonObjectKey<TParserLimits>` — replay-scans earlier members to test a current key exactly. It intentionally trades bounded CPU for RAM instead of storing an unbounded/nesting-proportional name table.

Unknown-object key replay can recursively validate earlier values. This is deliberate: memory usage remains fixed/bounded while CPU is limited by caller input and `ParserLimits`.

## Bounded text/byte helpers

- `DecodeJsonBoundedString<TPopulate,TValue>` — streams decoded UTF-8, enforces compile-time String capacity, and populates only during pass two.
- `Base64Value` — maps the standard RFC 4648 alphabet to six-bit values; all other bytes are invalid.
- `DecodeJsonBoundedBytes<TPopulate,TValue>` — validates strict padded canonical Base64, zero pad bits, final-quartet placement and decoded capacity; pass two appends decoded octets into bounded Bytes.

## Recursive value/container helpers

- `DecodeJsonValueWithFieldPolicy<TPopulate,TStrictness,TParserLimits,TFieldPolicy,TValue>` — central recursive dispatcher for bool, integer, floating, certified enum, Optional, fixed arrays, bounded String/Bytes/Vector, SchemaType and canonical strong adaptation. `TPopulate=false` performs validation only; `TPopulate=true` performs the replayed population pass.
- `JsonNumericFieldDecodingPolicy` is the zero-state Numeric schema-key policy. `DecodeJsonValue<TPopulate,...>` is a Numeric wrapper which constructs that policy and delegates to the policy-aware dispatcher. `JsonLocalisedText.hpp` supplies a LocalisedText schema decoder overload selected by policy type.
- `DecodeJsonFixedArray` — requires exactly the compile-time fixed extent and propagates the active Field policy to nested values.
- `DecodeJsonVector` — accepts up to `BoundedVectorTraits<T>::Capacity`; population clears only after pass-one validation and activates inline slots with `EmplaceBack`.
- `IsFieldSeen` / `MarkFieldSeen` — operate on the fixed 32-byte presence bitmap covering all 256 possible numeric FieldIdentifiers.
- `DecodeJsonSchemaField` — runtime-ID to compile-time `FieldBinding` dispatch while propagating the active Field policy into the Field value. Optional null/absence semantics and Field-local diagnostics are handled here.
- `FinaliseJsonSchemaPresence` — rejects missing required Fields and resets absent Optional Fields only during population.
- `DecodeJsonSchemaWithFieldPolicy` for `JsonNumericFieldDecodingPolicy` validates Numeric JSON objects in arbitrary member order, detects duplicate numeric IDs, applies Exact/IgnoreUnknown strictness, and invokes required-presence validation. LocalisedText provides its own overload over the same Field dispatch/presence helpers.

## Transactional invariant

The public operation runs the selected policy-aware decoder with `TPopulate=false` over the complete input before replaying it with `TPopulate=true`. The validation pass may read existing destination values only to seed a nothrow-copyable non-default strong semantic temporary; it must not mutate caller state. Population relies on the same immutable input and deterministic reverse adapters. Ordinary parser/range/capacity/adaptation failures are therefore discovered before destination mutation begins.

## Memory and concurrency

All retained parser state is caller input, fixed-size stack state, compile-time capacities, the 32-byte schema presence bitmap, or bounded destination storage. No allocation, registry, DOM, mutex, global state, or background lifecycle exists. Calls are reentrant for independent caller-owned inputs/destinations; no ISR-safety guarantee is made for `std::from_chars` or arbitrary user adapters.
