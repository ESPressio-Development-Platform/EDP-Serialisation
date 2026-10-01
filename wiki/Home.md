# EDP-Serialisation Wiki

EDP-Serialisation owns deterministic representation rules and the recursive compile-time qualification needed to decide which C++ values can participate in ESPressio Serialisation. The current `0.1.0` foundation implements Type qualification, strong-Type canonical adaptation declarations, enum certification, profile/resource vocabulary, System identifier adapters, and operation-specific result Types.

It deliberately does **not** own semantic Type/Field identity (`EDP-System`), bounded container storage or pairwise conversion (`EDP-BoundedTypes`), Transport, Persistence, Security, Threading, Localisation, or application object lifetime. JSON/CBOR encoding/decoding operations are not yet implemented in this foundation checkpoint and must not be inferred from the presence of codec tags/result Types.

The consumer entry point is `src/ESPressio_Serialisation.hpp`.

Direct production dependencies are `EDP-System` and `EDP-BoundedTypes`. The latter currently brings Memory/Platform/BoundedTopology/Portable dependencies transitively.

## Navigation

Start with [Architecture](Architecture) for ownership and layering, [Public API](Public-API) for supported consumer contracts, [Internal API](Internal-API) and [Private Implementation](Private-Implementation) for maintainer-facing compile-time machinery, [Dependency Contracts](Dependency-Contracts) for cross-repository boundaries, and [Build, Test and Source](Build-Test-and-Source) for validation/source navigation. [Reference Index](Reference-Index) provides exhaustive production-header coverage.
