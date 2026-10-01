# Internal API

The internal implementation surface spans compile-time classification in `SerialisableType.hpp`, shared codec-neutral sinks in `EncodingSinks.hpp`, JSON traversal/parsing in `JsonEncoding.hpp` / `JsonDecoding.hpp`, LocalisedText schema-key policy in `JsonLocalisedText.hpp`, deterministic CBOR traversal/parsing in `CborEncoding.hpp` / `CborDecoding.hpp`, root wrappers in the envelope headers, and public-operation/profile mapping helpers in `Operations.hpp`.

These symbols are **PRIVATE IMPLEMENTATION**, not cross-repository provider contracts: no other EDP repository should compile against them. They may change without preserving public compatibility so long as `SerialisableType<T>` semantics remain intact.

The classifier layer provides:

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

## JSON encoder internals

The JSON encoder layer provides:

- zero-storage measurement and caller-buffer sinks;
- exact UTF-8 validation and deterministic escaping;
- canonical integer/float/Base64 emission;
- compile-time Field binding lookup and ascending canonical Field emission;
- policy-aware schema key/prefix hooks shared by Numeric and LocalisedText;
- recursive array, bounded Vector, Optional, schema, enum and strong-Type traversal.

`Operations.hpp` maps JSON and CBOR internal outcomes to the public operation-specific result families and compile-time rejects unsupported codec/profile combinations. These Detail declarations are **PRIVATE IMPLEMENTATION**, not cross-repository contracts.

The decoder adds immutable input cursors, number tokens, exact Unicode String parsing, strict Base64, fixed presence-bitmaps, bounded unknown-value skipping, exact duplicate-key replay, recursive value/schema population and strong reverse adaptation. See [Reference — JSON Encoding](Reference-JsonEncoding), [Reference — JSON Decoding](Reference-JsonDecoding), and [Reference — Operations](Reference-Operations).

`JsonEnvelope.hpp` is **PRIVATE IMPLEMENTATION** for the explicit JSON root wrapper. It reuses policy-aware JsonEncoding/JsonDecoding and does not define a second body codec. `JsonLocalisedText.hpp` supplies Localisation-specific schema-key hooks, metadata replay and reverse-resolution without duplicating scalar/container codec logic.


## CBOR internals

`EncodingSinks.hpp` supplies the measurement/caller-buffer sink shared with JSON. `CborEncoding.hpp` owns canonical CBOR heads, scalar/category encoding, definite arrays/maps, schema ordering and recursive value traversal. `CborDecoding.hpp` owns canonical-head validation, exact-width floats, bounded text/bytes, structural unknown-value skipping, schema presence/strictness, and recursive validation/population. `CborEnvelope.hpp` wraps those body operations with the fixed V1 three-element Typed Envelope. All are **PRIVATE IMPLEMENTATION**.
