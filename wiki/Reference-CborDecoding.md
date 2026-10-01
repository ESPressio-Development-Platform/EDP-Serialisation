# Reference — CBOR Decoding

**Source:** `src/serialisation/CborDecoding.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header implements replayable, allocation-free V1 Numeric CBOR validation and population.

## State and outcomes

`CborDecodingStatus` distinguishes malformed/canonical representation failure, parser-resource failure, unknown/duplicate/missing Fields, Type mismatch, integer range failure, non-finite floating values, invalid UTF-8, capacity failure, adaptation failure, and Typed Envelope version/identity failures.

`CborInputCursor` is a non-owning pointer/length/position cursor over immutable caller bytes. `CborHead` retains one parsed major type and canonical argument. `CborSkipState` counts container items visited only while skipping unknown values.

## Header and scalar helpers

`CborHasBytes` checks remaining input. `ReadCborBigEndian` reconstructs unsigned arguments. `ReadCborHead` rejects indefinite/reserved additional information and non-shortest argument widths. `EnterCborContainer` and `AccountSkippedCborItem` enforce `ParserLimits`.

`ReadCborFloating` accepts only the schema-prescribed binary32/binary64 width and rejects non-finite values. `DecodeCborInteger`, `DecodeCborFloating`, `DecodeCborBoundedString`, and `DecodeCborBoundedBytes` enforce exact target category, range/capacity, and UTF-8 rules.

## Containers and schema

`SkipCborValue` structurally validates/skips unknown V1 values without retaining a DOM. `DecodeCborFixedArray` requires the exact compile-time extent. `DecodeCborVector` validates capacity before bounded population.

`DecodeCborSchemaField` dispatches runtime numeric Field identity to compile-time `FieldBinding`. `FinaliseCborSchemaPresence` rejects missing required Fields and resets omitted Optional Fields only during population. `DecodeCborSchema` accepts arbitrary input Field order, rejects duplicate numeric identifiers, and applies Exact/IgnoreUnknown strictness through the fixed 256-bit presence map.

## `DecodeCborValue`

Central recursive dispatcher for the complete V1 serialisable universe. `TPopulate=false` validates without mutation; `TPopulate=true` performs the second replay pass. Reverse strong-Type conversion is validated before population using the already-qualified nothrow temporary rules.

The public operation additionally rejects trailing bytes, giving the same transactional guarantee as JSON without allocating a second destination object.
