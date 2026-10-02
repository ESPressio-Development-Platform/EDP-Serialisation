# EDP-Serialisation

Deterministic, allocation-free, schema-driven serialisation for the ESPressio Development Platform.

The library is currently in its pre-1.0 implementation tranche at version `0.1.0`. The compile-time foundation is implemented together with deterministic JSON and CBOR Numeric `Measure`, all-or-nothing `Serialise`, and transactional two-pass `Deserialise` for Known-Type Body and explicit Typed Envelope roots. JSON LocalisedText Fields are also implemented through `EDP-Localisation`.

## Core Guarantees

- C++20 only.
- No dynamic allocation, runtime schema registry, RTTI, DOM, mutable global state, or hidden ownership.
- `EDP-System` remains authoritative for `TypeIdentifier`, `FieldIdentifier`, `FieldBinding`, `FieldSet`, and `SchemaType`.
- `EDP-BoundedTypes` remains authoritative for bounded values/containers and pairwise `TypeConversionAdapter` conversion.
- Strong semantic adaptation names one direct canonical representation through `CanonicalRepresentation<T>` and requires noexcept, success-aware conversion adapters in both directions.
- Enums require explicit `EnumSerialisationTraits<TEnum>` certification of their fixed-width underlying Type.
- Nested schema fields are qualified recursively through the existing System FieldSet traversal contract.
- Dynamic STL owners, pointers, views, long double, maps/sets, tuples and variants remain outside the V1 serialisable universe unless separately designed.

## Documentation

- [V1 Type Qualification](./docs/TYPE-QUALIFICATION.MD) — defines the recursive Type universe, enum certification, schema recursion, bounded containers, and strong semantic adaptation.
- [Profiles, Limits and Results](./docs/PROFILES-AND-RESULTS.MD) — defines compile-time codec/profile vocabulary, parser resource policy, and operation-specific result families.
- [JSON Numeric Known-Type Encoding](./docs/JSON-NUMERIC-ENCODING.MD) — defines the implemented `Measure`/`Serialise` profile, canonical wire rules, no-partial-output guarantee, and resource behavior.
- [JSON Numeric Known-Type Decoding](./docs/JSON-NUMERIC-DECODING.MD) — defines transactional `Deserialise`, strictness, numeric/text/Base64 validation, bounded sequence population, and parser limits.
- [JSON Typed Envelope](./docs/JSON-TYPED-ENVELOPE.MD) — defines the explicit root envelope, canonical TypeIdentifier text, version checking, and transactional envelope decode.
- [JSON LocalisedText Fields](./docs/JSON-LOCALISED-TEXT.MD) — defines RFC5646 metadata, forward/reverse Localisation resolution, collision rules, caller scratch, language priority, and transactional decode.
- [CBOR Numeric Profiles](./docs/CBOR-NUMERIC.MD) — defines deterministic Numeric CBOR for Known-Type Body and Typed Envelope roots, canonical heads, exact float widths, native text/byte strings, and transactional decode.
- [Dependency Graph](./DEPENDENCY_GRAPH.MD) — records direct ownership boundaries and current transitive dependency topology.

## Implemented API

```cpp
#include <ESPressio_Serialisation.hpp>

static_assert(ESPressio::Serialisation::SerialisableType<std::uint32_t>);
static_assert(ESPressio::Serialisation::SerialisableType<
    ESPressio::Bounded::String<32U>
>);
```

Public foundation components:

- `SerialisableType<T>` / `IsSerialisableType<T>`;
- `CanonicalRepresentation<T>`;
- `EnumSerialisationTraits<TEnum>`;
- `Optional<T>`;
- `Json` / `Cbor`, `RootProfile`, `FieldProfile`, `StrictnessPolicy`;
- `ParserLimits<>` / `DefaultParserLimits`;
- operation-specific `MeasurementResult`, `SerialisationResult`, and `DeserialisationResult` families;
- canonical adapters for `System::TypeIdentifier`, `FieldIdentifier`, `TypeAuthorityIdentifier`, and `TypeLocalIdentifier`;
- `Measure<Json>(value)` / `Measure<Cbor>(value)` for exact canonical Numeric size/validation;
- `Serialise<Json/Cbor>(value, std::span<std::byte> output)` for Numeric Known-Type Body encoding;
- `Deserialise<Json/Cbor>(std::span<const std::byte> input, destination)` for transactional Numeric Known-Type Body decoding;
- `Measure` / `Serialise` / `Deserialise` with `RootProfile::TypedEnvelope` for identified JSON or CBOR roots carrying envelope version and canonical root `TypeIdentifier`;
- LocalisedText overloads for both root profiles using caller-owned `EDP-Localisation` Resolver/context/scratch state, reserved `RFC5646` metadata, exact collision detection and transactional reverse Field resolution.

