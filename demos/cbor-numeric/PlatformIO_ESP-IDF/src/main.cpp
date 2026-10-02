#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <ESPressio_Serialisation.hpp>

namespace Demo {

    /// Result family for one demonstrated CBOR round trip.
    enum class RoundTripResult : std::uint8_t {

        /// Encoding and transactional decoding both succeeded.
        Succeeded = 0U,

        /// Source preparation failed.
        SourcePreparationFailed = 1U,

        /// Serialisation failed.
        SerialisationFailed = 2U,

        /// Deserialisation failed.
        DeserialisationFailed = 3U

    };


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


    /// Encodes and transactionally decodes one CBOR root representation.
    ///
    /// @tparam TRootProfile Known-Type Body or Typed Envelope root to demonstrate.
    /// @param output Caller-owned output byte span.
    /// @param decoded Existing destination populated only after full validation succeeds.
    /// @param encodedBytes Receives the exact encoded byte count only on success.
    /// @return Strongly typed round-trip outcome.
    template<ESPressio::Serialisation::RootProfile TRootProfile>
    RoundTripResult RoundTrip(
        std::span<std::byte> output,
        Packet& decoded,
        std::size_t& encodedBytes
    ) noexcept {
        Packet source{};
        source.Sequence = 1000U;

        if (
            source.Label.Assign("cbor") !=
            ESPressio::Bounded::StringAssignmentResult::Succeeded
        ) {
            return RoundTripResult::SourcePreparationFailed;
        }

        const std::uint8_t payload[]{0x01U, 0xFEU};
        if (
            source.Payload.Assign(
                payload,
                sizeof(payload)
            ) != ESPressio::Bounded::BytesAssignmentResult::Succeeded
        ) {
            return RoundTripResult::SourcePreparationFailed;
        }

        const auto encoded = ESPressio::Serialisation::Serialise<
            ESPressio::Serialisation::Cbor,
            TRootProfile
        >(
            source,
            output
        );
        if (!encoded.IsSuccessful()) {
            return RoundTripResult::SerialisationFailed;
        }

        const auto decodedResult = ESPressio::Serialisation::Deserialise<
            ESPressio::Serialisation::Cbor,
            TRootProfile
        >(
            std::span<const std::byte>{
                output.data(),
                encoded.BytesWritten
            },
            decoded
        );
        if (!decodedResult.IsSuccessful()) {
            return RoundTripResult::DeserialisationFailed;
        }

        encodedBytes = encoded.BytesWritten;
        return RoundTripResult::Succeeded;
    }

} // Demo

#include <cstdio>

/// Executes both supported CBOR Numeric root profiles once from the ESP-IDF application entry point.
extern "C" void app_main() {
    std::array<std::byte, 96U> output{};
    Demo::Packet knownTypeDecoded{};
    Demo::Packet envelopeDecoded{};
    std::size_t knownTypeBytes = 0U;
    std::size_t envelopeBytes = 0U;

    const auto knownTypeResult = Demo::RoundTrip<
        ESPressio::Serialisation::RootProfile::KnownTypeBody
    >(
        output,
        knownTypeDecoded,
        knownTypeBytes
    );
    if (knownTypeResult != Demo::RoundTripResult::Succeeded) {
        std::printf("CBOR Known-Type Body round-trip failed\n");
        return;
    }

    const auto envelopeResult = Demo::RoundTrip<
        ESPressio::Serialisation::RootProfile::TypedEnvelope
    >(
        output,
        envelopeDecoded,
        envelopeBytes
    );
    if (envelopeResult != Demo::RoundTripResult::Succeeded) {
        std::printf("CBOR Typed Envelope round-trip failed\n");
        return;
    }

    std::printf(
        "known-type=%u envelope=%u label=%s sequence=%u payload=%u\n",
        static_cast<unsigned>(knownTypeBytes),
        static_cast<unsigned>(envelopeBytes),
        envelopeDecoded.Label.CStr(),
        static_cast<unsigned>(envelopeDecoded.Sequence),
        static_cast<unsigned>(envelopeDecoded.Payload.Size())
    );
}
