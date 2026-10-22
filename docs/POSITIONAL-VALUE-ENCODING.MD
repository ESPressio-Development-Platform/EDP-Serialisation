# Heterogeneous positional CBOR and JSON values — owner-level additive G3 prerequisite

**Branch:** `EDP-Serialisation/feature/logging-v1` forked at `fix/cbor-bounded-schema-traversal` SHA `2b0c14164ae2682f4e6581e0bf9ff4a3f8cfddad`. Library version remains **0.1.0**, existing signatures/consumers unchanged.

## Problem and supported contract

The current EDP-Serialisation schema-root `Measure/Serialise` does not itself allow a compile-time **heterogeneous ordered positional argument sequence**, which EDP-Logging's governed Localisation diagnostic schemas will eventually need. Ad hoc CBOR scalars implemented in Logging would violate V2 component reuse and introduce inconsistent float widths/UTF-8/strong-Type handling.

EDP-Serialisation itself now exposes the additive `serialisation/PositionalValueEncoding.hpp` public functions (also through `ESPressio_Serialisation.hpp`):

- `MeasurePositionalCbor<TMaximumArguments>(const TValues&... values)`: validates the *entire* variable-type pack under the existing `SerialisableType<T>` concept and returns the owner `MeasurementResult` with exact byte count.
- `SerialisePositionalCbor<TMaximumArguments, TByteOperationsProvider>(std::span<std::byte> output, const TValues&... values)`: uses the existing owner `EncodingCountingSink` + `EncodingBufferSink` two-pass representation, then returns the existing typed `SerialisationResult` statuses and sizes. On invalid source or insufficient capacity **no output bytes change**.
- The internal `Detail::EncodePositionalCborValues` emits a canonical **definite array header** followed left-to-right by the **existing** `Detail::EncodeCborValue` for each argument. No duplicate CBOR number/string/float encoder and no unbounded buffer, heap/variant, runtime registry or new circular dependency.

The caller statically declares `TMaximumArguments`. A zero-argument diagnostic emits canonical `0x80`. Heterogeneous Types are compile-time arguments only. A subsequent, separately qualified additive [Tuple Schema Fields](./TUPLE-SCHEMA-FIELDS.MD) implementation now also accepts fixed heterogeneous `std::tuple<T...>` as a nested Schema Field, with byte-identical ordered-array encodings; this does not enable arbitrary dynamic containers or variants. The source values must remain stable through both passes, as in existing Serialise.

## Limitations / next work

This encodes **the CBOR or JSON positional array**. It does NOT mint EDP-System's governed LogRecord TypeIdentifier, construct a full LogRecord Typed Envelope, attach any FieldIdentifiers, validate that this pack matches a generated Localisation diagnostic's expected parameter Types, or produce language-specific text. The JSON counterpart uses the pre-existing Numeric `EncodeJsonValue` encoder (including strict Base64 and UTF-8 handling for bounded bytes/string values) and comma-separated definite JSON arrays. No direct Serialisation dependency on Logging is introduced. These are remaining G3/G4/G5 work, and no production Logging source imports this new owner API until its adapter is separately qualified.

## Tests

The corresponding `MeasurePositionalJson` and `SerialisePositionalJson` functions use the existing Numeric JSON encoder and precisely the same measurement-first no-output-mutation policy, for a canonical JSON array of heterogeneous arguments. The pack preserves uint64 exactly and uses deterministic finite float formatting.

The package's standard `tests/run_host_tests.sh` now also compiles and executes `tests/host/positional_encoding.cpp` so downstream use cannot silently skip this regression. The reference vector contains heterogeneous `bool`, negative `int8`, **exact uint64 max**, IEEE float32 1.5 and float64 3.25 in one definite CBOR array; verifies exact bytes, exact measurement, zero-argument array, nonfinite-value rejection and preflight atomicity under an undersized buffer. Existing positive and compile-fail tests continue unchanged.

Native GCC/Clang host evidence plus every impacted downstream runtime consumer must be recorded before claiming source qualification. No physical target/HIL proof is implied. This package has no new direct dependencies; the existing EDP-Platform-Portable/Memory ByteOperations owner remains in charge.