## Step-by-step example: JSON Numeric round trip

For a first complete use of the library, start with the runnable [JSON Numeric round-trip example](./examples/json-numeric-round-trip/README.MD). It deliberately uses the default Known-Type Body + Numeric Field profiles so the core workflow is visible without Typed Envelope, LocalisedText, or CBOR concepts obscuring it.

The example proceeds in the same order an application normally does:

1. **Define an application Type and schema.** `SensorReading` uses bounded/fixed-width payload Types, a stable `TypeIdentifier`, and `FieldBinding`s with numeric Field identifiers.
2. **Prove the Type is supported.** `static_assert(SerialisableType<SensorReading>)` makes unsupported schema content a compile-time error.
3. **Populate the source value.** Fallible bounded-string assignment is checked using its strongly typed result; the Optional sample count is deliberately disengaged.
4. **Measure first.** `Measure<Json>` validates the source and returns the exact number of bytes required for the canonical representation.
5. **Provide caller-owned storage.** The application owns a fixed `std::array<std::byte, ...>` and passes it as the required `std::span<std::byte>`; Serialisation allocates no output buffer.
6. **Serialise.** `Serialise<Json>` performs its own exact preflight and writes the complete representation only on success. The example proves `RequiredBytes == BytesWritten == Measure.RequiredBytes`.
7. **Deserialise exactly the written range.** Only `BytesWritten` bytes become the immutable input span. `Deserialise<Json>` validates the entire document before populating the destination.
8. **Verify the round trip.** The decoded value must exactly match the source, including the disengaged Optional Field.

The schema deliberately declares Fields in the order `8`, `2`, `5`; the emitted JSON contains the present Fields in canonical numeric order:

```json
{"2":"sensor-A","5":2350}
```

### Complete compilable source

The following block is mechanically checked against [`examples/json-numeric-round-trip/Portable_CMake/main.cpp`](./examples/json-numeric-round-trip/Portable_CMake/main.cpp), so documentation validation fails if the runnable source and README diverge.

