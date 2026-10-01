#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <ESPressio_Serialisation.hpp>

namespace TestSupport {

    /// Demonstrates explicit fixed-width enum certification.
    enum class Mode : std::uint8_t {
        /// Demonstration disabled state.
        Off = 0U,
        /// Demonstration automatic state.
        Automatic = 1U
    };

    /// Result family for the test-owned strong counter conversion adapters.
    enum class StrongCounterConversionResult : std::uint8_t {
        /// Conversion completed successfully.
        Succeeded = 0U
    };

    /// Nested schema used to prove recursive Field qualification.
    struct Nested final {

        // Schema payload.

        /// Example fixed-width numeric Field.
        std::uint16_t Value = 0U;

        // Schema metadata.

        /// Stable semantic Type identity owned by the test schema.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U}
        };

        /// Canonical FieldSet binding the numeric payload to FieldIdentifier 9.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Nested::Value, 9U>
        >;

    };

    /// Composite schema used to prove bounded, Optional, and nested schema recursion.
    struct Payload final {

        // Schema payload.

        /// Example bounded UTF-8 text Field.
        ESPressio::Bounded::String<16U> Name{};

        /// Example Optional fixed-width numeric Field.
        std::optional<std::uint32_t> Count{};

        /// Example recursively nested schema Field.
        Nested Child{};

        // Schema metadata.

        /// Stable semantic Type identity owned by the test schema.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U}
        };

        /// Canonical FieldSet deliberately declared out of numeric order.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Payload::Name, 7U>,
            ESPressio::System::FieldBinding<&Payload::Count, 2U>,
            ESPressio::System::FieldBinding<&Payload::Child, 4U>
        >;

    };

    /// Schema containing an unsupported dynamic STL owner.
    struct UnsupportedPayload final {

        // Schema payload.

        /// Dynamic String intentionally outside the V1 serialisable universe.
        std::string Dynamic{};

        // Schema metadata.

        /// Stable semantic Type identity owned by the negative test schema.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x03U}
        };

        /// Canonical FieldSet exposing the unsupported dynamic Field.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&UnsupportedPayload::Dynamic, 1U>
        >;

    };

    /// Strong semantic counter adapted to an exact uint32_t canonical representation.
    struct StrongCounter final {

        // Semantic state.

        /// Strong counter value retained by the semantic Type.
        std::uint32_t Value = 0U;

    };

    /// Schema proving that recursively valid Fields may themselves use canonical adaptation.
    struct AdaptedPayload final {

        // Schema payload.

        /// Strong semantic Field using the test-owned canonical conversion seam.
        StrongCounter Counter{};

        // Schema metadata.

        /// Stable semantic Type identity owned by the adapted-Field schema.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x04U}
        };

        /// Canonical FieldSet exposing the adapted strong semantic Field.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&AdaptedPayload::Counter, 3U>
        >;

    };

} // TestSupport

namespace ESPressio::Serialisation {

    /// Certifies the stable numeric domain of the test Mode enum.
    template<>
    struct EnumSerialisationTraits<TestSupport::Mode> final {

        // Certification metadata.

        /// Exact fixed-width numeric representation of Mode.
        using UnderlyingType = std::uint8_t;

    };

    /// Declares the direct canonical surrogate of the test strong counter Type.
    template<>
    struct CanonicalRepresentation<TestSupport::StrongCounter> final {

        // Representation metadata.

        /// Exact direct serialisable surrogate used by the counter adapters.
        using Type = std::uint32_t;

    };

} // ESPressio::Serialisation

namespace ESPressio::Bounded {

    /// Converts the test strong counter to its canonical fixed-width representation.
    template<>
    struct TypeConversionAdapter<TestSupport::StrongCounter, std::uint32_t> final {

        // Adapter metadata.

        /// Test-owned operation result Type.
        using ResultType = TestSupport::StrongCounterConversionResult;

        /// Indicates that this pairwise conversion is available.
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

        /// Copies the semantic counter value into its canonical representation.
        ///
        /// @param source Strong semantic counter being represented.
        /// @param target Caller-owned canonical numeric target.
        /// @return Succeeded because the representation is lossless.
        static constexpr ResultType Convert(
            const TestSupport::StrongCounter& source,
            std::uint32_t& target
        ) noexcept {
            target = source.Value;
            return ResultType::Succeeded;
        }

    };

