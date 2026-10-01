#include <cstdint>

#include <ESPressio_Serialisation.hpp>

/// Result family for the intentionally incomplete conversion integration.
enum class ConversionResult : std::uint8_t {
    /// Forward conversion completed successfully.
    Succeeded = 0U
};

/// Strong semantic Type whose reverse canonical adapter is intentionally absent.
struct StrongValue final {

    // Semantic state.

    /// Strong numeric value represented by the fixture.
    std::uint16_t Value = 0U;

};

namespace ESPressio::Serialisation {

    /// Declares the intended direct surrogate so the missing reverse adapter is the only defect.
    template<>
    struct CanonicalRepresentation<StrongValue> final {

        // Representation metadata.

        /// Direct fixed-width surrogate selected by the fixture.
        using Type = std::uint16_t;

    };

} // ESPressio::Serialisation

namespace ESPressio::Bounded {

    /// Supplies only the forward conversion so complete adaptation must remain unavailable.
    template<>
    struct TypeConversionAdapter<StrongValue, std::uint16_t> final {

        // Adapter metadata.

        /// Fixture-owned conversion result Type.
        using ResultType = ConversionResult;

        /// Indicates that the forward conversion is available.
        static constexpr bool IsAvailable = true;

        /// Indicates that the forward conversion is noexcept.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the fixture-owned conversion result.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for Succeeded.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Copies the strong fixture value into its direct numeric surrogate.
        ///
        /// @param source Strong semantic fixture value.
        /// @param target Caller-owned canonical surrogate.
        /// @return Succeeded because the forward representation is lossless.
        static constexpr ResultType Convert(
            const StrongValue& source,
            std::uint16_t& target
        ) noexcept {
            target = source.Value;
            return ResultType::Succeeded;
        }

    };

} // ESPressio::Bounded

static_assert(
    ESPressio::Serialisation::SerialisableType<StrongValue>,
    "Canonical adaptation must require both conversion directions"
);

/// Compile-fail harness entry point; the preceding assertion must reject the fixture first.
int main() {
    return 0;
}
