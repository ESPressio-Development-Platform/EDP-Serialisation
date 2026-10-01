# EDP-Serialisation Dependency Graph

> Baseline: 2026-10-01 — first implementation foundation. Direct dependencies are intentionally minimal; codec/localisation/memory dependencies are added only when implementation starts consuming them directly.

## Library Dependency Tree

```text
EDP-Serialisation
├── EDP-System [mandatory]
└── EDP-BoundedTypes [mandatory]
    ├── EDP-Memory [mandatory]
    │   ├── EDP-System [mandatory]
    │   ├── EDP-Platform [mandatory]
    │   │   └── EDP-System [mandatory, transitive]
    │   └── EDP-BoundedTopology [mandatory]
    └── EDP-Platform-Portable [mandatory]
        ├── EDP-Platform [mandatory]
        └── EDP-Memory [mandatory]
```

## Direct dependency ownership

- `EDP-System` — canonical semantic Type/Field identity and schema traversal (`TypeIdentifier`, `FieldIdentifier`, `FieldBinding`, `FieldSet`, `SchemaType`, `ForEachField`).
- `EDP-BoundedTypes` — bounded String/Bytes/Vector values and the pairwise `TypeConversionAdapter` plus generic success-predicate capability used by strong semantic adaptation.

The foundation introduces no dependency from System or BoundedTypes back to Serialisation. `EDP-Serialisation` remains above those libraries in the dependency DAG.

## Deferred direct dependencies

`EDP-Localisation` and `EDP-Memory` are not direct dependencies in this foundation slice because no implemented source currently calls their contracts. They must be declared directly in the slice where localised Field profiles and raw memory operations respectively begin consuming them.
