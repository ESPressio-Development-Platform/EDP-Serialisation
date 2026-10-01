# Reference — serialisation/Serialisation.hpp

**Source:** `src/serialisation/Serialisation.hpp`
**Classification:** INTERNAL AGGREGATION HEADER / PUBLICLY REACHABLE

Aggregates the focused production headers:

- `CanonicalRepresentation.hpp`;
- `EnumSerialisationTraits.hpp`;
- `JsonEncoding.hpp`;
- `Optional.hpp`;
- `Operations.hpp`;
- `ParserLimits.hpp`;
- `Profiles.hpp`;
- `Results.hpp`;
- `SerialisableType.hpp`;
- `SystemIdentifierAdapters.hpp`.

It declares no symbols, retains no state, and defines no dependency ownership beyond the focused headers. Ordinary consumers reach it through `ESPressio_Serialisation.hpp`; maintainers may change aggregation order only while preserving specialization visibility needed by `SerialisableType`.

The umbrella also includes `JsonDecoding.hpp`; this is private implementation exposure required by header composition, not a supported direct-consumption contract.
