# Reference — ESPressio_Serialisation.hpp

**Source:** `src/ESPressio_Serialisation.hpp`
**Classification:** PUBLIC ENTRY POINT

Repository-level umbrella header. It includes `serialisation/Serialisation.hpp` and declares no symbols of its own.

Consumers should include this header rather than depending on the internal aggregation layout. The umbrella currently exposes the compile-time foundation only; inclusion does not imply JSON/CBOR operation functions exist yet.
