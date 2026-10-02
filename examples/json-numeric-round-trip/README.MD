# JSON Numeric Round Trip

This example is the end-to-end starting point for `EDP-Serialisation`. It defines an application schema, proves that the Type satisfies the Serialisation contract, measures and serialises a value into caller-owned storage, then transactionally deserialises exactly the written bytes into a fresh destination.

The example deliberately uses the simplest implemented profile combination:

- codec: `Json`;
- root: `RootProfile::KnownTypeBody` (the default);
- Fields: `FieldProfile::Numeric` (the default);
- strictness: `StrictnessPolicy::Exact` (the default).

The resulting canonical representation is:

```json
{"2":"sensor-A","5":2350}
```

Field `8` is an Optional sample count. It is deliberately disengaged before serialisation, so it is omitted from the JSON object and becomes disengaged in the destination only after the complete input validates successfully.

## Source forms

- `Portable_CMake` — portable C++20 executable; this is the canonical source reproduced in the repository root `README.MD`.
- `Arduino_IDE` — one-shot Arduino sketch which prints the encoded representation and decoded result over Serial.
- `PlatformIO_Arduino` — PIOArduino Arduino-ESP32 project.
- `PlatformIO_ESP-IDF` — PIOArduino ESP-IDF project.
## Run the portable example

When the ESPressio repositories are checked out as siblings, the portable CMake project discovers its dependencies automatically:

```sh
cmake -S examples/json-numeric-round-trip/Portable_CMake \
      -B /tmp/edp-serialisation-json-round-trip
cmake --build /tmp/edp-serialisation-json-round-trip
/tmp/edp-serialisation-json-round-trip/EDP_Serialisation_JsonNumericRoundTrip
```

If dependencies are stored elsewhere, pass the corresponding `EDP_*_SOURCE_DIR` CMake variables explicitly, using `tests/CMakeLists.txt` as the authoritative list.

Expected output:

```text
Encoded JSON:
{"2":"sensor-A","5":2350}
Decoded reading: sensor=sensor-A temperature-centi-c=2350 sample-count=absent
Round trip succeeded.
```

## Build the ESP32 forms

Install current PIOArduino Core as described in the repository root README, then build either maintained project:
```sh
pio run -d examples/json-numeric-round-trip/PlatformIO_Arduino
pio run -d examples/json-numeric-round-trip/PlatformIO_ESP-IDF
```

For an attached ESP32 using the Arduino form, upload and monitor with the usual PIOArduino targets, for example:

```sh
pio run -d examples/json-numeric-round-trip/PlatformIO_Arduino -t upload
pio device monitor -b 115200
```

## What this example proves

The source demonstrates that schema declaration order does not determine Numeric JSON member order: the schema declares Fields `8`, `2`, `5`, while canonical output emits present Fields in ascending identifier order (`2`, then `5`). It also demonstrates deterministic preflight measurement, caller-owned `std::span<std::byte>` output, exact written-range input, operation-specific results, and transactional destination population.

This example intentionally does not introduce Typed Envelope, LocalisedText, CBOR, custom adapters, or non-default parser policies. Those facilities are documented separately and remain easier to understand after the basic round-trip workflow is established.
