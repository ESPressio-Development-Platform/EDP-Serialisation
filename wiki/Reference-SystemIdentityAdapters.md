# System Identity canonical adapters

Optional G3 owner-certified, lossless conversions in `serialisation/SystemIdentityAdapters.hpp`, imported by the default `ESPressio_Serialisation.hpp` umbrella, package version 0.1.0.

- `System::Identity::DeviceIdentifier` ↔ `Bounded::Bytes<16>`: exactly 16 octets, CBOR byte-string, JSON Base64. Reverse conversion rejects all shorter values, leaving the output unchanged.
- `System::Identity::RuntimeIncarnationId` ↔ `uint32_t`: canonical CBOR unsigned integer and JSON decimal; no ID assignment.
- `SystemIdentityAdaptationResult`: `Succeeded`, `IncorrectByteLength`, `ByteOperationsFailure`. All conversions are noexcept, owner-defined semantic types and no fabricated LogRecord TypeIdentifier.

See [contract](../docs/SYSTEM-IDENTITY-ADAPTERS.MD), [dependency graph](../DEPENDENCY_GRAPH.MD) and [tests](../tests/host/system_identity_adapters.cpp).
