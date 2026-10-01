# EDP-Serialisation

Deterministic, allocation-free, schema-driven serialisation for the ESPressio Development Platform.

The library is currently in its pre-1.0 implementation tranche at version `0.1.0`. The compile-time foundation is implemented, together with the first executable codec profile: exact `Measure` and all-or-nothing `Serialise` for JSON + Known-Type Body + Numeric Fields. JSON decoding, Typed Envelope execution, LocalisedText Fields, and CBOR remain later implementation slices.

## Core Guarantees

- C++20 only.
- No dynamic allocation, runtime schema registry, RTTI, DOM, mutable global state, or hidden ownership.
- `EDP-System` remains authoritative for `TypeIdentifier`, `FieldIdentifier`, `FieldBinding`, `FieldSet`, and `SchemaType`.
- `EDP-BoundedTypes` remains authoritative for bounded values/containers and pairwise `TypeConversionAdapter` conversion.
- Strong semantic adaptation names one direct canonical representation through `CanonicalRepresentation<T>` and requires noexcept, success-aware conversion adapters in both directions.
- Enums require explicit `EnumSerialisationTraits<TEnum>` certification of their fixed-width underlying Type.
- Nested schema fields are qualified recursively through the existing System FieldSet traversal contract.
- Dynamic STL owners, pointers, views, long double, maps/sets, tuples and variants remain outside the V1 serialisable universe unless separately designed.

## Documentation

- [V1 Type Qualification](./docs/TYPE-QUALIFICATION.MD) — defines the recursive Type universe, enum certification, schema recursion, bounded containers, and strong semantic adaptation.
- [Profiles, Limits and Results](./docs/PROFILES-AND-RESULTS.MD) — defines compile-time codec/profile vocabulary, parser resource policy, and operation-specific result families.
- [JSON Numeric Known-Type Encoding](./docs/JSON-NUMERIC-ENCODING.MD) — defines the implemented `Measure`/`Serialise` profile, canonical wire rules, no-partial-output guarantee, and resource behavior.
- [Dependency Graph](./DEPENDENCY_GRAPH.MD) — records direct ownership boundaries and current transitive dependency topology.

## Implemented API

```cpp
#include <ESPressio_Serialisation.hpp>

static_assert(ESPressio::Serialisation::SerialisableType<std::uint32_t>);
static_assert(ESPressio::Serialisation::SerialisableType<
    ESPressio::Bounded::String<32U>
>);
```

Public foundation components:

- `SerialisableType<T>` / `IsSerialisableType<T>`;
- `CanonicalRepresentation<T>`;
- `EnumSerialisationTraits<TEnum>`;
- `Optional<T>`;
- `Json` / `Cbor`, `RootProfile`, `FieldProfile`, `StrictnessPolicy`;
- `ParserLimits<>` / `DefaultParserLimits`;
- operation-specific `MeasurementResult`, `SerialisationResult`, and `DeserialisationResult` families;
- canonical adapters for `System::TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, and `TypeLocalIdentifier`;
- `Measure<Json>(value)` for exact canonical JSON size/validation;
- `Serialise<Json>(value, output, capacity)` for JSON + Known-Type Body + Numeric Fields.

## Fixed-width integer note

The V1 contract is defined in terms of the fixed-width C++ Types `std::int8_t` through `std::uint64_t`. C++20 does not retain typedef spelling: when a fixed-width typedef is the exact same C++ Type as a native spelling or another alias such as `std::size_t`, no concept can distinguish which spelling appeared in source. The implementation therefore qualifies the actual fixed-width Type identity available on the target. Developers must use the fixed-width spelling for portable schema intent; the compiler cannot prove that spelling after alias substitution. This limitation never permits native object-layout or host-endian wire encoding.

## Dependencies

Direct dependencies are `EDP-System`, `EDP-BoundedTypes`, `EDP-Memory`, and `EDP-Platform-Portable`. The latter two own the ByteOperations contract and default stateless portable provider used by caller-buffer writes. See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD).

## Demos

`demos/type-qualification` demonstrates schema recursion, bounded values, enum certification, profile vocabulary, and compile-time qualification. `demos/json-numeric-encoding` demonstrates canonical Numeric Field ordering, Optional omission, floating encoding, Base64 Bytes, exact measurement and caller-buffer serialisation. Both demos are maintained in Arduino IDE, PlatformIO Arduino, and PlatformIO ESP-IDF forms.

## Validation

Host validation includes functional compile-time/runtime assertions plus compile-fail contracts. The authoritative execution appliance for the current tranche is AI-AGENT-02; checked-in GitHub Actions remain reproducible workflow definitions rather than authoritative execution evidence.
