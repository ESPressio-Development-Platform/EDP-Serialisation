# Public API

**Primary entry point:** `src/ESPressio_Serialisation.hpp`.

The current supported consumer/extension surface is compile-time only; JSON/CBOR operation functions are not yet implemented.

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

These operation-specific result families are implemented vocabulary for later codec functions and do not themselves claim those functions are present yet.

## System identifier integration

`CanonicalRepresentation` and success-aware `EDP-BoundedTypes::TypeConversionAdapter` specialisations are provided for `System::TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, and `TypeLocalIdentifier`. This integration lives in Serialisation so System does not depend upward on representation concerns.

See the [Reference Index](Reference-Index) for declaration-level detail.
