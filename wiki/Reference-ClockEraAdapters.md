# Optional ClockEra adapter

Header `<serialisation/ClockEraAdapters.hpp>` (version `0.1.0`) activates an optional EDP-Serialisation → EDP-Clock dependency and certifies reversible `Clock::ClockEra` ↔ `uint64_t`. Canonical CBOR unsigned integer and JSON decimal encoding reuse the existing EDP-Serialisation value writers. Zero remains the invalid-era sentinel; serialization never assigns or validates ClockEra provenance. EDP-Mesh and concrete platform SDKs are **not** imported.

The default Serialisation umbrella does not import this optional adapter. [Architecture and contract](../docs/CLOCK-ERA-ADAPTERS.MD) • [Owner tests](../tests/run-clock-era-adapters-tests.sh) • [Dependency graph](../DEPENDENCY_GRAPH.MD).
