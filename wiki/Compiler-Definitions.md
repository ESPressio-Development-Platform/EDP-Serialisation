# Compiler Definitions and Build Controls

## Production compiler definitions

EDP-Serialisation currently defines **no production preprocessor configuration macros** and contains no production `#if`/`#ifdef` feature-selection surface.

The production language baseline is C++20, selected by package/demo build metadata rather than an EDP-owned macro.

## Test/build environment controls

The host runner recognizes environment variables rather than preprocessor definitions:

- `CXX` — host compiler executable; ordinary default is `g++`.
- `EDP_SERIALISATION_SANITIZER_MODE` — empty/default for functional validation, `undefined` for UBSan, `address` for ASan; other non-empty values fail the runner.
- the six `EDP_*_SOURCE_DIR` variables documented in [Build, Test and Source](Build-Test-and-Source) — dependency checkout locations.

These are test/build orchestration controls only; they do not alter the production Serialisation wire contract.

## External framework macros

No Arduino/ESP-IDF SDK macro currently gates Serialisation production functionality. The same foundation headers compile under host C++, Arduino-ESP32, and ESP-IDF.