<!-- JSON_NUMERIC_ROUND_TRIP_SOURCE_BEGIN -->
```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <span>

#include <ESPressio_Serialisation.hpp>

namespace Example {

    /// Application-owned sensor reading used by the JSON Numeric round-trip example.
    struct SensorReading final {

        // Application payload.

        /// Human-readable sensor name retained in deterministic bounded storage.
        ESPressio::Bounded::String<16U> SensorName{};

        /// Temperature represented as hundredths of a degree Celsius.
        std::uint16_t TemperatureCentiCelsius = 0U;

        /// Optional number of samples represented by this reading.
        std::optional<std::uint32_t> SampleCount{999U};

        // Schema metadata.

        /// Stable semantic identity of this application Type.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U,
                0x00U,
                0x08U,
                0x00U,
                0x00U,
                0x00U,
                0x00U,
                0x01U
            }
        };

        /// Numeric Field schema deliberately declared out of wire-order.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&SensorReading::SampleCount, 8U>,
            ESPressio::System::FieldBinding<&SensorReading::SensorName, 2U>,
            ESPressio::System::FieldBinding<&SensorReading::TemperatureCentiCelsius, 5U>
        >;

    };

    static_assert(ESPressio::Serialisation::SerialisableType<SensorReading>);

    // Round-trip comparison.

    /// Reports whether the decoded reading is identical to the source reading.
    ///
    /// @param source Original reading supplied to Serialise.
    /// @param decoded Reading populated by Deserialise.
    /// @return true when every application payload Field is identical.
    bool IsEquivalent(
        const SensorReading& source,
        const SensorReading& decoded
    ) noexcept {
        return source.SensorName == decoded.SensorName
            && source.TemperatureCentiCelsius == decoded.TemperatureCentiCelsius
            && source.SampleCount == decoded.SampleCount;
    }

} // Example

// Portable execution.

/// Executes one complete JSON Numeric Known-Type Body round trip.
///
/// @return EXIT_SUCCESS when the serialised representation round-trips exactly; EXIT_FAILURE otherwise.
int main() {
    Example::SensorReading source{};
    const auto labelResult = source.SensorName.Assign("sensor-A");
    if (labelResult != ESPressio::Bounded::StringAssignmentResult::Succeeded) {
        std::puts("Unable to populate the bounded sensor name.");
        return EXIT_FAILURE;
    }

    source.TemperatureCentiCelsius = 2350U;
    source.SampleCount.reset();

    const auto measurement = ESPressio::Serialisation::Measure<
        ESPressio::Serialisation::Json
    >(source);
    if (!measurement.IsSuccessful()) {
        std::printf(
            "Measurement failed with status %u.\n",
            static_cast<unsigned>(measurement.Status)
        );
        return EXIT_FAILURE;
    }

    std::array<std::byte, 128U> output{};
    if (measurement.RequiredBytes > output.size()) {
        std::puts("The example output buffer is unexpectedly too small.");
        return EXIT_FAILURE;
    }

    const auto serialisation = ESPressio::Serialisation::Serialise<
        ESPressio::Serialisation::Json
    >(
        source,
        std::span<std::byte>{
            output.data(),
            output.size()
        }
    );
    if (!serialisation.IsSuccessful()) {
        std::printf(
            "Serialisation failed with status %u.\n",
            static_cast<unsigned>(serialisation.Status)
        );
        return EXIT_FAILURE;
    }

    if (serialisation.RequiredBytes != measurement.RequiredBytes
        || serialisation.BytesWritten != measurement.RequiredBytes) {
        std::puts("Measure and Serialise disagree about the representation size.");
        return EXIT_FAILURE;
    }

    std::puts("Encoded JSON:");
    std::fwrite(
        output.data(),
        1U,
        serialisation.BytesWritten,
        stdout
    );
    std::putchar('\n');

    Example::SensorReading decoded{};
    const auto deserialisation = ESPressio::Serialisation::Deserialise<
        ESPressio::Serialisation::Json
    >(
        std::span<const std::byte>{
            output.data(),
            serialisation.BytesWritten
        },
        decoded
    );
    if (!deserialisation.IsSuccessful()) {
        std::printf(
            "Deserialisation failed with status %u at byte %u.\n",
            static_cast<unsigned>(deserialisation.Status),
            static_cast<unsigned>(deserialisation.Detail.ByteOffset)
        );
        return EXIT_FAILURE;
    }

    if (!Example::IsEquivalent(
        source,
        decoded
    )) {
        std::puts("The decoded reading does not match the source reading.");
        return EXIT_FAILURE;
    }

    std::printf(
        "Decoded reading: sensor=%s temperature-centi-c=%u sample-count=%s\n",
        decoded.SensorName.CStr(),
        static_cast<unsigned>(decoded.TemperatureCentiCelsius),
        decoded.SampleCount.has_value() ? "present" : "absent"
    );
    std::puts("Round trip succeeded.");
    return EXIT_SUCCESS;
}
```
<!-- JSON_NUMERIC_ROUND_TRIP_SOURCE_END -->

### Build and run

With the ESPressio repositories checked out as siblings:

```sh
cmake -S examples/json-numeric-round-trip/Portable_CMake \
      -B /tmp/edp-serialisation-json-round-trip
cmake --build /tmp/edp-serialisation-json-round-trip
/tmp/edp-serialisation-json-round-trip/EDP_Serialisation_JsonNumericRoundTrip
```

Expected output:

```text
Encoded JSON:
{"2":"sensor-A","5":2350}
Decoded reading: sensor=sensor-A temperature-centi-c=2350 sample-count=absent
Round trip succeeded.
```

Equivalent Arduino IDE, PIOArduino Arduino-ESP32, and PIOArduino ESP-IDF source forms live under [`examples/json-numeric-round-trip`](./examples/json-numeric-round-trip/). For the detailed wire and transactional contracts behind this walkthrough, see [JSON Numeric Known-Type Encoding](./docs/JSON-NUMERIC-ENCODING.MD), [JSON Numeric Known-Type Decoding](./docs/JSON-NUMERIC-DECODING.MD), and [Profiles, Limits and Results](./docs/PROFILES-AND-RESULTS.MD).

## Fixed-width integer note

The V1 contract is defined in terms of the fixed-width C++ Types `std::int8_t` through `std::uint64_t`. C++20 does not retain typedef spelling: when a fixed-width typedef is the exact same C++ Type as a native spelling or another alias such as `std::size_t`, no concept can distinguish which spelling appeared in source. The implementation therefore qualifies the actual fixed-width Type identity available on the target. Developers must use the fixed-width spelling for portable schema intent; the compiler cannot prove that spelling after alias substitution. This limitation never permits native object-layout or host-endian wire encoding.

## Dependencies

