# Reference — Results

**Source:** `src/serialisation/Results.hpp`
**Classification:** PUBLIC RESULT API

The header establishes separate result families for the three locked operation families. It deliberately does not use a library-wide status enum.

## `MeasurementStatus`

- `Succeeded` — source validated and exact size measured.
- `InvalidArgument` — caller/profile arguments are invalid.
- `ResourceLimitExceeded` — selected compile-time resource policy is insufficient.
- `InvalidLanguageMetadata` — required textual-profile language context is invalid.
- `FieldNameCollision` — two Fields would emit the same textual key.
- `LocalisationFailure` — Field presentation could not be resolved conclusively.
- `NonFiniteNumber` — NaN/infinity lies outside V1.
- `InvalidUtf8` — bounded String payload is not permitted UTF-8 text.
- `AdaptationFailed` — strong-Type conversion to its canonical surrogate failed.

## `SerialisationStatus`

- `Succeeded` — complete representation emitted.
- `InvalidArgument` — caller/profile arguments are invalid.
- `OutputBufferTooSmall` — caller output span is smaller than exact `RequiredBytes`; locked semantics require no partial message.
- `ResourceLimitExceeded` — selected compile-time resource policy is insufficient.
- `InvalidLanguageMetadata`, `FieldNameCollision`, `LocalisationFailure`, `NonFiniteNumber`, `InvalidUtf8`, `AdaptationFailed` — same source-validation domains described above, scoped to emission.

## `DeserialisationStatus`

- `Succeeded` — complete input validated and destination populated transactionally.
- `InvalidArgument` — caller/profile arguments are invalid.
- `MalformedRepresentation` — bytes are not structurally valid selected-codec input.
- `TrailingData` — data remains after one complete root value (ignoring permitted JSON trailing whitespace).
- `ResourceLimitExceeded` — parser resource policy exceeded.
- `UnsupportedEnvelopeVersion` — typed-envelope version is unknown.
- `TypeIdentifierMismatch` — typed-envelope identity differs from the compile-time target.
- `UnknownField` — exact policy encountered a valid but unknown Field.
- `DuplicateField` — same logical key/Field appears more than once.
- `MissingRequiredField` — non-Optional schema Field absent.
- `InvalidLanguageMetadata` — RFC5646 context absent/malformed/inconsistent where required.
- `FieldNameNotFound` — textual name conclusively maps to no Field.
- `FieldNameAmbiguous` — textual name maps to distinct Field identifiers.
- `LocalisationFailure` — Localisation proof could not be completed.
- `TypeMismatch` — encoded category does not match target schema Type.
- `NumericOutOfRange` — numeric value cannot fit the target fixed-width domain.
- `NumericUnderflow` — non-zero text float underflows completely to zero.
- `NonFiniteNumber` — NaN/infinity is outside V1.
- `InvalidUtf8` — decoded text violates V1 UTF-8/String constraints.
- `InvalidBase64` — JSON byte text is not strict canonical padded RFC 4648 Base64.
- `CapacityExceeded` — decoded bounded value exceeds compile-time capacity.
- `AdaptationFailed` — canonical surrogate cannot convert into the strong destination Type.

## `Diagnostic`

Fixed-size common context value; it owns no path string/tree and performs no allocation.

- `ByteOffset` — byte offset most directly associated with the result.
- `Type` — optional immediate semantic `System::TypeIdentifier` when known.
- `Field` — optional immediate `System::FieldIdentifier` when known.

## `MeasurementResult`

- `Status` — `MeasurementStatus` operation state.
- `RequiredBytes` — exact encoded size on success.
- `Detail` — bounded diagnostic context.
- `IsSuccessful()` — pure predicate, true only for `Succeeded`; no state mutation.

## `SerialisationResult`

- `Status` — `SerialisationStatus` operation state.
- `RequiredBytes` — exact complete representation size, including when output is too small.
- `BytesWritten` — committed output byte count; locked contract requires zero on failure.
- `Detail` — bounded diagnostic context.
- `IsSuccessful()` — pure predicate for `Succeeded`.

## `DeserialisationResult`

- `Status` — `DeserialisationStatus` operation state.
- `BytesConsumed` — complete validated root input bytes on success.
- `Detail` — bounded diagnostic context.
- `IsSuccessful()` — pure predicate for `Succeeded`.

These Types are the stable operation-specific vocabulary returned by the implemented JSON Numeric `Measure`, `Serialise`, and `Deserialise` operations. `UnsupportedEnvelopeVersion` and `TypeIdentifierMismatch` are now active Typed Envelope outcomes; LocalisedText/later-codec statuses remain reserved until those slices are implemented.
