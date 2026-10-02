#include <cstdint>
#include <optional>

#include <ESPressio_Serialisation.hpp>
#include <span>

namespace Demo {

    /// Schema decoded by the JSON Numeric/Known-Type demonstration.
    struct Reading final {

        // Schema payload.

        /// Human-readable sensor label.
        ESPressio::Bounded::String<16U> Label{};

        /// Optional sample count, disengaged when the Field is absent or null.
        std::optional<std::uint32_t> Count{99U};

        /// Deterministic binary32 measurement decoded from JSON numeric text.
        float Temperature = 0.0F;

        /// Arbitrary binary digest decoded from strict padded RFC 4648 Base64.
        ESPressio::Bounded::Bytes<8U> Digest{};

        // Schema metadata.

        /// Stable semantic identity of this demonstration Type.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x03U}
        };

        /// FieldSet deliberately declared independently of the incoming JSON member order.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Reading::Digest, 9U>,
            ESPressio::System::FieldBinding<&Reading::Label, 2U>,
            ESPressio::System::FieldBinding<&Reading::Temperature, 7U>,
            ESPressio::System::FieldBinding<&Reading::Count, 5U>
        >;

    };

    /// Decodes one complete JSON document transactionally into the supplied Reading.
    ///
    /// @param reading Existing destination populated only after complete validation succeeds.
    /// @return Operation-specific deserialisation result.
    ESPressio::Serialisation::DeserialisationResult DecodeReading(
        Reading& reading
    ) noexcept {
        constexpr char JsonDocument[] =
            "{\"9\":\"3q2+7w==\",\"7\":23.75,\"2\":\"sensor-B\"}";

        return ESPressio::Serialisation::Deserialise<
            ESPressio::Serialisation::Json
        >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(JsonDocument),
                sizeof(JsonDocument) - 1U
            }
        ),
        reading
    );
    }

} // Demo


#include <cstdio>

/// Executes the JSON decoding demonstration once from the ESP-IDF application entry point.
extern "C" void app_main() {
    Demo::Reading reading{};
    const auto result = Demo::DecodeReading(reading);
    if (!result.IsSuccessful()) {
        std::printf(
            "Deserialisation failed: %u at byte %u\n",
            static_cast<unsigned>(result.Status),
            static_cast<unsigned>(result.Detail.ByteOffset)
        );
        return;
    }

    std::printf(
        "label=%s temperature=%.2f count=%s digest-bytes=%u\n",
        reading.Label.CStr(),
        static_cast<double>(reading.Temperature),
        reading.Count.has_value() ? "engaged" : "absent",
        static_cast<unsigned>(reading.Digest.Size())
    );
}
