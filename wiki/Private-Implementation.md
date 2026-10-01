# Private Implementation

EDP-Serialisation has **no retained runtime private object state**. The implementation is header-only compile-time classification plus stack/caller-owned JSON traversal state, constexpr identifier adapters, and fixed-size result values.

Private implementation includes the `SerialisableType` classifier machinery plus JSON sinks, UTF-8/Base64/numeric encoding helpers, schema Field lookup, and operation-status mapping. It retains no registry, cache, allocator, schema copy, provider object, global mutable state, or background lifecycle.

Important invariants for future implementation:

- schema metadata continues to come from `EDP-System`; do not create a parallel runtime schema registry;
- canonical adaptation remains one hop at the represented Type boundary; nested schema/container values may independently use their own valid adaptations;
- bounded value/container semantics remain owned by `EDP-BoundedTypes`;
- no direct Memory or Localisation dependency is added until production source actually consumes those contracts;
- the implemented encoder must continue using caller-owned output and bounded local temporaries;
- pending decoders must preserve the locked transactional destination rules rather than hiding temporary object ownership in this internal layer.
