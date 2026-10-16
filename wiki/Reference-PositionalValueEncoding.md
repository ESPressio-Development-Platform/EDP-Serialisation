# src/serialisation/PositionalValueEncoding.hpp

**Classification:** PUBLIC OPERATION API with owner-private deterministic encoder adaptation. Available through `<ESPressio_Serialisation.hpp>`. EDP-Serialisation package `0.1.0`, additive `feature/logging-v1`.

## Public functions

- `template<std::size_t TMaximumArguments, SerialisableType... TValues> MeasurementResult MeasurePositionalCbor(const TValues&...)`: compile-time bound of argument count, exact CBOR definite array length and all value qualifications; existing `MeasurementStatus` and owner `Diagnostic` vocabulary.
- `template<std::size_t TMaximumArguments, class TByteOperationsProvider, SerialisableType... TValues> SerialisationResult SerialisePositionalCbor(std::span<std::byte>,const TValues&...)`: pre-measure before writing to caller-owned storage; output untouched on preflight error/short destination; existing `SerialisationStatus` and owner ByteOperations selection, no heap or runtime dispatch.
- `MeasurePositionalJson<MaxArgs>` and `SerialisePositionalJson<MaxArgs>`: matching exact-measure/transactional-preflight JSON array methods using Numeric `Detail::EncodeJsonValue`, typed `MeasurementResult` / `SerialisationResult` and existing ByteOperations providers.
- `Detail::EncodePositionalJsonValues<TSink,TValues...>`: **private** ordered JSON `[`/`,`/`]` prefix/element emitter that short-circuits first failure, reusing the owner Json scalar encoder.
- `Detail::EncodePositionalCborValues<TSink,TValues...>`: **private** parameter-pack encoder writing CBOR definite array head and then calling already established `Detail::EncodeCborValue` per argument, with short-circuit stop on first error. No duplicate scalar/float/string logic.

## Semantics

Matching `MeasurePositionalJson` and `SerialisePositionalJson` methods reuse the existing Numeric JSON value encoder, canonical comma-separated array syntax and identical two-pass output guarantees. Heterogeneous argument **pack** support does not qualify a C++ `tuple` or `variant` as a V1 SerialisableType, and does not replace typed schema roots. Float32 is preserved as CBOR 0xFA and float64 as 0xFB; unsigned64 is preserved precisely. Strong values and bounded strings/bytes remain subject to the Serialisation owner's pre-existing canonical adapters and UTF-8 rules.

No new dependency edge, runtime registry, Task, owner memory allocator, or Logging-specific type enters EDP-Serialisation. Output buffer and source values must be stable between passes, exactly as in `Serialise`. This by itself does not encode a governed LogRecord, certify arguments against generated Localisation parameter Types, or translate diagnostics into language-specific text.

## Tests

`tests/host/positional_encoding.cpp` verifies exact 26-byte heterogeneous CBOR golden vector, exact measurement, insufficient-output preflight nonmutation, nonfinite source rejection, and 0-argument canonical array. The JSON tests also verify exact uint64 integer text, f32/f64 decimal output, nonfinite rejection, short-output nonmutation and empty-array syntax. It is included in the ordinary `tests/run_host_tests.sh` suite with all existing positive and expected compile-fail tests. See [full spec](../docs/POSITIONAL-VALUE-ENCODING.MD) and [Dependency Graph](../DEPENDENCY_GRAPH.MD).
