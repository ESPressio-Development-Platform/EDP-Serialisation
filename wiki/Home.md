# EDP-Serialisation Wiki

EDP-Serialisation owns deterministic representation rules and the recursive compile-time qualification needed to decide which C++ values can participate in ESPressio Serialisation. The current `0.1.0` implementation provides Type qualification, strong-Type canonical adaptation, profile/resource vocabulary, System identifier adapters, and the first executable codec profile: exact `Measure` plus all-or-nothing `Serialise` for JSON + Known-Type Body + Numeric Fields.

It deliberately does **not** own semantic Type/Field identity (`EDP-System`), bounded container storage or pairwise conversion (`EDP-BoundedTypes`), Transport, Persistence, Security, Threading, Localisation, or application object lifetime. JSON deserialisation, Typed Envelope execution, LocalisedText Fields, and CBOR remain pending and must not be inferred from the implemented JSON Numeric/Known-Type encoder.

The consumer entry point is `src/ESPressio_Serialisation.hpp`.

Direct production dependencies are `EDP-System`, `EDP-BoundedTypes`, `EDP-Memory`, and `EDP-Platform-Portable`; the Memory/Portable edges are direct because production JSON output consumes the ByteOperations contract/default provider.

## Navigation

Start with [Architecture](Architecture) for ownership and layering, [Public API](Public-API) for supported consumer contracts, [Internal API](Internal-API) and [Private Implementation](Private-Implementation) for maintainer-facing compile-time machinery, [Dependency Contracts](Dependency-Contracts) for cross-repository boundaries, and [Build, Test and Source](Build-Test-and-Source) for validation/source navigation. [Reference Index](Reference-Index) provides exhaustive production-header coverage.
