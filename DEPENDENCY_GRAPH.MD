# EDP-Serialisation Dependency Graph

> Baseline: 2026-10-01 — JSON Numeric/Known-Type encoding implementation. Direct dependencies reflect source-level ownership rather than relying on transitive availability.

## Library Dependency Tree

```text
EDP-Serialisation
├── EDP-System [mandatory]
├── EDP-BoundedTypes [mandatory]
├── EDP-Memory [mandatory]
│   ├── EDP-System [mandatory]
│   ├── EDP-Platform [mandatory]
│   └── EDP-BoundedTopology [mandatory]
└── EDP-Platform-Portable [mandatory]
    ├── EDP-Platform [mandatory]
    └── EDP-Memory [mandatory]
```

`EDP-BoundedTypes` also depends on EDP-Memory and EDP-Platform-Portable, but Serialisation declares those edges directly because production JSON output now consumes their contracts itself.

## Direct dependency ownership

- `EDP-System` — canonical semantic Type/Field identity and schema traversal (`TypeIdentifier`, `FieldIdentifier`, `FieldBinding`, `FieldSet`, `SchemaType`, `ForEachField`).
- `EDP-BoundedTypes` — bounded String/Bytes/Vector values and pairwise `TypeConversionAdapter` plus generic success interpretation used by strong semantic adaptation.
- `EDP-Memory` — authoritative `ByteOperations` provider contract used for caller-buffer byte-range writes; Serialisation does not implement a parallel raw-copy primitive.
- `EDP-Platform-Portable` — default stateless Standard C/C++ `ByteOperationsProvider`. `Serialise` exposes the provider as a compile-time substitution point, so alternative conforming providers can be selected without retaining provider state.

No dependency points from these foundational repositories back to Serialisation. The DAG therefore remains acyclic.

## Deferred direct dependency

`EDP-Localisation` is not yet a direct dependency because `FieldProfile::LocalisedText` execution remains pending. It must be declared directly when production source begins consuming forward/reverse localisation contracts.
