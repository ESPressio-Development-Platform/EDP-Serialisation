#include <array>
#include <cstddef>
#include <cstdint>

#include <ESPressio_Serialisation.hpp>

namespace Demo {

    /// Small identified schema demonstrating deterministic CBOR Numeric execution.
    struct Packet final {

        // Schema payload.

        /// Sequence number encoded through canonical CBOR integer representation.
        std::uint16_t Sequence = 0U;

        /// Human-readable text encoded only as a CBOR text string.
        ESPressio::Bounded::String<12U> Label{};

        /// Arbitrary bytes encoded only as a CBOR byte string.
        ESPressio::Bounded::Bytes<4U> Payload{};

        // Schema metadata.

        /// Stable semantic Type identity carried only by the optional Typed Envelope root.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x06U
            }
        };

        /// FieldSet deliberately declared out of numeric order.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Packet::Payload, 9U>,
            ESPressio::System::FieldBinding<&Packet::Label, 5U>,
            ESPressio::System::FieldBinding<&Packet::Sequence, 1U>
        >;

    };

    /// Encodes and transactionally decodes one CBOR Typed Envelope.
    ///
    /// @param output Caller-owned output buffer.
    /// @param outputCapacity Writable output capacity in bytes.
    /// @param decoded Existing destination populated only after full validation succeeds.
    /// @return Number of encoded bytes, or zero when either operation fails.
    std::size_t RoundTrip(
        std::uint8_t* output,
        std::size_t outputCapacity,
        Packet& decoded
    ) noexcept {
        Packet source{};
        source.Sequence = 1000U;
        if (
            source.Label.Assign("cbor") !=
            ESPressio::Bounded::StringAssignmentResult::Succeeded
        ) {
            return 0U;
        }
        const std::uint8_t payload[]{0x01U, 0xFEU};
        if (
            source.Payload.Assign(payload, sizeof(payload)) !=
            ESPressio::Bounded::BytesAssignmentResult::Succeeded
        ) {
            return 0U;
        }

        const auto encoded = ESPressio::Serialisation::Serialise<
            ESPressio::Serialisation::Cbor,
            ESPressio::Serialisation::RootProfile::TypedEnvelope
        >(
            source,
            output,
            outputCapacity
        );
        if (!encoded.IsSuccessful()) { return 0U; }

        const auto decodedResult = ESPressio::Serialisation::Deserialise<
            ESPressio::Serialisation::Cbor,
            ESPressio::Serialisation::RootProfile::TypedEnvelope
        >(
            output,
            encoded.BytesWritten,
            decoded
        );
        return decodedResult.IsSuccessful()
            ? encoded.BytesWritten
            : 0U;
    }

} // Demo

#include <Arduino.h>

/// Executes the CBOR round-trip once after startup.
void setup() {
    Serial.begin(115200);
    std::array<std::uint8_t, 96U> output{};
    Demo::Packet decoded{};
    const auto bytes = Demo::RoundTrip(
        output.data(),
        output.size(),
        decoded
    );
    if (bytes == 0U) {
        Serial.println("CBOR round-trip failed");
        return;
    }
    Serial.printf(
        "encoded bytes=%u decoded label=%s sequence=%u payload=%u\n",
        static_cast<unsigned>(bytes),
        decoded.Label.CStr(),
        static_cast<unsigned>(decoded.Sequence),
        static_cast<unsigned>(decoded.Payload.Size())
    );
}

/// Arduino loop is intentionally idle after the one-shot demonstration.
void loop() {
}