The complete mandatory/conditional ESPressio dependency tree is:

```text
EDP-Serialisation
├── EDP-System [mandatory]
├── EDP-BoundedTypes [mandatory]
│   ├── EDP-Memory [mandatory]
│   │   ├── EDP-System [mandatory]
│   │   ├── EDP-Platform [mandatory]
│   │   │   └── EDP-System [mandatory, transitive]
│   │   └── EDP-BoundedTopology [mandatory]
│   └── EDP-Platform-Portable [mandatory]
│       ├── EDP-Platform [mandatory]
│       │   └── EDP-System [mandatory]
│       └── EDP-Memory [mandatory]
│           ├── EDP-System [mandatory]
│           ├── EDP-Platform [mandatory]
│           │   └── EDP-System [mandatory, transitive]
│           └── EDP-BoundedTopology [mandatory]
├── EDP-BoundedTopology [mandatory]
├── EDP-Memory [mandatory]
│   ├── EDP-System [mandatory]
│   ├── EDP-Platform [mandatory]
│   │   └── EDP-System [mandatory, transitive]
│   └── EDP-BoundedTopology [mandatory]
├── EDP-Platform-Portable [mandatory]
│   ├── EDP-Platform [mandatory]
│   │   └── EDP-System [mandatory]
│   └── EDP-Memory [mandatory]
│       ├── EDP-System [mandatory]
│       ├── EDP-Platform [mandatory]
│       │   └── EDP-System [mandatory, transitive]
│       └── EDP-BoundedTopology [mandatory]
└── EDP-Localisation [mandatory]
    ├── EDP-System [mandatory]
    ├── EDP-Memory [mandatory]
    │   ├── EDP-System [mandatory]
    │   ├── EDP-Platform [mandatory]
    │   │   └── EDP-System [mandatory, transitive]
    │   └── EDP-BoundedTopology [mandatory]
    └── EDP-Persistence [optional through EDP-Localisation]
        condition: only when an application instantiates Localisation FilePackSource
        └── EDP-System [mandatory when imposed]
```

Serialisation consumes `EDP-BoundedTopology::BoundedIndexSet` directly for one-bit-per-FieldIdentifier presence/duplicate state, `EDP-Memory` for ByteOperations, and `EDP-Platform-Portable` for the default stateless ByteOperations provider. See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD) for ownership and dependency-direction detail.

## C++20 and PIOArduino

ESPressio requires C++20. For the policy-required PlatformIO-format ESP32 projects, install **PIOArduino Core** rather than upstream PlatformIO Core. The checked-in `platformio.ini` files select the maintained PIOArduino `platform-espressif32` package and explicitly remove older GNU language flags before adding `-std=gnu++20`, ensuring Arduino-ESP32 and ESP-IDF builds use the supported C++20 baseline.

A current PIOArduino Core can be installed using its maintained installer:

```sh
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/pioarduino/pioarduino-core-installer/pioarduino/get-platformio.py
python3 get-platformio.py
```

The resulting command remains `pio`, so the project folder names retain the policy-mandated `PlatformIO_Arduino` / `PlatformIO_ESP-IDF` convention while execution uses PIOArduino.

## Demos

`demos/type-qualification` demonstrates schema recursion, bounded values, enum certification, profile vocabulary, and compile-time qualification. `demos/json-numeric-encoding` demonstrates canonical Numeric Field ordering, Optional omission, floating encoding, Base64 Bytes, exact measurement and caller-buffer serialisation. `demos/json-numeric-decoding` demonstrates schema-ID resolution, Optional omission semantics, floating decode, strict Base64 and transactional population. `demos/json-typed-envelope` demonstrates canonical envelope emission plus transactional root Type/version validation. `demos/json-localised-text` demonstrates caller-owned Localisation/scratch, RFC5646 metadata and textual Field round-trip. `demos/cbor-numeric` demonstrates deterministic CBOR Numeric encoding/transactional decoding for both Known-Type Body and Typed Envelope roots. Every logical demo is maintained in Arduino IDE, PlatformIO Arduino, and PlatformIO ESP-IDF forms; PIOArduino is the recommended builder for the latter two forms.

## Validation

Host validation includes functional compile-time/runtime assertions plus compile-fail contracts. The README JSON Numeric round-trip source is mechanically compared with the portable example, the portable project is compiled and executed, and both PIOArduino example projects are included in the maintained embedded build matrix. The authoritative execution appliance for the current tranche is AI-AGENT-02; checked-in GitHub Actions remain reproducible workflow definitions rather than authoritative execution evidence.
