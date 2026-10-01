#include <cstdint>

#include <ESPressio_Serialisation.hpp>

/// Result family for the negative canonical adaptation seam.
enum class SemanticConversionResult : std::uint8_t {
    /// Conversion completed successfully.
    Succeeded = 0U
};

/// Strong semantic Type which can exist explicitly but cannot seed generic transactional validation storage.
struct NonConstructibleSemantic final {

    // Semantic state.

    /// Strong fixed-width payload.
    std::uint32_t Value;

    // Construction.

    /// Prevents generic default construction.
    NonConstructibleSemantic() = delete;

    /// Creates one explicit semantic value.
    constexpr explicit NonConstructibleSemantic(
        std::uint32_t value
    ) noexcept :
        Value(value) {
    }

    /// Prevents generic copy-seeded transactional validation.
    NonConstructibleSemantic(const NonConstructibleSemantic&) = delete;

    /// Allows explicit move construction without supplying a generic validation seed.
    constexpr NonConstructibleSemantic(NonConstructibleSemantic&&) noexcept = default;

};

namespace ESPressio::Serialisation {

    /// Declares the direct fixed-width canonical surrogate of the negative semantic Type.
    template<>
    struct CanonicalRepresentation<NonConstructibleSemantic> final {

        /// Direct fixed-width canonical representation.
        using Type = std::uint32_t;

    };

} // ESPressio::Serialisation

namespace ESPressio::Bounded {

    /// Supplies the complete forward adapter so construction is the only qualification defect.
    template<>
    struct TypeConversionAdapter<NonConstructibleSemantic, std::uint32_t> final {
        using ResultType = SemanticConversionResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;
        static constexpr bool IsSuccessful(ResultType result) noexcept {
            return result == ResultType::Succeeded;
        }
        static constexpr ResultType Convert(
            const NonConstructibleSemantic& source,
            std::uint32_t& target
        ) noexcept {
            target = source.Value;
            return ResultType::Succeeded;
        }
    };

    /// Supplies the complete reverse adapter so construction is the only qualification defect.
    template<>
    struct TypeConversionAdapter<std::uint32_t, NonConstructibleSemantic> final {
        using ResultType = SemanticConversionResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;
        static constexpr bool IsSuccessful(ResultType result) noexcept {
            return result == ResultType::Succeeded;
        }
        static constexpr ResultType Convert(
            const std::uint32_t& source,
            NonConstructibleSemantic& target
        ) noexcept {
            target.Value = source;
            return ResultType::Succeeded;
        }
    };

} // ESPressio::Bounded

static_assert(
    ESPressio::Serialisation::SerialisableType<NonConstructibleSemantic>,
    "Adapted semantic Types require nothrow default or nothrow copy construction for transactional decode validation"
);

/// Compile-fail harness entry point; the preceding assertion must reject the fixture first.
int main() {
    return 0;
}
