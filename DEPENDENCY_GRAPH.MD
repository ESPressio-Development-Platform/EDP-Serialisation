# EDP-Serialisation Dependency Graph

> Baseline: 2026-10-01 — JSON Numeric + LocalisedText execution for Known-Type Body and Typed Envelope roots. Direct dependencies reflect source-level ownership rather than relying on transitive availability.

## Library Dependency Tree

```text
EDP-Serialisation
├── EDP-System [mandatory]
├── EDP-BoundedTypes [mandatory]
├── EDP-Memory [mandatory]
│   ├── EDP-System [mandatory]
│   ├── EDP-Platform [mandatory]
│   └── EDP-BoundedTopology [mandatory]
├── EDP-Platform-Portable [mandatory]
│   ├── EDP-Platform [mandatory]
│   └── EDP-Memory [mandatory]
└── EDP-Localisation [mandatory]
    ├── EDP-System [mandatory]
    └── EDP-Memory [mandatory through its provider contract]
```

`EDP-BoundedTypes` also depends on EDP-Memory and EDP-Platform-Portable, but Serialisation declares those edges directly because production JSON output now consumes their contracts itself.

## Direct dependency ownership

- `EDP-System` — canonical semantic Type/Field identity and schema traversal (`TypeIdentifier`, `FieldIdentifier`, `FieldBinding`, `FieldSet`, `SchemaType`, `ForEachField`).
- `EDP-BoundedTypes` — bounded String/Bytes/Vector values and pairwise `TypeConversionAdapter` plus generic success interpretation used by strong semantic adaptation.
- `EDP-Memory` — authoritative `ByteOperations` provider contract used for caller-buffer byte-range writes; Serialisation does not implement a parallel raw-copy primitive.
- `EDP-Platform-Portable` — default stateless Standard C/C++ `ByteOperationsProvider`. `Serialise` exposes the provider as a compile-time substitution point, so alternative conforming providers can be selected without retaining provider state.
- `EDP-Localisation` — canonical language identities/context validation, forward Field-name materialisation, reverse FieldIdentifier resolution, requested-to-terminal fallback, supported-language enumeration and ambiguity proof for `FieldProfile::LocalisedText`.

No dependency points from these foundational repositories back to Serialisation. The DAG therefore remains acyclic.

## LocalisedText dependency

`EDP-Localisation` is now a direct production dependency because `JsonLocalisedText.hpp` and LocalisedText operation overloads consume its public context, language, scratch and Resolver contracts. Serialisation does not parse language packs or duplicate fallback policy.

Typed Envelope uses the existing direct `EDP-System` dependency for canonical root `TypeIdentifier`; this checkpoint introduces no new dependency edge.
