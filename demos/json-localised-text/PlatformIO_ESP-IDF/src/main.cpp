#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include <ESPressio_Serialisation.hpp>

namespace Demo {

    /// Result shape returned by the compact demonstration Resolver.
    struct FieldNameResult final {

        /// Forward Field-name lookup outcome.
        ESPressio::Localisation::LocalisationStatus Status =
            ESPressio::Localisation::LocalisationStatus::Success;

        /// Orthogonal facts such as caller scratch exhaustion.
        ESPressio::Localisation::LocalisationFacts Facts{};

        /// UTF-8 Field-name bytes copied to caller scratch.
        std::size_t BytesWritten = 0U;

        /// Complete UTF-8 Field-name byte count required.
        std::size_t RequiredBytes = 0U;
    };

    /// Field presentation identity accepted by the demonstration Resolver.
    struct FieldPresentationIdentifier final {

        /// Owning schema Type identity.
        ESPressio::System::TypeIdentifier Type;

        /// Type-local Field identity.
        ESPressio::System::FieldIdentifier Field;
    };

    /// Small stateless Localisation Resolver facade used to keep the demo focused on Serialisation.
    class Resolver final {
    public:

        /// Field presentation identity consumed by Serialisation's LocalisedText policy.
        using FieldPresentationIdentifier = Demo::FieldPresentationIdentifier;

        /// Validates the caller-supplied requested/terminal language context.
        [[nodiscard]] ESPressio::Localisation::ValidationResult ValidateContext(
            const ESPressio::Localisation::LocalisationContext& context
        ) const noexcept {
            return {
                ESPressio::Localisation::ValidateLocalisationContext(context).Status ==
                        ESPressio::Localisation::LocalisationContextValidationStatus::Succeeded
                    ? ESPressio::Localisation::ValidationStatus::Success
                    : ESPressio::Localisation::ValidationStatus::InvalidArgument
            };
        }

        /// Resolves the single demo Field to the textual key `Temperature`.
        [[nodiscard]] FieldNameResult ResolveFieldName(
            const ESPressio::Localisation::LocalisationContext& context,
            const FieldPresentationIdentifier& field,
            ESPressio::Localisation::WritableTextView destination,
            ESPressio::Localisation::TextOutputMode outputMode
        ) const noexcept {
            static_cast<void>(context);
            static_cast<void>(field);
            static_cast<void>(outputMode);
            constexpr char Name[] = "Temperature";
            FieldNameResult result{};
            result.RequiredBytes = sizeof(Name) - 1U;
            result.BytesWritten = destination.Capacity < result.RequiredBytes
                ? destination.Capacity
                : result.RequiredBytes;
            for (std::size_t index = 0U; index < result.BytesWritten; ++index) {
                destination.Data[index] = Name[index];
            }
            if (result.BytesWritten != result.RequiredBytes) {
                result.Facts.Set(ESPressio::Localisation::LocalisationFact::BufferTooSmall);
            }
            return result;
        }

        /// Reverse-resolves one textual key within an explicit language context.
        [[nodiscard]] ESPressio::Localisation::FieldIdentifierResolutionResult ResolveFieldIdentifier(
            const ESPressio::Localisation::LocalisationContext& context,
            ESPressio::System::TypeIdentifier type,
            ESPressio::Localisation::TextView fieldName
        ) const noexcept {
            static_cast<void>(context);
            static_cast<void>(type);
            return Resolve(fieldName);
        }

        /// Reverse-resolves one textual key across the demo's complete language universe.
        [[nodiscard]] ESPressio::Localisation::FieldIdentifierResolutionResult ResolveFieldIdentifierAcrossLanguages(
            ESPressio::System::TypeIdentifier type,
            ESPressio::Localisation::TextView fieldName
        ) const noexcept {
            static_cast<void>(type);
            return Resolve(fieldName);
        }

    private:

