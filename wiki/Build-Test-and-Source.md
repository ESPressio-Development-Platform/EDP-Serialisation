# Build, Test and Source

## Production source

- `src/ESPressio_Serialisation.hpp` — repository-level public umbrella.
- `src/serialisation/Serialisation.hpp` — internal public-header aggregator.
- focused headers under `src/serialisation/` own traits, profiles, limits, results, Type qualification, and System identifier integration.

The [Reference Index](Reference-Index) maps every production header to a dedicated reference page.

## Host validation

`tests/run_host_tests.sh` builds the functional host suite plus every source under `tests/compile_fail/`. It requires explicit source-directory environment variables for current dependency checkouts because EDP-BoundedTypes' public umbrella currently traverses its Memory/Platform/BoundedTopology/Portable stack.

Supported test-runner controls:

- `CXX` — selects the host compiler (validated with GCC and Clang on AI-AGENT-02).
- `EDP_SERIALISATION_SANITIZER_MODE=undefined` — UBSan host pass.
- `EDP_SERIALISATION_SANITIZER_MODE=address` — ASan host pass.
- `EDP_SYSTEM_SOURCE_DIR`, `EDP_BOUNDED_TYPES_SOURCE_DIR`, `EDP_PLATFORM_SOURCE_DIR`, `EDP_MEMORY_SOURCE_DIR`, `EDP_BOUNDED_TOPOLOGY_SOURCE_DIR`, `EDP_PLATFORM_PORTABLE_SOURCE_DIR` — explicit dependency source roots.

`tests/CMakeLists.txt` supplies a second host-build surface using the same dependency roots.

## Compile-fail contracts

Current negative contracts prove:

- canonical strong-Type adaptation requires both conversion directions;
- directly nested Optional is rejected;
- enums require explicit `EnumSerialisationTraits` certification.

## Embedded demos

`demos/type-qualification` exists in all three V2-required forms. AI-AGENT-02 validated both PlatformIO variants against Arduino-ESP32 and ESP-IDF. The appliance currently has no `arduino-cli`, so the Arduino IDE `.ino` form is maintained/buildable source but did not receive a distinct Arduino-CLI execution gate in this checkpoint.

The ESP-IDF build emitted the existing `esp32dev` profile warning (configured 4 MB flash vs detected 2 MB) and still linked/produced firmware successfully.

## GitHub Actions

`.github/workflows/validate.yml` is the maintained reproducible workflow definition. Per current V2 policy, GitHub Actions execution is not authoritative validation evidence; AI-AGENT-02 execution is authoritative for this tranche.
