#include <array>
#include <cstdint>

#include <ESPressio_Serialisation.hpp>
#include <span>

namespace Demo {

    /// Small identified schema used to demonstrate the explicit JSON Typed Envelope profile.
    struct Packet final {

        // Schema payload.

        /// Human-readable packet label.
        ESPressio::Bounded::String<12U> Label{};

        /// Example sequence number.
        std::uint16_t Sequence = 0U;

        // Schema metadata.

        /// Stable semantic Type identity carried by the Typed Envelope root.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x04U}
        };

        /// FieldSet deliberately declared out of numeric order.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Packet::Label, 5U>,
            ESPressio::System::FieldBinding<&Packet::Sequence, 1U>
        >;

    };

    /// Encodes and transactionally decodes one Typed Envelope.
    ///
    /// @param output Caller-owned output buffer.
    /// @param outputCapacity Writable output capacity.
    /// @param decoded Destination populated only after full envelope validation succeeds.
    /// @return Number of encoded bytes, or zero when either operation fails.
    std::size_t RoundTrip(
        std::uint8_t* output,
        std::size_t outputCapacity,
        Packet& decoded
    ) noexcept {
        Packet source{};
        if (source.Label.Assign(
            "typed",
            5U
        ) != ESPressio::Bounded::StringAssignmentResult::Succeeded) {
            return 0U;
        }
        source.Sequence = 42U;

        const auto encoded = ESPressio::Serialisation::Serialise<
            ESPressio::Serialisation::Json,
            ESPressio::Serialisation::RootProfile::TypedEnvelope
        >(
        source,
        std::as_writable_bytes(
            std::span{
                output,
                outputCapacity
            }
        )
    );
        if (!encoded.IsSuccessful()) { return 0U; }

        const auto decodedResult = ESPressio::Serialisation::Deserialise<
            ESPressio::Serialisation::Json,
            ESPressio::Serialisation::RootProfile::TypedEnvelope
        >(
        std::as_bytes(
            std::span{
                output,
                encoded.BytesWritten
            }
        ),
        decoded
    );
        return decodedResult.IsSuccessful()
            ? encoded.BytesWritten
            : 0U;
    }

} // Demo


#include <cstdio>

/// Executes the Typed Envelope round-trip once from the ESP-IDF application entry point.
extern "C" void app_main() {
    std::array<std::uint8_t, 160U> output{};
    Demo::Packet decoded{};
    const auto bytes = Demo::RoundTrip(
        output.data(),
        output.size(),
        decoded
    );
    if (bytes == 0U) {
        std::printf("Typed Envelope round-trip failed\n");
        return;
    }
    std::printf(
        "%.*s\ndecoded label=%s sequence=%u\n",
        static_cast<int>(bytes),
        reinterpret_cast<const char*>(output.data()),
        decoded.Label.CStr(),
        static_cast<unsigned>(decoded.Sequence)
    );
}
