# Internal API

The current internal implementation surface is entirely compile-time and lives under `ESPressio::Serialisation::Detail` in `src/serialisation/SerialisableType.hpp`.

These symbols are **PRIVATE IMPLEMENTATION**, not cross-repository provider contracts: no other EDP repository should compile against them. They may change without preserving public compatibility so long as `SerialisableType<T>` semantics remain intact.

The internal layer provides:

- cv-normalisation (`NormalizedType`);
- fixed-width integer and supported-floating classifiers;
- Optional classification;
- detection of canonical-representation and enum-certification specialisations;
- the `PotentialSchemaType` safety pre-gate around current `EDP-System/main` schema predicates;
- recursive schema-Field qualification through `System::ForEachField`;
- enum-certification validation;
- direct canonical-adaptation validation;
- recursive container/schema/adaptation trait specialisations.

The compatibility pre-gate does not replace System ownership. `EDP-System#10` already owns the substitution-safe `IdentifiedType`/`SchemaType` correction on its `serialisation_prerequisites` branch; Serialisation retains the pre-gate while consuming current System `main`.

For symbol-level invariants see [Reference — SerialisableType](Reference-SerialisableType).
