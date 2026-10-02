# EDP-Serialisation

Deterministic, allocation-free, schema-driven serialisation for the ESPressio Development Platform.

The library is currently in its pre-1.0 implementation tranche at version `0.1.0`. The compile-time foundation is implemented together with deterministic JSON and CBOR Numeric `Measure`, all-or-nothing `Serialise`, and transactional two-pass `Deserialise` for Known-Type Body and explicit Typed Envelope roots. JSON LocalisedText Fields are also implemented through `EDP-Localisation`.

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
- [JSON Numeric Known-Type Decoding](./docs/JSON-NUMERIC-DECODING.MD) — defines transactional `Deserialise`, strictness, numeric/text/Base64 validation, bounded sequence population, and parser limits.
- [JSON Typed Envelope](./docs/JSON-TYPED-ENVELOPE.MD) — defines the explicit root envelope, canonical TypeIdentifier text, version checking, and transactional envelope decode.
- [JSON LocalisedText Fields](./docs/JSON-LOCALISED-TEXT.MD) — defines RFC5646 metadata, forward/reverse Localisation resolution, collision rules, caller scratch, language priority, and transactional decode.
- [CBOR Numeric Profiles](./docs/CBOR-NUMERIC.MD) — defines deterministic Numeric CBOR for Known-Type Body and Typed Envelope roots, canonical heads, exact float widths, native text/byte strings, and transactional decode.
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
- `Measure<Json>(value)` / `Measure<Cbor>(value)` for exact canonical Numeric size/validation;
- `Serialise<Json/Cbor>(value, std::span<std::byte> output)` for Numeric Known-Type Body encoding;
- `Deserialise<Json/Cbor>(std::span<const std::byte> input, destination)` for transactional Numeric Known-Type Body decoding;
- `Measure` / `Serialise` / `Deserialise` with `RootProfile::TypedEnvelope` for identified JSON or CBOR roots carrying envelope version and canonical root `TypeIdentifier`;
- LocalisedText overloads for both root profiles using caller-owned `EDP-Localisation` Resolver/context/scratch state, reserved `RFC5646` metadata, exact collision detection and transactional reverse Field resolution.

## Fixed-width integer note

The V1 contract is defined in terms of the fixed-width C++ Types `std::int8_t` through `std::uint64_t`. C++20 does not retain typedef spelling: when a fixed-width typedef is the exact same C++ Type as a native spelling or another alias such as `std::size_t`, no concept can distinguish which spelling appeared in source. The implementation therefore qualifies the actual fixed-width Type identity available on the target. Developers must use the fixed-width spelling for portable schema intent; the compiler cannot prove that spelling after alias substitution. This limitation never permits native object-layout or host-endian wire encoding.

## Dependencies

The complete mandatory/conditional ESPressio dependency tree is:

```text
EDP-Serialisation
├── EDP-System [mandatory]
├── EDP-BoundedTypes [mandatory]
│   ├── EDP-Memory [mandatory]
│   │   ├── EDP-System [mandatory]
│   │   ├── EDP-Platform [mandatory]
│   │   │   └── EDP-System [mandatory, transitive]
│   │   └── EDP-BoundedTopology [mandatory]
│   └── EDP-Platform-Portable [mandatory]
│       ├── EDP-Platform [mandatory]
│       │   └── EDP-System [mandatory]
│       └── EDP-Memory [mandatory]
│           ├── EDP-System [mandatory]
│           ├── EDP-Platform [mandatory]
│           │   └── EDP-System [mandatory, transitive]
│           └── EDP-BoundedTopology [mandatory]
├── EDP-BoundedTopology [mandatory]
├── EDP-Memory [mandatory]
│   ├── EDP-System [mandatory]
│   ├── EDP-Platform [mandatory]
│   │   └── EDP-System [mandatory, transitive]
│   └── EDP-BoundedTopology [mandatory]
├── EDP-Platform-Portable [mandatory]
│   ├── EDP-Platform [mandatory]
│   │   └── EDP-System [mandatory]
│   └── EDP-Memory [mandatory]
│       ├── EDP-System [mandatory]
│       ├── EDP-Platform [mandatory]
│       │   └── EDP-System [mandatory, transitive]
│       └── EDP-BoundedTopology [mandatory]
└── EDP-Localisation [mandatory]
    ├── EDP-System [mandatory]
    ├── EDP-Memory [mandatory]
    │   ├── EDP-System [mandatory]
    │   ├── EDP-Platform [mandatory]
    │   │   └── EDP-System [mandatory, transitive]
    │   └── EDP-BoundedTopology [mandatory]
    └── EDP-Persistence [optional through EDP-Localisation]
        condition: only when an application instantiates Localisation FilePackSource
        └── EDP-System [mandatory when imposed]
```

Serialisation consumes `EDP-BoundedTopology::BoundedIndexSet` directly for one-bit-per-FieldIdentifier presence/duplicate state, `EDP-Memory` for ByteOperations, and `EDP-Platform-Portable` for the default stateless ByteOperations provider. See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD) for ownership and dependency-direction detail.

## C++20 and PIOArduino

ESPressio requires C++20. For the policy-required PlatformIO-format ESP32 projects, install **PIOArduino Core** rather than upstream PlatformIO Core. The checked-in `platformio.ini` files select the maintained PIOArduino `platform-espressif32` package and explicitly remove older GNU language flags before adding `-std=gnu++20`, ensuring Arduino-ESP32 and ESP-IDF builds use the supported C++20 baseline.

A current PIOArduino Core can be installed using its maintained installer:

```sh
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/pioarduino/pioarduino-core-installer/pioarduino/get-platformio.py
python3 get-platformio.py
```

The resulting command remains `pio`, so the project folder names retain the policy-mandated `PlatformIO_Arduino` / `PlatformIO_ESP-IDF` convention while execution uses PIOArduino.

## Demos

`demos/type-qualification` demonstrates schema recursion, bounded values, enum certification, profile vocabulary, and compile-time qualification. `demos/json-numeric-encoding` demonstrates canonical Numeric Field ordering, Optional omission, floating encoding, Base64 Bytes, exact measurement and caller-buffer serialisation. `demos/json-numeric-decoding` demonstrates schema-ID resolution, Optional omission semantics, floating decode, strict Base64 and transactional population. `demos/json-typed-envelope` demonstrates canonical envelope emission plus transactional root Type/version validation. `demos/json-localised-text` demonstrates caller-owned Localisation/scratch, RFC5646 metadata and textual Field round-trip. `demos/cbor-numeric` demonstrates deterministic CBOR Numeric encoding/transactional decoding for both Known-Type Body and Typed Envelope roots. Every logical demo is maintained in Arduino IDE, PlatformIO Arduino, and PlatformIO ESP-IDF forms; PIOArduino is the recommended builder for the latter two forms.

## Validation

Host validation includes functional compile-time/runtime assertions plus compile-fail contracts. The authoritative execution appliance for the current tranche is AI-AGENT-02; checked-in GitHub Actions remain reproducible workflow definitions rather than authoritative execution evidence.