    /// Converts the canonical fixed-width representation back to the test strong counter.
    template<>
    struct TypeConversionAdapter<std::uint32_t, TestSupport::StrongCounter> final {

        // Adapter metadata.

        /// Test-owned operation result Type.
        using ResultType = TestSupport::StrongCounterConversionResult;

        /// Indicates that this pairwise conversion is available.
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

        /// Reconstructs the semantic counter from its canonical numeric representation.
        ///
        /// @param source Canonical numeric representation being interpreted.
        /// @param target Caller-owned semantic counter populated on success.
        /// @return Succeeded because the representation is lossless.
        static constexpr ResultType Convert(
            const std::uint32_t& source,
            TestSupport::StrongCounter& target
        ) noexcept {
            target.Value = source;
            return ResultType::Succeeded;
        }

    };

} // ESPressio::Bounded

/// Runs the complete Serialisation foundation host contract suite.
int main() {
    using namespace ESPressio::Serialisation;

    static_assert(SerialisableType<bool>);
    static_assert(SerialisableType<std::int8_t>);
    static_assert(SerialisableType<std::int16_t>);
    static_assert(SerialisableType<std::int32_t>);
    static_assert(SerialisableType<std::int64_t>);
    static_assert(SerialisableType<std::uint8_t>);
    static_assert(SerialisableType<std::uint16_t>);
    static_assert(SerialisableType<std::uint32_t>);
    static_assert(SerialisableType<std::uint64_t>);
    static_assert(SerialisableType<float>);
    static_assert(SerialisableType<double>);
    static_assert(!SerialisableType<long double>);
    static_assert(SerialisableType<TestSupport::Mode>);
    static_assert(SerialisableType<ESPressio::Bounded::String<32U>>);
    static_assert(SerialisableType<ESPressio::Bounded::Bytes<32U>>);
    static_assert(SerialisableType<ESPressio::Bounded::Vector<std::uint16_t, 8U>>);
    static_assert(SerialisableType<std::array<std::uint8_t, 8U>>);
    static_assert(SerialisableType<std::uint16_t[4U]>);
    static_assert(SerialisableType<std::optional<std::uint32_t>>);
    static_assert(!SerialisableType<std::optional<std::optional<std::uint32_t>>>);
    static_assert(!SerialisableType<std::string>);
    static_assert(!SerialisableType<std::vector<std::uint32_t>>);
    static_assert(!SerialisableType<std::uint32_t*>);
    static_assert(SerialisableType<TestSupport::Nested>);
    static_assert(SerialisableType<TestSupport::Payload>);
    static_assert(!SerialisableType<TestSupport::UnsupportedPayload>);
    static_assert(SerialisableType<TestSupport::StrongCounter>);
    static_assert(SerialisableType<TestSupport::AdaptedPayload>);
    static_assert(SerialisableType<ESPressio::System::TypeIdentifier>);
    static_assert(SerialisableType<ESPressio::System::FieldIdentifier>);
    static_assert(DefaultParserLimits::MaximumNestingDepth == 32U);
    static_assert(DefaultParserLimits::MaximumSkippedContainerItems == 1024U);
    static_assert(TypedEnvelopeVersion == 1U);

    MeasurementResult measurement{};
    assert(measurement.IsSuccessful());
    SerialisationResult serialisation{};
    assert(serialisation.IsSuccessful());
    DeserialisationResult deserialisation{};
    assert(deserialisation.IsSuccessful());

    ESPressio::System::TypeIdentifier identifier{
        ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x2AU}
    };
    ESPressio::System::TypeIdentifier::Storage bytes{};
    const auto forward = ESPressio::Bounded::TypeConversionAdapter<
        ESPressio::System::TypeIdentifier,
        ESPressio::System::TypeIdentifier::Storage
    >::Convert(
        identifier,
        bytes
    );
    assert((ESPressio::Bounded::IsTypeConversionSuccessful<
        ESPressio::System::TypeIdentifier,
        ESPressio::System::TypeIdentifier::Storage
    >(forward)));
    assert(bytes[7U] == 0x2AU);

    return 0;
}
