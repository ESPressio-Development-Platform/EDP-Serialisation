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

## Direct dependency ownership

- `EDP-System` — canonical semantic Type/Field identity and schema traversal (`TypeIdentifier`, `FieldIdentifier`, `FieldBinding`, `FieldSet`, `SchemaType`, `ForEachField`).
- `EDP-BoundedTypes` — bounded String/Bytes/Vector values and pairwise `TypeConversionAdapter` plus generic success interpretation used by strong semantic adaptation.
- `EDP-BoundedTopology` — reusable `BoundedIndexSet` used directly as the exact one-bit-per-FieldIdentifier schema presence/duplicate state. Serialisation does not retain a parallel hand-written bitmap.
- `EDP-Memory` — authoritative `ByteOperations` provider contract used for caller-buffer byte-range writes; Serialisation does not implement a parallel raw-copy primitive.
- `EDP-Platform-Portable` — default stateless Standard C/C++ `ByteOperationsProvider`; callers may select another conforming stateless provider at compile time.
- `EDP-Localisation` — canonical language identities/context validation, forward Field-name materialisation, reverse FieldIdentifier resolution, requested-to-terminal fallback, supported-language enumeration and ambiguity proof for `FieldProfile::LocalisedText`.

## Optional/transitive dependency conditions

Serialisation itself has no optional direct ESPressio dependency. `EDP-Localisation` exposes an optional `EDP-Persistence` edge only when a consuming application includes/instantiates Localisation `FilePackSource`; Serialisation does not instantiate that source itself. When imposed, `EDP-Persistence` depends on `EDP-System`.

## Dependency direction

No dependency points from these foundational repositories back to Serialisation. The graph therefore remains acyclic. `EDP-BoundedTopology` is dependency-minimal and is intentionally reusable by Memory, Serialisation, and other higher domains.
