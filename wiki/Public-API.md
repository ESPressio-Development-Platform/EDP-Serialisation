# Public API

**Primary entry point:** `src/ESPressio_Serialisation.hpp`.

The supported surface now includes the compile-time foundation plus JSON + Known-Type Body + Numeric Field encoding and transactional decoding operations. Typed Envelope, LocalisedText, and CBOR remain pending.

## Type qualification and extension

- `SerialisableType<T>` / `IsSerialisableType<T>` — canonical recursive V1 qualification.
- `CanonicalRepresentation<T>` — PUBLIC EXTENSION API naming one direct serialisable surrogate for a strong semantic Type. It deliberately owns no conversion behavior.
- `EnumSerialisationTraits<TEnum>` — PUBLIC EXTENSION API certifying an enum's stable fixed-width numeric domain.
- `Optional<T>` — exact zero-overhead alias of `std::optional<T>`.

## Profiles and limits

- `Json`, `Cbor` — compile-time codec tags reserved for codec operation templates.
- `RootProfile` — `KnownTypeBody` or `TypedEnvelope`.
- `FieldProfile` — `Numeric` or `LocalisedText`.
- `StrictnessPolicy` — exact or explicit unknown-Field skipping policy.
- `TypedEnvelopeVersion` — V1 typed-envelope representation version (`1`).
- `ParserLimits<>` / `DefaultParserLimits` — compile-time syntactic nesting and unknown-value skip bounds.

## Result vocabulary

- `MeasurementStatus` / `MeasurementResult`.
- `SerialisationStatus` / `SerialisationResult`.
- `DeserialisationStatus` / `DeserialisationResult`.
- `Diagnostic` — bounded common byte-offset/Type/Field context; it owns no dynamic path tree.

`MeasurementResult` and `SerialisationResult` are returned by the implemented encoding operations. `DeserialisationResult` is returned by transactional JSON decoding.

## Encoding operations

- `Measure<Json>(value)` validates the complete source and returns the exact canonical encoded byte count for Known-Type Body + Numeric Fields.
- `Serialise<Json>(value, output, capacity)` performs exact preflight measurement before writing and returns `BytesWritten == 0` for invalid source, failed adaptation, null output, or insufficient capacity. Its optional compile-time ByteOperations provider parameter defaults to EDP-Platform-Portable and must satisfy the EDP-Memory provider contract.
- The root/Field template parameters default to `KnownTypeBody` / `Numeric`; selecting an unimplemented profile currently fails at compile time rather than falling back silently.

The caller owns source/output/input/destination storage. The source and forward canonical adapters must remain observationally stable during encoding. The immutable input and reverse adapters must remain deterministic during deserialisation replay.

## Decoding operation

- `Deserialise<Json, KnownTypeBody, Numeric, Strictness, ParserLimits>(input, length, destination)` performs complete validate-only pass one, rejects trailing non-whitespace, then replays the same immutable bytes to populate. `StrictnessPolicy::Exact` rejects unknown schema Fields; `IgnoreUnknownFields` structurally validates/skips them within the compile-time parser limits.
- Successful decode reports the complete input length in `BytesConsumed`; failure reports zero consumed bytes and preserves the destination for all validated failure modes.
- String/Bytes/sequence capacities, duplicate/missing Field rules, numeric ranges, UTF-8/Base64 and reverse canonical adapters are validated before population.

## System identifier integration

`CanonicalRepresentation` and success-aware `EDP-BoundedTypes::TypeConversionAdapter` specialisations are provided for `System::TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, and `TypeLocalIdentifier`. This integration lives in Serialisation so System does not depend upward on representation concerns.

See the [Reference Index](Reference-Index) for declaration-level detail.
