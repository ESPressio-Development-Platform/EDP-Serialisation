# Public API

**Primary entry point:** `src/ESPressio_Serialisation.hpp`.

The supported surface now includes the compile-time foundation plus JSON + Known-Type Body + Numeric Field encoding operations. Decode, Typed Envelope, LocalisedText, and CBOR remain pending.

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

`MeasurementResult` and `SerialisationResult` are returned by the implemented encoding operations. `DeserialisationResult` remains reserved for the pending decoder.

## Encoding operations

- `Measure<Json>(value)` validates the complete source and returns the exact canonical encoded byte count for Known-Type Body + Numeric Fields.
- `Serialise<Json>(value, output, capacity)` performs exact preflight measurement before writing and returns `BytesWritten == 0` for invalid source, failed adaptation, null output, or insufficient capacity. Its optional compile-time ByteOperations provider parameter defaults to EDP-Platform-Portable and must satisfy the EDP-Memory provider contract.
- The root/Field template parameters default to `KnownTypeBody` / `Numeric`; selecting an unimplemented profile currently fails at compile time rather than falling back silently.

The caller owns source and output storage. The source and any forward canonical adapters must remain observationally stable during the two-pass call.

## System identifier integration

`CanonicalRepresentation` and success-aware `EDP-BoundedTypes::TypeConversionAdapter` specialisations are provided for `System::TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, and `TypeLocalIdentifier`. This integration lives in Serialisation so System does not depend upward on representation concerns.

See the [Reference Index](Reference-Index) for declaration-level detail.
