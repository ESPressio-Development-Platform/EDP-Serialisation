# Public API

**Primary entry point:** `src/ESPressio_Serialisation.hpp`.

The supported surface now includes the compile-time foundation plus JSON Numeric and LocalisedText encoding/transactional decoding for both Known-Type Body and Typed Envelope roots. CBOR remains pending.

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

- `Measure<Json>(value)` validates the complete source and returns the exact canonical encoded byte count; `RootProfile::TypedEnvelope` adds deterministic root metadata around the same Numeric body.
- `Serialise<Json>(value, output, capacity)` performs exact preflight measurement before writing and returns `BytesWritten == 0` for invalid source, failed adaptation, null output, or insufficient capacity. Its optional compile-time ByteOperations provider parameter defaults to EDP-Platform-Portable and must satisfy the EDP-Memory provider contract.
- The root/Field template parameters default to `KnownTypeBody` / `Numeric`. `TypedEnvelope` is implemented for identified roots. Dedicated LocalisedText overloads accept a caller-owned EDP-Localisation Resolver/context plus scratch; CBOR selections still fail at compile time rather than falling back silently.

The caller owns source/output/input/destination storage. The source and forward canonical adapters must remain observationally stable during encoding. The immutable input and reverse adapters must remain deterministic during deserialisation replay.

## Decoding operation

- `Deserialise<Json, KnownTypeBody, Numeric, Strictness, ParserLimits>(...)` performs complete validate-only pass one, rejects trailing non-whitespace, then replays the same immutable bytes to populate. `StrictnessPolicy::Exact` rejects unknown schema Fields; `IgnoreUnknownFields` structurally validates/skips them within the compile-time parser limits.
- `Deserialise<Json, TypedEnvelope, Numeric, ...>(...)` additionally requires an identified root and validates envelope version plus exact root TypeIdentifier before the population pass. Unknown versions return `UnsupportedEnvelopeVersion`; identity mismatch returns `TypeIdentifierMismatch`.
- Successful decode reports the complete input length in `BytesConsumed`; failure reports zero consumed bytes and preserves the destination for all validated failure modes. String/Bytes/sequence capacities, duplicate/missing Field rules, numeric ranges, UTF-8/Base64 and reverse canonical adapters are validated before population.

## LocalisedText operations

- `Measure<Json, Root, LocalisedText>(value, resolver, context, fieldScratch, comparisonScratch)` validates the Localisation context, resolves every emitted Field name, proves exact textual-key uniqueness/reserved-name safety, and returns exact canonical JSON size.
- `Serialise<Json, Root, LocalisedText>(...)` performs that exact preflight before writing, emits `RFC5646` first in every schema object, then emits resolved names in canonical numeric FieldIdentifier order.
- `Deserialise<Json, Root, LocalisedText>(input, length, destination, resolver, context, fieldScratch)` uses embedded RFC5646 when present and requires it to equal `context.RequestedLanguage`; otherwise it uses the caller context.
- The overload taking `terminalLanguage` instead of a full context permits payload-only language metadata and, when metadata is absent, calls complete all-language reverse resolution.
- Caller scratch is bounded and non-owning. Encoding needs two independent scratch views for collision proof; decoding needs one. Scratch exhaustion is a resource-limit failure, never hidden allocation.

Only conclusive `FieldNameNotFound` is skippable under `IgnoreUnknownFields`; ambiguity or Localisation provider/pack/dataset failures remain hard failures.

## System identifier integration

`CanonicalRepresentation` and success-aware `EDP-BoundedTypes::TypeConversionAdapter` specialisations are provided for `System::TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, and `TypeLocalIdentifier`. This integration lives in Serialisation so System does not depend upward on representation concerns.

See the [Reference Index](Reference-Index) for declaration-level detail.
