# EDP-Serialisation Dependency Graph

> Revalidated: 2026-10-02 — V2 policy-compliance remediation adds direct `EDP-BoundedTopology` reuse for the fixed 256-Field presence set. No dependency direction is circular.

## Library Dependency Tree

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

### G3 additive positional-CBOR owner facility (2026-10-16)

`PositionalValueEncoding.hpp` uses exclusively the pre-existing EDP-Serialisation `EncodeCborValue`, `EncodingCountingSink`/`EncodingBufferSink` and selected EDP-Memory/EDP-Platform-Portable ByteOperations infrastructure. There are **no new direct or conditional dependency edges**, no Logging import and no changed transitive path. Future EDP-Logging adaptation will depend on Serialisation, never vice versa.

## Direct dependency ownership

- `EDP-System` — canonical semantic Type/Field identity and schema traversal (`TypeIdentifier`, `FieldIdentifier`, `FieldBinding`, `FieldSet`, `SchemaType`, `ForEachField`).
- `EDP-BoundedTypes` — bounded String/Bytes/Vector values and pairwise `TypeConversionAdapter` plus generic success interpretation used by strong semantic adaptation.
- `EDP-BoundedTopology` — reusable `BoundedIndexSet` used directly as the exact one-bit-per-FieldIdentifier schema presence/duplicate state. Serialisation does not retain a parallel hand-written bitmap.
- `EDP-Memory` — authoritative `ByteOperations` provider contract used for caller-buffer byte-range writes; Serialisation does not implement a parallel raw-copy primitive.
- `EDP-Platform-Portable` — default stateless Standard C/C++ `ByteOperationsProvider`; callers may select another conforming stateless provider at compile time.
- `EDP-Localisation` — canonical language identities/context validation, forward Field-name materialisation, reverse FieldIdentifier resolution, requested-to-terminal fallback, supported-language enumeration and ambiguity proof for `FieldProfile::LocalisedText`.

## Optional/transitive dependency conditions

The default Serialisation umbrella has no optional direct ESPressio dependency; explicit import of serialisation/ClockEraAdapters.hpp activates EDP-Clock as an optional direct source dependency. `EDP-Localisation` exposes an optional `EDP-Persistence` edge only when a consuming application includes/instantiates Localisation `FilePackSource`; Serialisation does not instantiate that source itself. When imposed, `EDP-Persistence` depends on `EDP-System`.

## Dependency direction

No dependency points from these foundational repositories back to Serialisation. The graph therefore remains acyclic. `EDP-BoundedTopology` is dependency-minimal and is intentionally reusable by Memory, Serialisation, and other higher domains.

### Additional owner-certified System identity adapters (2026-10-16)

`SystemIdentityAdapters.hpp` uses the ALREADY mandatory `EDP-System` and `EDP-BoundedTypes` owners; its canonical binary representation is `Bounded::Bytes<16>`, integer `uint32_t`. Existing bounded codecs are reused. **No new EDP-Clock/Mesh/Logging or platform concrete edge, and no version change**.

### Optional G3 ClockEra strong-adapter dependency

Only a consumer importing `<serialisation/ClockEraAdapters.hpp>` activates **EDP-Serialisation -> EDP-Clock [optional]**. The EDP-Clock owner depends on EDP-System and EDP-Platform, which are already foundations; it does not depend on EDP-Serialisation. **This is acyclic**. The default `ESPressio_Serialisation.hpp` remains Clock-free and mandatory `library.json` dependencies remain unchanged. Application Bootstrap must declare EDP-Clock explicitly when selecting this optional source header. The adapter never imports EDP-Mesh or any platform concrete.
