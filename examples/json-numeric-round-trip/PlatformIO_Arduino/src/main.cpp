#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include <Arduino.h>
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

// Arduino lifecycle.

/// Executes the JSON Numeric round-trip example once after startup.
void setup() {
    Serial.begin(115200);

    Example::SensorReading source{};
    const auto labelResult = source.SensorName.Assign("sensor-A");
    if (labelResult != ESPressio::Bounded::StringAssignmentResult::Succeeded) {
        Serial.println("Unable to populate the bounded sensor name.");
        return;
    }

    source.TemperatureCentiCelsius = 2350U;
    source.SampleCount.reset();

    const auto measurement = ESPressio::Serialisation::Measure<
        ESPressio::Serialisation::Json
    >(source);
    if (!measurement.IsSuccessful()) {
        Serial.printf(
            "Measurement failed with status %u.\n",
            static_cast<unsigned>(measurement.Status)
        );
        return;
    }

    std::array<std::byte, 128U> output{};
    if (measurement.RequiredBytes > output.size()) {
        Serial.println("The example output buffer is unexpectedly too small.");
        return;
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
        Serial.printf(
            "Serialisation failed with status %u.\n",
            static_cast<unsigned>(serialisation.Status)
        );
        return;
    }

    if (serialisation.RequiredBytes != measurement.RequiredBytes
        || serialisation.BytesWritten != measurement.RequiredBytes) {
        Serial.println("Measure and Serialise disagree about the representation size.");
        return;
    }

    Serial.println("Encoded JSON:");
    Serial.write(
        reinterpret_cast<const std::uint8_t*>(output.data()),
        serialisation.BytesWritten
    );
    Serial.println();

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
        Serial.printf(
            "Deserialisation failed with status %u at byte %u.\n",
            static_cast<unsigned>(deserialisation.Status),
            static_cast<unsigned>(deserialisation.Detail.ByteOffset)
        );
        return;
    }

    if (!Example::IsEquivalent(
        source,
        decoded
    )) {
        Serial.println("The decoded reading does not match the source reading.");
        return;
    }

    Serial.printf(
        "Decoded reading: sensor=%s temperature-centi-c=%u sample-count=%s\n",
        decoded.SensorName.CStr(),
        static_cast<unsigned>(decoded.TemperatureCentiCelsius),
        decoded.SampleCount.has_value() ? "present" : "absent"
    );
    Serial.println("Round trip succeeded.");
}

/// Arduino loop is intentionally idle after the one-shot example.
void loop() {
}
