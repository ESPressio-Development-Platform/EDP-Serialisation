# Private Implementation

EDP-Serialisation has **no retained runtime private object state**. The implementation is header-only compile-time classification plus stack/caller-owned JSON/CBOR traversal state, constexpr identifier adapters, and fixed-size result values.

Private implementation includes the `SerialisableType` classifier machinery plus shared output sinks, JSON UTF-8/Base64/numeric helpers, canonical CBOR head/scalar/container helpers, replayable JSON/CBOR parser state, numeric conversion, duplicate-key replay, schema presence/Field dispatch, LocalisedText language/key resolution policy, and operation-status mapping. It retains no registry, cache, allocator, schema copy, provider object, global mutable state, or background lifecycle.

Important invariants for future implementation:

- schema metadata continues to come from `EDP-System`; do not create a parallel runtime schema registry;
- canonical adaptation remains one hop at the represented Type boundary; nested schema/container values may independently use their own valid adaptations;
- bounded value/container semantics remain owned by `EDP-BoundedTypes`;
- direct dependencies remain justified by production consumption: Memory owns byte operations and Localisation owns LocalisedText language/name semantics;
- the implemented encoder must continue using caller-owned output and bounded local temporaries;
- the decoder must preserve transactional destination rules through complete validation before replayed population, without hiding a second object/DOM or unbounded key dictionary.

- Typed Envelope implementation must remain a fixed-state root wrapper over the selected JSON/CBOR body codec and must not grow a runtime Type registry/factory.
