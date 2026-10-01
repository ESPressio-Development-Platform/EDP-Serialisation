# Dependency Contracts

## Production dependency: EDP-System

**Kind:** mandatory direct package/API dependency.

Serialisation consumes `TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, `TypeLocalIdentifier`, `SchemaType`, `FieldBinding`, `FieldSet`, `FieldValueOf`, and `ForEachField`. System remains the sole owner of semantic identity and schema metadata; Serialisation does not copy these into a registry.

`SystemIdentifierAdapters.hpp` adapts System value Types from the Serialisation side so EDP-System does not acquire a reverse dependency. The current Serialisation Type classifier uses a safe structural pre-gate before `System::SchemaType` because `EDP-System#10`'s substitution-safe predicate correction is implemented on `serialisation_prerequisites` rather than current System `main`.

No System provider object is owned or retained, and no EDP-System Composition capability is selected by this dependency.

## Production dependency: EDP-BoundedTypes

**Kind:** mandatory direct package/API dependency.

Serialisation consumes bounded `String`, `Bytes`, and `Vector` families plus the pairwise `TypeConversionAdapter`, `IsTypeConversionAvailable`, `HasTypeConversionSuccessPredicate`, and `IsTypeConversionSuccessful` extension contract.

BoundedTypes remains authoritative for container storage/capacity semantics and pairwise conversion ownership. Serialisation requires success-aware, `noexcept` adapters for canonical strong-Type adaptation but does not replace or reinterpret the adapter-specific result enum.

## Transitive dependency topology

Current BoundedTypes `main` brings:

- `EDP-Memory` — ByteOperations abstraction used internally by Bounded String/Bytes;
- `EDP-Platform-Portable` — default portable ByteOperations provider;
- `EDP-Platform` and `EDP-System` transitively;
- `EDP-BoundedTopology` transitively through current EDP-Memory.

These are **not** direct production dependencies of the current Serialisation foundation because Serialisation source does not yet call their contracts directly. The host test harness accepts explicit checkout roots for the transitive headers needed to compile the current BoundedTypes public umbrella.

## Demo/build dependencies

PlatformIO demo projects declare direct `EDP-System` and `EDP-BoundedTypes` Git dependencies. PlatformIO resolves the latter's current dependency graph transitively. The Arduino `library.properties` contract declares both direct dependencies in the `0.1.x` range.

## Deferred direct dependencies

- `EDP-Memory` becomes a direct dependency only when Serialisation production code performs raw memory operations which must consume its abstractions.
- `EDP-Localisation` becomes direct when the LocalisedText Field profile actually calls forward/reverse presentation resolution.

Neither dependency may be added merely because future architecture anticipates it.

## Dependency direction invariant

The intended DAG remains:

```text
EDP-System -----------+
                      v
              EDP-Serialisation
                      ^
                      |
EDP-BoundedTypes -----+
```

Foundational owners must not depend back on EDP-Serialisation. Transport, Persistence, Security, Threading, Command, Event, State and application layers are consumers above this repository rather than dependencies below it.
