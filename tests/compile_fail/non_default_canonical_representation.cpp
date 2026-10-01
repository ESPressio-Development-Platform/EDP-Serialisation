#include <cstdint>
#include <type_traits>

#include <ESPressio_Serialisation.hpp>

namespace TestSupport {

    /// Result family for the negative strong-Type conversion adapters.
    enum class ConversionResult : std::uint8_t {
        /// Conversion completed successfully.
        Succeeded = 0U
    };

    /// Strong semantic source used by the negative qualification test.
    struct StrongValue final {

        // Semantic state.

        /// Test-owned semantic payload.
        std::uint32_t Value = 0U;

    };

    /// Canonical surrogate which deliberately cannot be default-constructed.
    struct NonDefaultRepresentation final {

        // Representation state.

        /// Canonical numeric payload.
        std::uint32_t Value;

        // Schema metadata.

        /// Stable semantic Type identity proving the surrogate is otherwise a valid schema Type.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U}
        };

        /// Canonical FieldSet making this representation directly serialisable except for construction.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&NonDefaultRepresentation::Value, 1U>
        >;

        // Construction.

        /// Prevents generic caller-owned temporary construction.
        NonDefaultRepresentation() = delete;

        /// Creates the representation from one explicit numeric value.
        ///
        /// @param value Canonical numeric payload.
        constexpr explicit NonDefaultRepresentation(
            std::uint32_t value
        ) noexcept :
            Value(value) {
        }

    };

} // TestSupport

namespace ESPressio::Serialisation {

    /// Declares the deliberately non-default-constructible canonical surrogate.
    template<>
    struct CanonicalRepresentation<TestSupport::StrongValue> final {

        // Representation metadata.

        /// Direct surrogate Type selected by the negative test.
        using Type = TestSupport::NonDefaultRepresentation;

    };

} // ESPressio::Serialisation

namespace ESPressio::Bounded {

    /// Supplies the forward pairwise conversion required by the adaptation seam.
    template<>
    struct TypeConversionAdapter<TestSupport::StrongValue, TestSupport::NonDefaultRepresentation> final {

        // Adapter metadata.

        /// Test-owned conversion result Type.
        using ResultType = TestSupport::ConversionResult;

        /// Indicates that this conversion is available.
        static constexpr bool IsAvailable = true;

        /// Indicates that the conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the test-owned conversion result.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for Succeeded.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Populates an already-existing representation object.
        ///
        /// @param source Strong source value.
        /// @param target Existing representation target.
        /// @return Succeeded.
        static constexpr ResultType Convert(
            const TestSupport::StrongValue& source,
            TestSupport::NonDefaultRepresentation& target
        ) noexcept {
            target.Value = source.Value;
            return ResultType::Succeeded;
        }

    };

    /// Supplies the reverse pairwise conversion required by the adaptation seam.
    template<>
    struct TypeConversionAdapter<TestSupport::NonDefaultRepresentation, TestSupport::StrongValue> final {

        // Adapter metadata.

        /// Test-owned conversion result Type.
        using ResultType = TestSupport::ConversionResult;

        /// Indicates that this conversion is available.
        static constexpr bool IsAvailable = true;

        /// Indicates that the conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the test-owned conversion result.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for Succeeded.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Populates an existing strong target from the canonical representation.
        ///
        /// @param source Canonical source representation.
        /// @param target Existing strong target.
        /// @return Succeeded.
        static constexpr ResultType Convert(
            const TestSupport::NonDefaultRepresentation& source,
            TestSupport::StrongValue& target
        ) noexcept {
            target.Value = source.Value;
            return ResultType::Succeeded;
        }

    };

} // ESPressio::Bounded

static_assert(
    ESPressio::System::SchemaType<TestSupport::NonDefaultRepresentation>,
    "The negative surrogate must otherwise satisfy the canonical System schema contract"
);

static_assert(
    !std::is_nothrow_default_constructible_v<TestSupport::NonDefaultRepresentation>,
    "The negative surrogate must fail specifically because nothrow default construction is unavailable"
);

static_assert(
    ESPressio::Serialisation::SerialisableType<TestSupport::StrongValue>,
    "A canonical surrogate which cannot be nothrow default-constructed must not qualify"
);

int main() {
    return 0;
}
