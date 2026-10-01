#include <array>
#include <cstdint>
#include <optional>

#include <ESPressio_Serialisation.hpp>

namespace Demo {

    /// Schema encoded by the JSON Numeric/Known-Type demonstration.
    struct Reading final {

        // Schema payload.

        /// Human-readable sensor label.
        ESPressio::Bounded::String<16U> Label{};

        /// Optional sample count, omitted from object output when disengaged.
        std::optional<std::uint32_t> Count{};

        /// Deterministic binary32 measurement encoded as shortest round-tripping JSON decimal.
        float Temperature = 21.5F;

        /// Arbitrary binary digest represented as canonical padded Base64 in JSON.
        ESPressio::Bounded::Bytes<8U> Digest{};

        // Schema metadata.

        /// Stable semantic identity of this demonstration Type.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U}
        };

        /// FieldSet deliberately declared out of numeric order to demonstrate canonical ordering.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Reading::Digest, 9U>,
            ESPressio::System::FieldBinding<&Reading::Label, 2U>,
            ESPressio::System::FieldBinding<&Reading::Temperature, 7U>,
            ESPressio::System::FieldBinding<&Reading::Count, 5U>
        >;

    };

    /// Builds the demonstration value and serialises it to caller-owned storage.
    ///
    /// @param output Caller-owned output bytes.
    /// @param bytesWritten Receives the exact successful output length.
    /// @return Serialisation operation result.
    ESPressio::Serialisation::SerialisationResult BuildJson(
        std::array<std::uint8_t, 128U>& output,
        std::size_t& bytesWritten
    ) noexcept {
        Reading reading{};
        const auto labelResult = reading.Label.Assign("sensor-A");
        if (labelResult != ESPressio::Bounded::StringAssignmentResult::Succeeded) {
            return {
                ESPressio::Serialisation::SerialisationStatus::InvalidArgument,
                0U,
                0U,
                {}
            };
        }

        const std::uint8_t digest[]{0xDEU, 0xADU, 0xBEU, 0xEFU};
        const auto digestResult = reading.Digest.Append(
            digest,
            sizeof(digest)
        );
        if (digestResult != ESPressio::Bounded::BytesAppendResult::Succeeded) {
            return {
                ESPressio::Serialisation::SerialisationStatus::InvalidArgument,
                0U,
                0U,
                {}
            };
        }

        const auto result = ESPressio::Serialisation::Serialise<
            ESPressio::Serialisation::Json
        >(
            reading,
            output.data(),
            output.size()
        );
        bytesWritten = result.BytesWritten;
        return result;
    }

} // Demo

#include <Arduino.h>

/// Executes the JSON encoding demonstration once after startup.
void setup() {
    Serial.begin(115200);

    std::array<std::uint8_t, 128U> output{};
    std::size_t bytesWritten = 0U;
    const auto result = Demo::BuildJson(
        output,
        bytesWritten
    );

    if (!result.IsSuccessful()) {
        Serial.printf(
            "Serialisation failed: %u\n",
            static_cast<unsigned>(result.Status)
        );
        return;
    }

    Serial.write(
        output.data(),
        bytesWritten
    );
    Serial.println();
}

/// Arduino loop is intentionally idle after the one-shot demonstration.
void loop() {
}
