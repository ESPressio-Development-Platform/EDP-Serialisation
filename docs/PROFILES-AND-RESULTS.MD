# Profiles, Limits and Result Vocabulary

The compile-time vocabulary is now consumed by JSON + Numeric Fields encoding and transactional decoding for both Known-Type Body and Typed Envelope roots. LocalisedText and CBOR execution remain pending.

## Codec and profile selection

`Json` and `Cbor` are compile-time codec tags. `RootProfile` selects `KnownTypeBody` or `TypedEnvelope`; `FieldProfile` selects numeric or localised-text Field keys. `StrictnessPolicy` selects exact schema matching or explicit unknown-Field skipping.

`TypedEnvelopeVersion` is currently `1` and versions only the typed-envelope representation, not a library release or application schema.

## Parser limits

`ParserLimits<MaximumNestingDepth, MaximumSkippedContainerItems>` is a zero-state compile-time policy. The default is 32 levels and 1024 skipped/unknown container items. Type-owned capacities remain separate and authoritative for bounded String/Bytes/Vector values.

## Operation-specific results

V2 code-convention policy deliberately prohibits one library-wide result enum. `MeasurementStatus`, `SerialisationStatus`, and `DeserialisationStatus` therefore describe only their coherent operation families. Their result structs expose relevant byte counts and a compact shared `Diagnostic` containing byte offset and optional immediate Type/Field identity.

`MeasurementResult` and `SerialisationResult` are returned by the implemented JSON Numeric encoding operations for Known-Type Body and Typed Envelope roots. `DeserialisationResult` is returned by the transactional decoder; success reports the complete consumed input length, while failure reports zero consumed bytes plus bounded diagnostic context. Typed Envelope failures use the existing `UnsupportedEnvelopeVersion` and `TypeIdentifierMismatch` statuses.
