# Private Implementation

EDP-Serialisation currently has **no runtime private object state**. The implemented foundation is header-only compile-time classification plus trivial constexpr identifier adapters and result value Types.

The only private implementation machinery is the `ESPressio::Serialisation::Detail` classifier set documented under [Internal API](Internal-API) and [Reference — SerialisableType](Reference-SerialisableType). It retains no registry, cache, allocator, schema copy, provider object, global mutable state, or background lifecycle.

Important invariants for future implementation:

- schema metadata continues to come from `EDP-System`; do not create a parallel runtime schema registry;
- canonical adaptation remains one hop at the represented Type boundary; nested schema/container values may independently use their own valid adaptations;
- bounded value/container semantics remain owned by `EDP-BoundedTypes`;
- no direct Memory or Localisation dependency is added until production source actually consumes those contracts;
- later codecs must preserve caller ownership and the locked transactional/deserialization rules rather than hiding temporary object ownership in this internal layer.