        /// Performs the exact byte comparison shared by both reverse-resolution entry points.
        [[nodiscard]] static ESPressio::Localisation::FieldIdentifierResolutionResult Resolve(
            ESPressio::Localisation::TextView fieldName
        ) noexcept {
            constexpr char Name[] = "Temperature";
            if (fieldName.Size != sizeof(Name) - 1U) {
                return {
                    ESPressio::Localisation::FieldIdentifierResolutionStatus::NotFound,
                    std::nullopt
                };
            }
            for (std::size_t index = 0U; index < fieldName.Size; ++index) {
                if (fieldName.Data[index] != Name[index]) {
                    return {
                        ESPressio::Localisation::FieldIdentifierResolutionStatus::NotFound,
                        std::nullopt
                    };
                }
            }
            return {
                ESPressio::Localisation::FieldIdentifierResolutionStatus::Success,
                ESPressio::System::FieldIdentifier{0U}
            };
        }
    };

    /// Identified schema whose Field is represented through a localised textual key.
    struct Reading final {

        // Schema payload.

        /// Example temperature payload.
        std::uint16_t Temperature = 0U;

        // Schema metadata.

        /// Stable semantic Type identity for the demo Reading.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x05U
            }
        };
        /// Canonical schema binding of `Temperature` to FieldIdentifier zero.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Reading::Temperature, 0U>
        >;
    };

    /// Encodes and transactionally decodes one LocalisedText JSON body.
    ///
    /// @param output Caller-owned JSON output storage.
    /// @param outputCapacity Writable output capacity in bytes.
    /// @param decoded Destination populated only after complete transactional validation.
    /// @return Encoded byte count on success, otherwise zero.
    std::size_t RoundTrip(
        std::uint8_t* output,
        std::size_t outputCapacity,
        Reading& decoded
    ) noexcept {
        constexpr auto English =
            ESPressio::Localisation::LanguageIdentifierView::Validate("en-GB");
        static_assert(English.IsValuePresent);
        const ESPressio::Localisation::LocalisationContext context{
            English.Value,
            English.Value
        };
        Resolver resolver{};
        std::array<char, 24U> fieldScratchStorage{};
        std::array<char, 24U> comparisonScratchStorage{};
        const ESPressio::Localisation::WritableTextView fieldScratch{
            fieldScratchStorage.data(), fieldScratchStorage.size()
        };
        const ESPressio::Localisation::WritableTextView comparisonScratch{
            comparisonScratchStorage.data(), comparisonScratchStorage.size()
        };

        Reading source{};
        source.Temperature = 23U;
        const auto encoded = ESPressio::Serialisation::Serialise<
            ESPressio::Serialisation::Json,
            ESPressio::Serialisation::RootProfile::KnownTypeBody,
            ESPressio::Serialisation::FieldProfile::LocalisedText
        >(
            source,
            output,
            outputCapacity,
            resolver,
            context,
            fieldScratch,
            comparisonScratch
        );
        if (!encoded.IsSuccessful()) { return 0U; }

        const auto decodedResult = ESPressio::Serialisation::Deserialise<
            ESPressio::Serialisation::Json,
            ESPressio::Serialisation::RootProfile::KnownTypeBody,
            ESPressio::Serialisation::FieldProfile::LocalisedText
        >(
            output,
            encoded.BytesWritten,
            decoded,
            resolver,
            context,
            fieldScratch
        );
        return decodedResult.IsSuccessful() ? encoded.BytesWritten : 0U;
    }

} // Demo


#include <cstdio>

/// Runs one LocalisedText round-trip from the ESP-IDF application entry point.
extern "C" void app_main() {
    std::array<std::uint8_t, 128U> output{};
    Demo::Reading decoded{};
    const auto bytes = Demo::RoundTrip(output.data(), output.size(), decoded);
    if (bytes == 0U) {
        std::printf("LocalisedText round-trip failed\n");
        return;
    }
    std::printf(
        "%.*s\ndecoded temperature=%u\n",
        static_cast<int>(bytes),
        reinterpret_cast<const char*>(output.data()),
        static_cast<unsigned>(decoded.Temperature)
    );
}
