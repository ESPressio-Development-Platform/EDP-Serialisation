#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <vector>

#include <ESPressio_Serialisation.hpp>
#include <span>

#include "TestPacks.hpp"

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


    /// Result family for a deliberately fallible strong-Type conversion seam.
    enum class FallibleStrongConversionResult : std::uint8_t {
        /// Conversion completed successfully.
        Succeeded = 0U,
        /// Source value was deliberately rejected by the test adapter.
        Rejected = 1U
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


    /// Strong semantic value whose forward adapter can reject one sentinel value.
    struct FallibleStrong final {

        // Semantic state.

        /// Test-owned semantic counter value.
        std::uint32_t Value = 0U;

    };

    /// Schema matching the real EDP-Localisation generated Type/Field test pack.
    struct LocalisedReading final {

        // Schema payload.

        /// Temperature value presented through localised Field name `Temperature`.
        std::uint32_t Temperature = 0U;

        // Schema metadata.

        /// Stable Type identity present in the Localisation generated pack fixture.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0xABU, 0xCDU, 0xEFU
            }
        };

        /// Canonical FieldSet matching generated FieldIdentifier zero.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&LocalisedReading::Temperature, 0U>
        >;

    };

    /// Two-Field schema used to exercise textual presentation collisions.
    struct LocalisedPair final {

        // Schema payload.

        /// First test value.
        std::uint32_t First = 1U;

        /// Second test value.
        std::uint32_t Second = 2U;

        // Schema metadata.

        /// Stable test-owned Type identity.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x06U
            }
        };

        /// Canonical two-Field schema.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&LocalisedPair::First, 0U>,
            ESPressio::System::FieldBinding<&LocalisedPair::Second, 1U>
        >;

    };

    /// Nested LocalisedText leaf used to prove recursive Field-policy propagation.
    struct LocalisedLeaf final {

        /// Leaf numeric payload.
        std::uint16_t Value = 0U;

        /// Stable leaf Type identity.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x08U
            }
        };

        /// Single Field binding using identifier zero.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&LocalisedLeaf::Value, 0U>
        >;

    };

    /// Root schema containing another LocalisedText schema object.
    struct LocalisedNestedRoot final {

        /// Nested schema payload.
        LocalisedLeaf Child{};

        /// Stable root Type identity.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x09U
            }
        };

        /// Single nested Field binding using identifier zero.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&LocalisedNestedRoot::Child, 0U>
        >;

    };

    /// Empty schema used to prove unknown textual Field skipping without required-Field noise.
    struct LocalisedEmpty final {

        /// Stable test-owned Type identity.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x07U
            }
        };

        /// Explicit zero-Field schema.
        using Fields = ESPressio::System::FieldSet<>;

    };

    /// Behaviour selected for the focused LocalisedText resolver test double.
    enum class TestLocalisationMode : std::uint8_t {
        /// Distinct forward/reverse names map normally.
        Normal = 0U,
        /// Every forward Field maps to the same name.
        Collision = 1U,
        /// Forward Field names collide with reserved RFC5646 metadata.
        Reserved = 2U,
        /// Reverse lookup reports conclusively absent names.
        NotFound = 3U,
        /// Cross-language reverse lookup reports conflicting identities.
        Ambiguous = 4U,
        /// Resolver validation reports an unavailable provider/pack dependency.
        Unavailable = 5U
    };

    /// Minimal Field presentation identity matching the Resolver surface consumed by Serialisation.
    struct TestFieldPresentationIdentifier final {

        /// Owning schema Type identity.
        ESPressio::System::TypeIdentifier Type;

        /// Type-local Field identity.
        ESPressio::System::FieldIdentifier Field;

    };

    /// Focused forward-lookup result used by the LocalisedText resolver test double.
    struct TestLocalisationResolveResult final {

        /// Lookup status.
        ESPressio::Localisation::LocalisationStatus Status =
            ESPressio::Localisation::LocalisationStatus::Success;

        /// Successful-resolution facts including scratch-capacity reporting.
        ESPressio::Localisation::LocalisationFacts Facts{};

        /// Bytes copied to caller scratch.
        std::size_t BytesWritten = 0U;

        /// Complete UTF-8 representation size.
        std::size_t RequiredBytes = 0U;

    };

    /// Small resolver test double used only for impossible-valid-pack failure states.
    class TestLocalisationResolver final {
    public:

        /// Field presentation identity accepted by ResolveFieldName.
        using FieldPresentationIdentifier = TestFieldPresentationIdentifier;

    private:

        /// Selected deterministic test behaviour.
        TestLocalisationMode _mode = TestLocalisationMode::Normal;

        /// Compares caller text to one exact ASCII literal.
        [[nodiscard]] static bool IsTextEqual(
            ESPressio::Localisation::TextView text,
            const char* expected,
            std::size_t expectedSize
        ) noexcept {
            if (text.Size != expectedSize) { return false; }
            for (std::size_t index = 0U; index < text.Size; ++index) {
                if (text.Data[index] != expected[index]) { return false; }
            }
            return true;
        }

    public:

        /// Creates one deterministic resolver test double.
        explicit TestLocalisationResolver(
            TestLocalisationMode mode
        ) noexcept :
            _mode(mode) {
        }

        /// Accepts every syntactically valid context supplied by the operation tests.
        [[nodiscard]] ESPressio::Localisation::ValidationResult ValidateContext(
            const ESPressio::Localisation::LocalisationContext& context
        ) const noexcept {
            static_cast<void>(context);
            return {
                _mode == TestLocalisationMode::Unavailable
                    ? ESPressio::Localisation::ValidationStatus::ProviderUnavailable
                    : ESPressio::Localisation::ValidationStatus::Success
            };
        }

        /// Resolves one deterministic test Field presentation into caller-owned scratch.
        [[nodiscard]] TestLocalisationResolveResult ResolveFieldName(
            const ESPressio::Localisation::LocalisationContext& context,
            const FieldPresentationIdentifier& field,
            ESPressio::Localisation::WritableTextView destination,
            ESPressio::Localisation::TextOutputMode outputMode
        ) const noexcept {
            static_cast<void>(context);
            static_cast<void>(outputMode);
            if (_mode == TestLocalisationMode::Unavailable) {
                TestLocalisationResolveResult failure{};
                failure.Status = ESPressio::Localisation::LocalisationStatus::ProviderUnavailable;
                return failure;
            }
            const char* name = field.Field.Value() == 0U ? "Alpha" : "Beta";
            std::size_t nameSize = field.Field.Value() == 0U ? 5U : 4U;
            if (_mode == TestLocalisationMode::Collision) {
                name = "Same";
                nameSize = 4U;
            } else if (_mode == TestLocalisationMode::Reserved) {
                name = "RFC5646";
                nameSize = 7U;
            }

            const auto copySize = destination.Capacity < nameSize
                ? destination.Capacity
                : nameSize;
            for (std::size_t index = 0U; index < copySize; ++index) {
                destination.Data[index] = name[index];
            }
            TestLocalisationResolveResult result{};
            result.BytesWritten = copySize;
            result.RequiredBytes = nameSize;
            if (copySize != nameSize) {
                result.Facts.Set(
                    ESPressio::Localisation::LocalisationFact::BufferTooSmall
                );
            }
            return result;
        }

        /// Resolves one deterministic textual name in one explicit language context.
        [[nodiscard]] ESPressio::Localisation::FieldIdentifierResolutionResult ResolveFieldIdentifier(
            const ESPressio::Localisation::LocalisationContext& context,
            ESPressio::System::TypeIdentifier type,
            ESPressio::Localisation::TextView fieldName
        ) const noexcept {
            static_cast<void>(context);
            static_cast<void>(type);
            if (_mode == TestLocalisationMode::NotFound) {
                return {
                    ESPressio::Localisation::FieldIdentifierResolutionStatus::NotFound,
                    std::nullopt
                };
            }
            if (_mode == TestLocalisationMode::Unavailable) {
                return {
                    ESPressio::Localisation::FieldIdentifierResolutionStatus::ProviderUnavailable,
                    std::nullopt
                };
            }
            if (IsTextEqual(
                fieldName,
                "Alpha",
                5U
            )) {
                return {
                    ESPressio::Localisation::FieldIdentifierResolutionStatus::Success,
                    ESPressio::System::FieldIdentifier{0U}
                };
            }
            if (IsTextEqual(
                fieldName,
                "Beta",
                4U
            )) {
                return {
                    ESPressio::Localisation::FieldIdentifierResolutionStatus::Success,
                    ESPressio::System::FieldIdentifier{1U}
                };
            }
            return {
                ESPressio::Localisation::FieldIdentifierResolutionStatus::NotFound,
                std::nullopt
            };
        }

        /// Resolves one textual name across all test languages.
        [[nodiscard]] ESPressio::Localisation::FieldIdentifierResolutionResult ResolveFieldIdentifierAcrossLanguages(
            ESPressio::System::TypeIdentifier type,
            ESPressio::Localisation::TextView fieldName
        ) const noexcept {
            if (_mode == TestLocalisationMode::Ambiguous) {
                return {
                    ESPressio::Localisation::FieldIdentifierResolutionStatus::Ambiguous,
                    std::nullopt
                };
            }
            const auto terminal = ESPressio::Localisation::LanguageIdentifierView::Validate("en-GB");
            return ResolveFieldIdentifier(
                {terminal.Value, terminal.Value},
                type,
                fieldName
            );
        }

    };

    /// Real in-binary Localisation source used by the integration tests.
    using RealLocalisationSource = ESPressio::Localisation::InBinaryPackSource<
        ESPressio::Platform::Portable::Memory::ByteOperationsProvider
    >;

    /// Real Resolver bound to the generated Localisation integration-test contract.
    using RealLocalisationResolver = ESPressio::Localisation::Resolver<
        RealLocalisationSource,
        ESPressio::Platform::Portable::Memory::ByteOperationsProvider,
        TestGenerated::Contract
    >;

    /// Schema proving canonical numeric Field ordering at both identifier boundaries.
    struct BoundaryFields final {

        // Schema payload.

        /// High-identifier Field deliberately declared first in the C++ object.
        std::uint8_t High = 2U;

        /// Low-identifier Field deliberately declared second in the C++ object.
        std::uint8_t Low = 1U;

        // Schema metadata.

        /// Stable semantic Type identity owned by the boundary ordering test.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x05U}
        };

        /// Canonical FieldSet deliberately declared in descending numeric identity order.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&BoundaryFields::High, 255U>,
            ESPressio::System::FieldBinding<&BoundaryFields::Low, 0U>
        >;

    };


    /// Sparse optional schema exposing largest FieldIdentifier under test.
    ///
    /// Deliberately unsorted declared bindings prove canonical ordering does
    /// not rely upon recursive traversal of all 256 candidate identifiers.
    struct SparseOptionalFields final {

        // Schema payload.

        /// Optional high-identifier data.
        std::optional<std::uint8_t> High{};

        /// Required middle-identifier data.
        std::uint8_t Middle = 3U;

        /// Required low-identifier data.
        std::uint8_t Low = 1U;

        // Schema metadata.

        /// Type-local test identity.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{
                0x00U, 0x00U, 0x01U, 0x00U,
                0x00U, 0x00U, 0x00U, 0x06U
            }
        };

        /// Actual FieldSet traversal follows non-canonical descending order.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&SparseOptionalFields::High, 255U>,
            ESPressio::System::FieldBinding<&SparseOptionalFields::Middle, 100U>,
            ESPressio::System::FieldBinding<&SparseOptionalFields::Low, 0U>
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


    /// Declares the direct canonical surrogate of the deliberately fallible strong Type.
    template<>
    struct CanonicalRepresentation<TestSupport::FallibleStrong> final {

        // Representation metadata.

        /// Exact direct serialisable surrogate used by the fallible test adapters.
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


    /// Converts the deliberately fallible strong Type to its canonical representation.
    template<>
    struct TypeConversionAdapter<TestSupport::FallibleStrong, std::uint32_t> final {

        // Adapter metadata.

        /// Test-owned operation result Type.
        using ResultType = TestSupport::FallibleStrongConversionResult;

        /// Indicates that this pairwise conversion is available.
        static constexpr bool IsAvailable = true;

        /// Indicates that the conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the fallible test conversion result.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for Succeeded.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Converts every value except the deliberate sentinel 99.
        ///
        /// @param source Strong semantic source value.
        /// @param target Caller-owned canonical numeric target.
        /// @return Rejected for sentinel 99, otherwise Succeeded.
        static constexpr ResultType Convert(
            const TestSupport::FallibleStrong& source,
            std::uint32_t& target
        ) noexcept {
            if (source.Value == 99U) { return ResultType::Rejected; }

            target = source.Value;
            return ResultType::Succeeded;
        }

    };

    /// Converts the canonical numeric representation back to the fallible test Type.
    template<>
    struct TypeConversionAdapter<std::uint32_t, TestSupport::FallibleStrong> final {

        // Adapter metadata.

        /// Test-owned operation result Type.
        using ResultType = TestSupport::FallibleStrongConversionResult;

        /// Indicates that this pairwise conversion is available.
        static constexpr bool IsAvailable = true;

        /// Indicates that the conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the fallible test conversion result.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for Succeeded.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Reconstructs the strong semantic value except for the deliberate sentinel 99.
        ///
        /// @param source Canonical numeric representation.
        /// @param target Caller-owned semantic target.
        /// @return Rejected for sentinel 99, otherwise Succeeded.
        static constexpr ResultType Convert(
            const std::uint32_t& source,
            TestSupport::FallibleStrong& target
        ) noexcept {
            if (source == 99U) { return ResultType::Rejected; }

            target.Value = source;
            return ResultType::Succeeded;
        }

    };

} // ESPressio::Bounded

/// Verifies that one value measures and serialises to one exact canonical JSON spelling.
///
/// @tparam TValue Serialisable source Type.
/// @tparam TExpectedSize Fixed expected text storage extent including the null terminator.
/// @param value Source value under test.
/// @param expected Expected canonical JSON bytes as a null-terminated test literal.
template<class TValue, std::size_t TExpectedSize>
void ExpectCanonicalJson(
    const TValue& value,
    const char (&expected)[TExpectedSize]
) {
    const auto measurement = ESPressio::Serialisation::Measure<
        ESPressio::Serialisation::Json
    >(value);
    assert(measurement.IsSuccessful());
    assert(measurement.RequiredBytes == TExpectedSize - 1U);

    std::array<std::uint8_t, 512U> output{};
    const auto result = ESPressio::Serialisation::Serialise<
        ESPressio::Serialisation::Json
    >(
        value,
        std::as_writable_bytes(
            std::span{
                output.data(),
                output.size()
            }
        )
    );
    assert(result.IsSuccessful());
    assert(result.RequiredBytes == measurement.RequiredBytes);
    assert(result.BytesWritten == measurement.RequiredBytes);

    for (std::size_t index = 0U; index < result.BytesWritten; ++index) {
        assert(output[index] == static_cast<std::uint8_t>(expected[index]));
    }
}

/// Deserialises one null-terminated JSON test literal through the default Known-Type/Numeric profile.
///
/// @tparam TValue Serialisable destination Type.
/// @tparam TInputSize Fixed input storage extent including the test literal terminator.
/// @param input JSON test literal.
/// @param destination Existing destination object.
/// @return Complete operation-specific deserialisation result.
template<class TValue, std::size_t TInputSize>
ESPressio::Serialisation::DeserialisationResult DecodeJsonLiteral(
    const char (&input)[TInputSize],
    TValue& destination
) noexcept {
    return ESPressio::Serialisation::Deserialise<
        ESPressio::Serialisation::Json
    >(
        std::as_bytes(
            std::span<const char>{
                input,
                TInputSize - 1U
            }
        ),
        destination
    );
}

/// Verifies exact deterministic CBOR measurement and caller-buffer output.
///
/// @tparam TValue Serialisable source Type.
/// @tparam TExpectedSize Exact expected CBOR byte count.
/// @param value Source value.
/// @param expected Exact canonical CBOR bytes.
template<class TValue, std::size_t TExpectedSize>
void ExpectCanonicalCbor(
    const TValue& value,
    const std::array<std::uint8_t, TExpectedSize>& expected
) noexcept {
    const auto measurement = ESPressio::Serialisation::Measure<
        ESPressio::Serialisation::Cbor
    >(value);
    assert(measurement.IsSuccessful());
    assert(measurement.RequiredBytes == expected.size());

    std::array<std::uint8_t, 512U> output{};
    const auto result = ESPressio::Serialisation::Serialise<
        ESPressio::Serialisation::Cbor
    >(
        value,
        std::as_writable_bytes(
            std::span{
                output.data(),
                output.size()
            }
        )
    );
    assert(result.IsSuccessful());
    assert(result.RequiredBytes == expected.size());
    assert(result.BytesWritten == expected.size());
    for (std::size_t index = 0U; index < expected.size(); ++index) {
        assert(output[index] == expected[index]);
    }
}

/// Deserialises one fixed CBOR byte sequence through the default Known-Type/Numeric profile.
///
/// @tparam TValue Serialisable destination Type.
/// @tparam TInputSize Fixed CBOR byte count.
/// @param input Exact caller-owned CBOR bytes.
/// @param destination Existing destination object.
/// @return Complete operation-specific deserialisation result.
template<class TValue, std::size_t TInputSize>
ESPressio::Serialisation::DeserialisationResult DecodeCborBytes(
    const std::array<std::uint8_t, TInputSize>& input,
    TValue& destination
) noexcept {
    return ESPressio::Serialisation::Deserialise<
        ESPressio::Serialisation::Cbor
    >(
        std::as_bytes(
            std::span{
                input.data(),
                input.size()
            }
        ),
        destination
    );
}

/// Runs the complete Serialisation foundation plus JSON/CBOR encoding and decoding host contract suite.
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

    // Canonical scalar encodings.

    ExpectCanonicalJson(
        true,
        "true"
    );
    ExpectCanonicalJson(
        false,
        "false"
    );
    ExpectCanonicalJson(
        std::int32_t{-1234567},
        "-1234567"
    );
    ExpectCanonicalJson(
        std::uint64_t{18446744073709551615ULL},
        "18446744073709551615"
    );
    ExpectCanonicalJson(
        1.25F,
        "1.25"
    );
    ExpectCanonicalJson(
        1.25,
        "1.25"
    );
    ExpectCanonicalJson(
        0.0,
        "0"
    );
    ExpectCanonicalJson(
        -0.0,
        "-0"
    );
    ExpectCanonicalJson(
        TestSupport::Mode::Automatic,
        "1"
    );

    // Optional, fixed-array, bounded-sequence, and strong-Type encodings.

    std::optional<std::uint32_t> absent{};
    ExpectCanonicalJson(
        absent,
        "null"
    );
    absent = 42U;
    ExpectCanonicalJson(
        absent,
        "42"
    );

    const std::array<std::uint16_t, 3U> fixedArray{1U, 2U, 3U};
    ExpectCanonicalJson(
        fixedArray,
        "[1,2,3]"
    );

    const std::uint16_t nativeArray[]{7U, 8U};
    ExpectCanonicalJson(
        nativeArray,
        "[7,8]"
    );

    ESPressio::Bounded::Vector<std::uint16_t, 4U> boundedVector{};
    assert(boundedVector.PushBack(4U) == ESPressio::Bounded::VectorPushBackResult::Succeeded);
    assert(boundedVector.PushBack(5U) == ESPressio::Bounded::VectorPushBackResult::Succeeded);
    ExpectCanonicalJson(
        boundedVector,
        "[4,5]"
    );

    TestSupport::StrongCounter strongCounter{77U};
    ExpectCanonicalJson(
        strongCounter,
        "77"
    );

    // Canonical Bytes/Base64 and System identifier surrogate encodings.

    ESPressio::Bounded::Bytes<8U> binary{};
    const std::uint8_t binarySource[]{0x01U, 0x02U, 0xFEU};
    assert(
        binary.Append(
            binarySource,
            3U
        ) == ESPressio::Bounded::BytesAppendResult::Succeeded
    );
    ExpectCanonicalJson(
        binary,
        "\"AQL+\""
    );

    ESPressio::Bounded::Bytes<2U> oneByte{};
    const std::uint8_t oneByteSource[]{0xFFU};
    assert(
        oneByte.Append(
            oneByteSource,
            1U
        ) == ESPressio::Bounded::BytesAppendResult::Succeeded
    );
    ExpectCanonicalJson(
        oneByte,
        "\"/w==\""
    );

    ESPressio::Bounded::Bytes<2U> twoBytes{};
    const std::uint8_t twoByteSource[]{0xFFU, 0xEEU};
    assert(
        twoBytes.Append(
            twoByteSource,
            2U
        ) == ESPressio::Bounded::BytesAppendResult::Succeeded
    );
    ExpectCanonicalJson(
        twoBytes,
        "\"/+4=\""
    );

    ExpectCanonicalJson(
        identifier,
        "[0,0,1,0,0,0,0,42]"
    );

    // Deterministic String escaping and schema Field ordering/omission.

    ESPressio::Bounded::String<16U> controls{};
    const char controlBytes[]{static_cast<char>(0x1FU)};
    assert(
        controls.Assign(
            controlBytes,
            1U
        ) == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    ExpectCanonicalJson(
        controls,
        "\"\\u001f\""
    );

    TestSupport::Payload payload{};
    const char payloadText[]{'A', '\"', '\\', '\n', static_cast<char>(0xE2U), static_cast<char>(0x82U), static_cast<char>(0xACU)};
    assert(
        payload.Name.Assign(
            payloadText,
            sizeof(payloadText)
        ) == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    payload.Child.Value = 11U;
    ExpectCanonicalJson(
        payload,
        "{\"4\":{\"9\":11},\"7\":\"A\\\"\\\\\\n€\"}"
    );

    payload.Count = 6U;
    ExpectCanonicalJson(
        payload,
        "{\"2\":6,\"4\":{\"9\":11},\"7\":\"A\\\"\\\\\\n€\"}"
    );

    const TestSupport::BoundaryFields boundary{};
    ExpectCanonicalJson(
        boundary,
        "{\"0\":1,\"255\":2}"
    );

    const TestSupport::AdaptedPayload adaptedPayload{TestSupport::StrongCounter{88U}};
    ExpectCanonicalJson(
        adaptedPayload,
        "{\"3\":88}"
    );

    // Validation failures must prevent any caller-output mutation.

    std::array<std::uint8_t, 64U> untouched{};
    untouched.fill(0xA5U);
    const auto infiniteMeasurement = Measure<Json>(
        std::numeric_limits<double>::infinity()
    );
    assert(infiniteMeasurement.Status == MeasurementStatus::NonFiniteNumber);
    assert(infiniteMeasurement.RequiredBytes == 0U);
    const auto infiniteSerialisation = Serialise<Json>(
        std::numeric_limits<double>::infinity(),
        std::as_writable_bytes(
            std::span{
                untouched.data(),
                untouched.size()
            }
        )
    );
    assert(infiniteSerialisation.Status == SerialisationStatus::NonFiniteNumber);
    assert(infiniteSerialisation.BytesWritten == 0U);
    for (const auto byte : untouched) {
        assert(byte == 0xA5U);
    }

    ESPressio::Bounded::String<8U> invalidUtf8{};
    const char invalidUtf8Bytes[]{static_cast<char>(0xC0U), static_cast<char>(0xAFU)};
    assert(
        invalidUtf8.Assign(
            invalidUtf8Bytes,
            sizeof(invalidUtf8Bytes)
        ) == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    const auto invalidUtf8Measurement = Measure<Json>(invalidUtf8);
    assert(invalidUtf8Measurement.Status == MeasurementStatus::InvalidUtf8);
    assert(invalidUtf8Measurement.Detail.ByteOffset == 0U);
    untouched.fill(0x5AU);
    const auto invalidUtf8Serialisation = Serialise<Json>(
        invalidUtf8,
        std::as_writable_bytes(
            std::span{
                untouched.data(),
                untouched.size()
            }
        )
    );
    assert(invalidUtf8Serialisation.Status == SerialisationStatus::InvalidUtf8);
    assert(invalidUtf8Serialisation.BytesWritten == 0U);
    for (const auto byte : untouched) {
        assert(byte == 0x5AU);
    }

    const TestSupport::FallibleStrong rejected{99U};
    static_assert(SerialisableType<TestSupport::FallibleStrong>);
    const auto rejectedMeasurement = Measure<Json>(rejected);
    assert(rejectedMeasurement.Status == MeasurementStatus::AdaptationFailed);
    untouched.fill(0x6BU);
    const auto rejectedSerialisation = Serialise<Json>(
        rejected,
        std::as_writable_bytes(
            std::span{
                untouched.data(),
                untouched.size()
            }
        )
    );
    assert(rejectedSerialisation.Status == SerialisationStatus::AdaptationFailed);
    assert(rejectedSerialisation.BytesWritten == 0U);
    for (const auto byte : untouched) {
        assert(byte == 0x6BU);
    }

    // Capacity and caller-argument failures report exact required size without writing.

    const auto payloadMeasurement = Measure<Json>(payload);
    assert(payloadMeasurement.IsSuccessful());
    std::array<std::uint8_t, 8U> tooSmall{};
    tooSmall.fill(0xCCU);
    const auto insufficient = Serialise<Json>(
        payload,
        std::as_writable_bytes(
            std::span{
                tooSmall.data(),
                tooSmall.size()
            }
        )
    );
    assert(insufficient.Status == SerialisationStatus::OutputBufferTooSmall);
    assert(insufficient.RequiredBytes == payloadMeasurement.RequiredBytes);
    assert(insufficient.BytesWritten == 0U);
    for (const auto byte : tooSmall) {
        assert(byte == 0xCCU);
    }


    // JSON Numeric/Known-Type transactional deserialisation.

    std::uint16_t decodedInteger = 9U;
    auto decodeResult = DecodeJsonLiteral(
        "12.0",
        decodedInteger
    );
    assert(decodeResult.IsSuccessful());
    assert(decodeResult.BytesConsumed == 4U);
    assert(decodedInteger == 12U);

    decodeResult = DecodeJsonLiteral(
        "1e2",
        decodedInteger
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedInteger == 100U);

    decodeResult = DecodeJsonLiteral(
        "100e-2",
        decodedInteger
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedInteger == 1U);

    decodedInteger = 91U;
    decodeResult = DecodeJsonLiteral(
        "1.5",
        decodedInteger
    );
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert(decodedInteger == 91U);

    std::uint8_t decodedUnsigned8 = 7U;
    decodeResult = DecodeJsonLiteral(
        "255",
        decodedUnsigned8
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedUnsigned8 == 255U);
    decodedUnsigned8 = 7U;
    decodeResult = DecodeJsonLiteral(
        "256",
        decodedUnsigned8
    );
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedUnsigned8 == 7U);
    decodeResult = DecodeJsonLiteral(
        "-1",
        decodedUnsigned8
    );
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedUnsigned8 == 7U);
    decodeResult = DecodeJsonLiteral(
        "-0",
        decodedUnsigned8
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedUnsigned8 == 0U);

    std::int8_t decodedSigned8 = 3;
    decodeResult = DecodeJsonLiteral(
        "-128",
        decodedSigned8
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedSigned8 == std::numeric_limits<std::int8_t>::min());
    decodedSigned8 = 3;
    decodeResult = DecodeJsonLiteral(
        "-129",
        decodedSigned8
    );
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedSigned8 == 3);

    bool decodedBool = false;
    decodeResult = DecodeJsonLiteral(
        "true",
        decodedBool
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedBool);
    decodedBool = true;
    decodeResult = DecodeJsonLiteral(
        "1",
        decodedBool
    );
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert(decodedBool);

    TestSupport::Mode decodedMode = TestSupport::Mode::Off;
    decodeResult = DecodeJsonLiteral(
        "7",
        decodedMode
    );
    assert(decodeResult.IsSuccessful());
    assert(static_cast<std::uint8_t>(decodedMode) == 7U);

    double decodedDouble = 42.0;
    decodeResult = DecodeJsonLiteral(
        "-0",
        decodedDouble
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedDouble == 0.0);
    assert(std::signbit(decodedDouble));
    decodeResult = DecodeJsonLiteral(
        "0e999999999999999999999",
        decodedDouble
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedDouble == 0.0);
    decodedDouble = 42.0;
    decodeResult = DecodeJsonLiteral(
        "1e-10000",
        decodedDouble
    );
    assert(decodeResult.Status == DeserialisationStatus::NumericUnderflow);
    assert(decodedDouble == 42.0);
    decodeResult = DecodeJsonLiteral(
        "1e10000",
        decodedDouble
    );
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedDouble == 42.0);

    ESPressio::Bounded::String<8U> decodedText{};
    assert(
        decodedText.Assign("old") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral(
        "\"A\\u20ac\"",
        decodedText
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedText.View() == std::string_view{"A€"});

    const char validRawUtf8[]{
        '"',
        static_cast<char>(0xE2U),
        static_cast<char>(0x82U),
        static_cast<char>(0xACU),
        '"',
        '\0'
    };
    decodeResult = DecodeJsonLiteral(
        validRawUtf8,
        decodedText
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedText.Size() == 3U);
    assert(static_cast<std::uint8_t>(decodedText[0U]) == 0xE2U);
    assert(static_cast<std::uint8_t>(decodedText[1U]) == 0x82U);
    assert(static_cast<std::uint8_t>(decodedText[2U]) == 0xACU);
    decodeResult = DecodeJsonLiteral(
        "\"\\ud83d\\ude00\"",
        decodedText
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedText.Size() == 4U);
    assert(static_cast<std::uint8_t>(decodedText[0U]) == 0xF0U);
    assert(static_cast<std::uint8_t>(decodedText[3U]) == 0x80U);

    assert(
        decodedText.Assign("keep") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral(
        "\"\\u0000\"",
        decodedText
    );
    assert(decodeResult.Status == DeserialisationStatus::InvalidUtf8);
    assert(decodedText.View() == std::string_view{"keep"});

    const char invalidRawUtf8[]{'"', static_cast<char>(0xC0U), static_cast<char>(0xAFU), '"', '\0'};
    decodeResult = DecodeJsonLiteral(
        invalidRawUtf8,
        decodedText
    );
    assert(decodeResult.Status == DeserialisationStatus::InvalidUtf8);
    assert(decodedText.View() == std::string_view{"keep"});

    ESPressio::Bounded::String<3U> tinyText{};
    assert(
        tinyText.Assign("old") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral(
        "\"abcd\"",
        tinyText
    );
    assert(decodeResult.Status == DeserialisationStatus::CapacityExceeded);
    assert(tinyText.View() == std::string_view{"old"});

    ESPressio::Bounded::Bytes<4U> decodedBytes{};
    assert(
        decodedBytes.PushBack(9U) == ESPressio::Bounded::BytesPushBackResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral(
        "\"AQL+\"",
        decodedBytes
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedBytes.Size() == 3U);
    assert(decodedBytes[0U] == 1U);
    assert(decodedBytes[1U] == 2U);
    assert(decodedBytes[2U] == 254U);

    ESPressio::Bounded::Bytes<2U> preservedBytes{};
    assert(
        preservedBytes.PushBack(0xAAU) == ESPressio::Bounded::BytesPushBackResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral(
        "\"Zg\"",
        preservedBytes
    );
    assert(decodeResult.Status == DeserialisationStatus::InvalidBase64);
    assert(preservedBytes.Size() == 1U && preservedBytes[0U] == 0xAAU);
    decodeResult = DecodeJsonLiteral(
        "\"Zh==\"",
        preservedBytes
    );
    assert(decodeResult.Status == DeserialisationStatus::InvalidBase64);
    assert(preservedBytes.Size() == 1U && preservedBytes[0U] == 0xAAU);
    decodeResult = DecodeJsonLiteral(
        "\"Zm8=\"",
        preservedBytes
    );
    assert(decodeResult.IsSuccessful());
    assert(preservedBytes.Size() == 2U);
    assert(preservedBytes[0U] == static_cast<std::uint8_t>('f'));
    assert(preservedBytes[1U] == static_cast<std::uint8_t>('o'));

    std::array<std::uint16_t, 3U> decodedArray{9U, 9U, 9U};
    decodeResult = DecodeJsonLiteral(
        "[1,2,3]",
        decodedArray
    );
    assert(decodeResult.IsSuccessful());
    assert((decodedArray == std::array<std::uint16_t, 3U>{1U, 2U, 3U}));
    decodedArray = {9U, 9U, 9U};
    decodeResult = DecodeJsonLiteral(
        "[1,2]",
        decodedArray
    );
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert((decodedArray == std::array<std::uint16_t, 3U>{9U, 9U, 9U}));
    decodeResult = DecodeJsonLiteral(
        "[1,2,3,4]",
        decodedArray
    );
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert((decodedArray == std::array<std::uint16_t, 3U>{9U, 9U, 9U}));

    std::uint16_t decodedCArray[2U]{8U, 9U};
    decodeResult = DecodeJsonLiteral(
        "[4,5]",
        decodedCArray
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedCArray[0U] == 4U && decodedCArray[1U] == 5U);

    ESPressio::Bounded::Vector<std::uint16_t, 2U> decodedVector{};
    assert(
        decodedVector.PushBack(99U) == ESPressio::Bounded::VectorPushBackResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral(
        "[6,7]",
        decodedVector
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedVector.Size() == 2U && decodedVector[0U] == 6U && decodedVector[1U] == 7U);
    decodeResult = DecodeJsonLiteral(
        "[1,2,3]",
        decodedVector
    );
    assert(decodeResult.Status == DeserialisationStatus::CapacityExceeded);
    assert(decodedVector.Size() == 2U && decodedVector[0U] == 6U && decodedVector[1U] == 7U);

    std::optional<std::uint32_t> decodedOptional{55U};
    decodeResult = DecodeJsonLiteral(
        "null",
        decodedOptional
    );
    assert(decodeResult.IsSuccessful());
    assert(!decodedOptional.has_value());
    decodeResult = DecodeJsonLiteral(
        "73",
        decodedOptional
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedOptional.has_value() && decodedOptional.value() == 73U);

    TestSupport::Payload decodedPayload{};
    assert(
        decodedPayload.Name.Assign("before") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodedPayload.Count = 88U;
    decodedPayload.Child.Value = 99U;
    decodeResult = DecodeJsonLiteral(
        "{\"7\":\"A\\u20ac\",\"4\":{\"9\":11}}",
        decodedPayload
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedPayload.Name.View() == std::string_view{"A€"});
    assert(!decodedPayload.Count.has_value());
    assert(decodedPayload.Child.Value == 11U);

    decodeResult = DecodeJsonLiteral(
        "{\"4\":{\"9\":12},\"2\":6,\"7\":\"ordered-anyway\"}",
        decodedPayload
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedPayload.Count.has_value() && decodedPayload.Count.value() == 6U);
    assert(decodedPayload.Child.Value == 12U);
    assert(decodedPayload.Name.View() == std::string_view{"ordered-anyway"});

    assert(
        decodedPayload.Name.Assign("stable") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodedPayload.Count = 77U;
    decodedPayload.Child.Value = 66U;
    decodeResult = DecodeJsonLiteral(
        "{\"2\":1,\"4\":{\"9\":70000},\"7\":\"changed\"}",
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedPayload.Name.View() == std::string_view{"stable"});
    assert(decodedPayload.Count.has_value() && decodedPayload.Count.value() == 77U);
    assert(decodedPayload.Child.Value == 66U);

    decodeResult = DecodeJsonLiteral(
        "{\"7\":\"x\"}",
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::MissingRequiredField);
    assert(decodedPayload.Name.View() == std::string_view{"stable"});
    assert(decodedPayload.Child.Value == 66U);

    decodeResult = DecodeJsonLiteral(
        "{\"4\":{\"9\":1},\"4\":{\"9\":2},\"7\":\"x\"}",
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::DuplicateField);
    assert(decodedPayload.Child.Value == 66U);

    decodeResult = DecodeJsonLiteral(
        "{\"10\":1,\"4\":{\"9\":1},\"7\":\"x\"}",
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::UnknownField);
    assert(decodedPayload.Child.Value == 66U);

    const char ignoredUnknown[] = "{\"10\":{\"free\":[1,true,null]},\"4\":{\"9\":21},\"7\":\"ok\"}";
    decodeResult = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::Numeric,
        StrictnessPolicy::IgnoreUnknownFields
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(ignoredUnknown),
                sizeof(ignoredUnknown) - 1U
            }
        ),
        decodedPayload
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedPayload.Child.Value == 21U);
    assert(decodedPayload.Name.View() == std::string_view{"ok"});

    const char duplicateNestedUnknown[] =
        "{\"10\":{\"a\":1,\"\\u0061\":2},\"4\":{\"9\":1},\"7\":\"x\"}";
    decodeResult = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::Numeric,
        StrictnessPolicy::IgnoreUnknownFields
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(duplicateNestedUnknown),
                sizeof(duplicateNestedUnknown) - 1U
            }
        ),
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::DuplicateField);

    const char duplicateUnknown[] = "{\"10\":1,\"10\":2,\"4\":{\"9\":1},\"7\":\"x\"}";
    decodeResult = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::Numeric,
        StrictnessPolicy::IgnoreUnknownFields
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(duplicateUnknown),
                sizeof(duplicateUnknown) - 1U
            }
        ),
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::DuplicateField);

    const char limitedUnknown[] = "{\"10\":[1,2],\"4\":{\"9\":1},\"7\":\"x\"}";
    decodeResult = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::Numeric,
        StrictnessPolicy::IgnoreUnknownFields,
        ParserLimits<32U, 1U>
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(limitedUnknown),
                sizeof(limitedUnknown) - 1U
            }
        ),
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::ResourceLimitExceeded);

    const char limitedDepth[] = "{\"4\":{\"9\":1},\"7\":\"x\"}";
    decodeResult = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::Numeric,
        StrictnessPolicy::Exact,
        ParserLimits<1U, 8U>
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(limitedDepth),
                sizeof(limitedDepth) - 1U
            }
        ),
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::ResourceLimitExceeded);

    const char trailingJson[] = "{\"4\":{\"9\":1},\"7\":\"x\"} trailing";
    decodeResult = Deserialise<Json>(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(trailingJson),
                sizeof(trailingJson) - 1U
            }
        ),
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::TrailingData);

    TestSupport::StrongCounter decodedStrong{1U};
    decodeResult = DecodeJsonLiteral(
        "44",
        decodedStrong
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedStrong.Value == 44U);

    TestSupport::FallibleStrong decodedFallible{8U};
    decodeResult = DecodeJsonLiteral(
        "99",
        decodedFallible
    );
    assert(decodeResult.Status == DeserialisationStatus::AdaptationFailed);
    assert(decodedFallible.Value == 8U);

    ESPressio::System::FieldIdentifier decodedFieldIdentifier{1U};
    decodeResult = DecodeJsonLiteral(
        "42",
        decodedFieldIdentifier
    );
    assert(decodeResult.IsSuccessful());
    assert(decodedFieldIdentifier.Value() == 42U);

    const char malformedJson[] = "{\"4\":{\"9\":1},\"7\":\"x\",}";
    decodeResult = Deserialise<Json>(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(malformedJson),
                sizeof(malformedJson) - 1U
            }
        ),
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::MalformedRepresentation);

    // Sparse 0/100/255 Field dispatch must select actual bindings without
    // recursively consuming one stack frame for every absent numeric ID.
    TestSupport::SparseOptionalFields decodedSparseJson{};
    auto sparseJsonResult = DecodeJsonLiteral(
        "{\"255\":24,\"100\":3,\"0\":1}",
        decodedSparseJson
    );
    assert(sparseJsonResult.IsSuccessful());
    assert(decodedSparseJson.Low == 1U);
    assert(decodedSparseJson.Middle == 3U);
    assert(decodedSparseJson.High.has_value());
    assert(*decodedSparseJson.High == 24U);

    sparseJsonResult = DecodeJsonLiteral(
        "{\"100\":3,\"0\":1}",
        decodedSparseJson
    );
    assert(sparseJsonResult.IsSuccessful());
    assert(!decodedSparseJson.High.has_value());

    sparseJsonResult = DecodeJsonLiteral(
        "{\"255\":null,\"0\":1,\"100\":3}",
        decodedSparseJson
    );
    assert(sparseJsonResult.IsSuccessful());
    assert(!decodedSparseJson.High.has_value());

    // Typed Envelope canonical output and transactional metadata validation.

    TestSupport::Payload envelopeSource{};
    assert(
        envelopeSource.Name.Assign(
            "typed",
            5U
        ) == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    envelopeSource.Child.Value = 21U;
    const auto envelopeMeasurement = Measure<
        Json,
        RootProfile::TypedEnvelope
    >(envelopeSource);
    assert(envelopeMeasurement.IsSuccessful());
    std::array<std::uint8_t, 512U> envelopeOutput{};
    const auto envelopeSerialisation = Serialise<
        Json,
        RootProfile::TypedEnvelope
    >(
        envelopeSource,
        std::as_writable_bytes(
            std::span{
                envelopeOutput.data(),
                envelopeOutput.size()
            }
        )
    );
    assert(envelopeSerialisation.IsSuccessful());
    constexpr char ExpectedEnvelope[] =
        "{\"$edp\":{\"v\":1,\"type\":\"0000010000000002\"},\"value\":{\"4\":{\"9\":21},\"7\":\"typed\"}}";
    assert(envelopeSerialisation.BytesWritten == sizeof(ExpectedEnvelope) - 1U);
    for (std::size_t index = 0U; index < sizeof(ExpectedEnvelope) - 1U; ++index) {
        assert(envelopeOutput[index] == static_cast<std::uint8_t>(ExpectedEnvelope[index]));
    }

    TestSupport::Payload envelopeDestination{};
    envelopeDestination.Child.Value = 99U;
    auto envelopeDeserialisation = Deserialise<
        Json,
        RootProfile::TypedEnvelope
    >(
        std::as_bytes(
            std::span{
                envelopeOutput.data(),
                envelopeSerialisation.BytesWritten
            }
        ),
        envelopeDestination
    );
    assert(envelopeDeserialisation.IsSuccessful());
    assert(envelopeDestination.Child.Value == 21U);
    assert(envelopeDestination.Name.View() == "typed");

    constexpr char ReorderedEnvelope[] =
        "{\"value\":{\"7\":\"ordered\",\"4\":{\"9\":31}},\"$edp\":{\"type\":\"0000010000000002\",\"v\":1}}";
    envelopeDestination.Child.Value = 77U;
    envelopeDeserialisation = Deserialise<
        Json,
        RootProfile::TypedEnvelope
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(ReorderedEnvelope),
                sizeof(ReorderedEnvelope) - 1U
            }
        ),
        envelopeDestination
    );
    assert(envelopeDeserialisation.IsSuccessful());
    assert(envelopeDestination.Child.Value == 31U);
    assert(envelopeDestination.Name.View() == "ordered");

    constexpr char UnsupportedEnvelope[] =
        "{\"$edp\":{\"v\":2,\"type\":\"0000010000000002\"},\"value\":{\"4\":{\"9\":1},\"7\":\"bad\"}}";
    envelopeDestination.Child.Value = 77U;
    assert(envelopeDestination.Name.Assign(
        "before",
        6U
    ) == ESPressio::Bounded::StringAssignmentResult::Succeeded);
    envelopeDeserialisation = Deserialise<
        Json,
        RootProfile::TypedEnvelope
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(UnsupportedEnvelope),
                sizeof(UnsupportedEnvelope) - 1U
            }
        ),
        envelopeDestination
    );
    assert(envelopeDeserialisation.Status == DeserialisationStatus::UnsupportedEnvelopeVersion);
    assert(envelopeDestination.Child.Value == 77U);
    assert(envelopeDestination.Name.View() == "before");

    constexpr char MismatchedEnvelope[] =
        "{\"$edp\":{\"v\":1,\"type\":\"0000010000000003\"},\"value\":{\"4\":{\"9\":1},\"7\":\"bad\"}}";
    envelopeDeserialisation = Deserialise<
        Json,
        RootProfile::TypedEnvelope
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(MismatchedEnvelope),
                sizeof(MismatchedEnvelope) - 1U
            }
        ),
        envelopeDestination
    );
    assert(envelopeDeserialisation.Status == DeserialisationStatus::TypeIdentifierMismatch);
    assert(envelopeDestination.Child.Value == 77U);
    assert(envelopeDestination.Name.View() == "before");

    constexpr char DuplicateEnvelopeMetadata[] =
        "{\"$edp\":{\"v\":1,\"v\":1,\"type\":\"0000010000000002\"},\"value\":{\"4\":{\"9\":1},\"7\":\"bad\"}}";
    envelopeDeserialisation = Deserialise<
        Json,
        RootProfile::TypedEnvelope
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(DuplicateEnvelopeMetadata),
                sizeof(DuplicateEnvelopeMetadata) - 1U
            }
        ),
        envelopeDestination
    );
    assert(envelopeDeserialisation.Status == DeserialisationStatus::DuplicateField);
    assert(envelopeDestination.Child.Value == 77U);

    constexpr char MissingEnvelopeValue[] =
        "{\"$edp\":{\"v\":1,\"type\":\"0000010000000002\"}}";
    envelopeDeserialisation = Deserialise<
        Json,
        RootProfile::TypedEnvelope
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(MissingEnvelopeValue),
                sizeof(MissingEnvelopeValue) - 1U
            }
        ),
        envelopeDestination
    );
    assert(envelopeDeserialisation.Status == DeserialisationStatus::MalformedRepresentation);
    assert(envelopeDestination.Child.Value == 77U);

    // JSON LocalisedText integration against the real EDP-Localisation pack reader.

    ESPressio::Platform::Portable::Memory::ByteOperationsProvider localisationBytes;
    TestSupport::RealLocalisationSource localisationSource(
        TestGenerated::Descriptors,
        sizeof(TestGenerated::Descriptors) / sizeof(TestGenerated::Descriptors[0]),
        localisationBytes
    );
    TestSupport::RealLocalisationResolver localisationResolver(
        localisationSource,
        localisationBytes
    );
    const ESPressio::Localisation::LocalisationContext germanContext{
        TestGenerated::GermanValidation.Value,
        TestGenerated::EnglishValidation.Value
    };
    const ESPressio::Localisation::LocalisationContext englishContext{
        TestGenerated::EnglishValidation.Value,
        TestGenerated::EnglishValidation.Value
    };
    std::array<char, 64U> localisationScratchA{};
    std::array<char, 64U> localisationScratchB{};
    const ESPressio::Localisation::WritableTextView scratchA{
        localisationScratchA.data(), localisationScratchA.size()
    };
    const ESPressio::Localisation::WritableTextView scratchB{
        localisationScratchB.data(), localisationScratchB.size()
    };

    TestSupport::LocalisedReading localisedSource{};
    localisedSource.Temperature = 27U;
    const auto localisedMeasurement = Measure<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        localisedSource,
        localisationResolver,
        germanContext,
        scratchA,
        scratchB
    );
    assert(localisedMeasurement.IsSuccessful());
    std::array<std::uint8_t, 256U> localisedOutput{};
    const auto localisedSerialisation = Serialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        localisedSource,
        std::as_writable_bytes(
            std::span{
                localisedOutput.data(),
                localisedOutput.size()
            }
        ),
        localisationResolver,
        germanContext,
        scratchA,
        scratchB
    );
    assert(localisedSerialisation.IsSuccessful());
    constexpr char ExpectedLocalised[] =
        "{\"RFC5646\":\"de\",\"Temperature\":27}";
    assert(localisedSerialisation.BytesWritten == sizeof(ExpectedLocalised) - 1U);
    for (std::size_t index = 0U; index < sizeof(ExpectedLocalised) - 1U; ++index) {
        assert(localisedOutput[index] == static_cast<std::uint8_t>(ExpectedLocalised[index]));
    }

    TestSupport::LocalisedReading localisedDestination{};
    localisedDestination.Temperature = 99U;
    auto localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                localisedOutput.data(),
                localisedSerialisation.BytesWritten
            }
        ),
        localisedDestination,
        localisationResolver,
        germanContext,
        scratchA
    );
    assert(localisedDecode.IsSuccessful());
    assert(localisedDestination.Temperature == 27U);

    constexpr char NoMetadataLocalised[] = "{\"Temperature\":31}";
    localisedDestination.Temperature = 99U;
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(NoMetadataLocalised),
                sizeof(NoMetadataLocalised) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        TestGenerated::EnglishValidation.Value,
        scratchA
    );
    assert(localisedDecode.IsSuccessful());
    assert(localisedDestination.Temperature == 31U);

    constexpr char MismatchedLanguage[] =
        "{\"RFC5646\":\"de\",\"Temperature\":40}";
    localisedDestination.Temperature = 88U;
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(MismatchedLanguage),
                sizeof(MismatchedLanguage) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        englishContext,
        scratchA
    );
    assert(localisedDecode.Status == DeserialisationStatus::InvalidLanguageMetadata);
    assert(localisedDestination.Temperature == 88U);

    constexpr char NonStringLanguage[] =
        "{\"RFC5646\":123,\"Temperature\":40}";
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(NonStringLanguage),
                sizeof(NonStringLanguage) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        germanContext,
        scratchA
    );
    assert(localisedDecode.Status == DeserialisationStatus::InvalidLanguageMetadata);
    assert(localisedDestination.Temperature == 88U);

    constexpr char NonCanonicalLanguage[] =
        "{\"RFC5646\":\"EN-gb\",\"Temperature\":40}";
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(NonCanonicalLanguage),
                sizeof(NonCanonicalLanguage) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        TestGenerated::EnglishValidation.Value,
        scratchA
    );
    assert(localisedDecode.Status == DeserialisationStatus::InvalidLanguageMetadata);
    assert(localisedDestination.Temperature == 88U);

    constexpr char UnknownLocalised[] =
        "{\"RFC5646\":\"en-GB\",\"Unknown\":1,\"Temperature\":44}";
    localisedDestination.Temperature = 88U;
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(UnknownLocalised),
                sizeof(UnknownLocalised) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        englishContext,
        scratchA
    );
    assert(localisedDecode.Status == DeserialisationStatus::FieldNameNotFound);
    assert(localisedDestination.Temperature == 88U);
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText,
        StrictnessPolicy::IgnoreUnknownFields
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(UnknownLocalised),
                sizeof(UnknownLocalised) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        englishContext,
        scratchA
    );
    assert(localisedDecode.IsSuccessful());
    assert(localisedDestination.Temperature == 44U);

    std::array<char, 4U> tinyLocalisationScratch{};
    const ESPressio::Localisation::WritableTextView tinyScratch{
        tinyLocalisationScratch.data(), tinyLocalisationScratch.size()
    };
    const auto scratchLimitedMeasurement = Measure<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        localisedSource,
        localisationResolver,
        germanContext,
        tinyScratch,
        scratchB
    );
    assert(scratchLimitedMeasurement.Status == MeasurementStatus::ResourceLimitExceeded);

    const auto overlapMeasurement = Measure<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        localisedSource,
        localisationResolver,
        germanContext,
        scratchA,
        scratchA
    );
    assert(overlapMeasurement.Status == MeasurementStatus::InvalidArgument);

    localisedDestination.Temperature = 88U;
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                localisedOutput.data(),
                localisedSerialisation.BytesWritten
            }
        ),
        localisedDestination,
        localisationResolver,
        germanContext,
        tinyScratch
    );
    assert(localisedDecode.Status == DeserialisationStatus::ResourceLimitExceeded);
    assert(localisedDestination.Temperature == 88U);

    constexpr char DuplicateLanguageMetadata[] =
        "{\"RFC5646\":\"de\",\"RFC5646\":\"de\",\"Temperature\":1}";
    localisedDestination.Temperature = 88U;
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(DuplicateLanguageMetadata),
                sizeof(DuplicateLanguageMetadata) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        germanContext,
        scratchA
    );
    assert(localisedDecode.Status == DeserialisationStatus::DuplicateField);
    assert(localisedDestination.Temperature == 88U);

    constexpr char DuplicateLogicalField[] =
        "{\"RFC5646\":\"en-GB\",\"Temperature\":1,\"\\u0054emperature\":2}";
    localisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(DuplicateLogicalField),
                sizeof(DuplicateLogicalField) - 1U
            }
        ),
        localisedDestination,
        localisationResolver,
        englishContext,
        scratchA
    );
    assert(localisedDecode.Status == DeserialisationStatus::DuplicateField);
    assert(localisedDestination.Temperature == 88U);

    const auto localisedEnvelope = Serialise<
        Json,
        RootProfile::TypedEnvelope,
        FieldProfile::LocalisedText
    >(
        localisedSource,
        std::as_writable_bytes(
            std::span{
                localisedOutput.data(),
                localisedOutput.size()
            }
        ),
        localisationResolver,
        germanContext,
        scratchA,
        scratchB
    );
    assert(localisedEnvelope.IsSuccessful());
    localisedDestination.Temperature = 0U;
    localisedDecode = Deserialise<
        Json,
        RootProfile::TypedEnvelope,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                localisedOutput.data(),
                localisedEnvelope.BytesWritten
            }
        ),
        localisedDestination,
        localisationResolver,
        germanContext,
        scratchA
    );
    assert(localisedDecode.IsSuccessful());
    assert(localisedDestination.Temperature == 27U);

    // Recursive LocalisedText schema propagation.

    TestSupport::TestLocalisationResolver normalResolver{
        TestSupport::TestLocalisationMode::Normal
    };
    TestSupport::LocalisedNestedRoot nestedLocalised{};
    nestedLocalised.Child.Value = 71U;
    std::array<std::uint8_t, 256U> nestedLocalisedOutput{};
    const auto nestedLocalisedSerialisation = Serialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        nestedLocalised,
        std::as_writable_bytes(
            std::span{
                nestedLocalisedOutput.data(),
                nestedLocalisedOutput.size()
            }
        ),
        normalResolver,
        englishContext,
        scratchA,
        scratchB
    );
    assert(nestedLocalisedSerialisation.IsSuccessful());
    constexpr char ExpectedNestedLocalised[] =
        "{\"RFC5646\":\"en-GB\",\"Alpha\":{\"RFC5646\":\"en-GB\",\"Alpha\":71}}";
    assert(nestedLocalisedSerialisation.BytesWritten == sizeof(ExpectedNestedLocalised) - 1U);
    TestSupport::LocalisedNestedRoot nestedLocalisedDestination{};
    const auto nestedLocalisedDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                nestedLocalisedOutput.data(),
                nestedLocalisedSerialisation.BytesWritten
            }
        ),
        nestedLocalisedDestination,
        normalResolver,
        englishContext,
        scratchA
    );
    assert(nestedLocalisedDecode.IsSuccessful());
    assert(nestedLocalisedDestination.Child.Value == 71U);

    // Focused resolver-double coverage for valid runtime failure outcomes.

    TestSupport::LocalisedPair pair{};
    TestSupport::TestLocalisationResolver collisionResolver{
        TestSupport::TestLocalisationMode::Collision
    };
    auto localisedFailure = Measure<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        pair,
        collisionResolver,
        englishContext,
        scratchA,
        scratchB
    );
    assert(localisedFailure.Status == MeasurementStatus::FieldNameCollision);

    TestSupport::TestLocalisationResolver reservedResolver{
        TestSupport::TestLocalisationMode::Reserved
    };
    localisedFailure = Measure<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        pair,
        reservedResolver,
        englishContext,
        scratchA,
        scratchB
    );
    assert(localisedFailure.Status == MeasurementStatus::FieldNameCollision);

    TestSupport::TestLocalisationResolver unavailableResolver{
        TestSupport::TestLocalisationMode::Unavailable
    };
    localisedFailure = Measure<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        pair,
        unavailableResolver,
        englishContext,
        scratchA,
        scratchB
    );
    assert(localisedFailure.Status == MeasurementStatus::LocalisationFailure);

    TestSupport::TestLocalisationResolver ambiguousResolver{
        TestSupport::TestLocalisationMode::Ambiguous
    };
    TestSupport::LocalisedEmpty emptyLocalised{};
    constexpr char AmbiguousLocalised[] = "{\"Something\":1}";
    auto ambiguousDecode = Deserialise<
        Json,
        RootProfile::KnownTypeBody,
        FieldProfile::LocalisedText
    >(
        std::as_bytes(
            std::span{
                reinterpret_cast<const std::uint8_t*>(AmbiguousLocalised),
                sizeof(AmbiguousLocalised) - 1U
            }
        ),
        emptyLocalised,
        ambiguousResolver,
        TestGenerated::EnglishValidation.Value,
        scratchA
    );
    assert(ambiguousDecode.Status == DeserialisationStatus::FieldNameAmbiguous);


    // Deterministic CBOR Numeric profile contracts.

    ExpectCanonicalCbor(
        static_cast<std::uint8_t>(23U),
        std::array<std::uint8_t, 1U>{0x17U}
    );
    ExpectCanonicalCbor(
        static_cast<std::uint8_t>(24U),
        std::array<std::uint8_t, 2U>{0x18U, 0x18U}
    );
    ExpectCanonicalCbor(
        static_cast<std::int16_t>(-25),
        std::array<std::uint8_t, 2U>{0x38U, 0x18U}
    );
    ExpectCanonicalCbor(
        1.5F,
        std::array<std::uint8_t, 5U>{0xFAU, 0x3FU, 0xC0U, 0x00U, 0x00U}
    );
    ExpectCanonicalCbor(
        1.5,
        std::array<std::uint8_t, 9U>{
            0xFBU, 0x3FU, 0xF8U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
        }
    );
    ExpectCanonicalCbor(
        -0.0F,
        std::array<std::uint8_t, 5U>{0xFAU, 0x80U, 0x00U, 0x00U, 0x00U}
    );

    ESPressio::Bounded::String<8U> cborText{};
    assert(
        cborText.Assign("A") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    ExpectCanonicalCbor(
        cborText,
        std::array<std::uint8_t, 2U>{0x61U, 0x41U}
    );

    ESPressio::Bounded::Bytes<4U> cborBytes{};
    const std::uint8_t cborByteSource[]{0x01U, 0xFEU};
    assert(
        cborBytes.Assign(
            cborByteSource,
            sizeof(cborByteSource)
        ) == ESPressio::Bounded::BytesAssignmentResult::Succeeded
    );
    ExpectCanonicalCbor(
        cborBytes,
        std::array<std::uint8_t, 3U>{0x42U, 0x01U, 0xFEU}
    );

    const std::array<std::uint16_t, 3U> cborFixedArray{1U, 24U, 256U};
    ExpectCanonicalCbor(
        cborFixedArray,
        std::array<std::uint8_t, 7U>{
            0x83U, 0x01U, 0x18U, 0x18U, 0x19U, 0x01U, 0x00U
        }
    );

    TestSupport::Payload cborPayload{};
    assert(
        cborPayload.Name.Assign("A") ==
        ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    cborPayload.Child.Value = 11U;
    ExpectCanonicalCbor(
        cborPayload,
        std::array<std::uint8_t, 8U>{
            0xA2U,
            0x04U, 0xA1U, 0x09U, 0x0BU,
            0x07U, 0x61U, 0x41U
        }
    );
    cborPayload.Count = 24U;
    ExpectCanonicalCbor(
        cborPayload,
        std::array<std::uint8_t, 11U>{
            0xA3U,
            0x02U, 0x18U, 0x18U,
            0x04U, 0xA1U, 0x09U, 0x0BU,
            0x07U, 0x61U, 0x41U
        }
    );

    ExpectCanonicalCbor(
        TestSupport::BoundaryFields{},
        std::array<std::uint8_t, 6U>{
            0xA2U, 0x00U, 0x01U, 0x18U, 0xFFU, 0x02U
        }
    );
    ExpectCanonicalCbor(
        TestSupport::StrongCounter{77U},
        std::array<std::uint8_t, 2U>{0x18U, 0x4DU}
    );

    // Sparse schema boundary regression: canonical ascending numeric keys
    // 0, 100, 255, despite reverse declared FieldSet ordering. Optional
    // omission must not force 256 nested runtime stack frames.
    TestSupport::SparseOptionalFields sparseSchema{};
    ExpectCanonicalCbor(
        sparseSchema,
        std::array<std::uint8_t, 6U>{
            0xA2U, 0x00U, 0x01U, 0x18U, 0x64U, 0x03U
        }
    );

    sparseSchema.High = 24U;
    ExpectCanonicalCbor(
        sparseSchema,
        std::array<std::uint8_t, 10U>{
            0xA3U, 0x00U, 0x01U, 0x18U, 0x64U, 0x03U,
            0x18U, 0xFFU, 0x18U, 0x18U
        }
    );

    // CBOR preflight failures preserve caller output exactly.

    std::array<std::uint8_t, 32U> cborUntouched{};
    cborUntouched.fill(0xA5U);
    const auto cborInfiniteMeasurement = Measure<Cbor>(
        std::numeric_limits<double>::infinity()
    );
    assert(cborInfiniteMeasurement.Status == MeasurementStatus::NonFiniteNumber);
    const auto cborInfiniteSerialisation = Serialise<Cbor>(
        std::numeric_limits<double>::infinity(),
        std::as_writable_bytes(
            std::span{
                cborUntouched.data(),
                cborUntouched.size()
            }
        )
    );
    assert(cborInfiniteSerialisation.Status == SerialisationStatus::NonFiniteNumber);
    assert(cborInfiniteSerialisation.BytesWritten == 0U);
    for (const auto byte : cborUntouched) { assert(byte == 0xA5U); }

    cborUntouched.fill(0x5AU);
    const auto cborRejectedSerialisation = Serialise<Cbor>(
        TestSupport::FallibleStrong{99U},
        std::as_writable_bytes(
            std::span{
                cborUntouched.data(),
                cborUntouched.size()
            }
        )
    );
    assert(cborRejectedSerialisation.Status == SerialisationStatus::AdaptationFailed);
    assert(cborRejectedSerialisation.BytesWritten == 0U);
    for (const auto byte : cborUntouched) { assert(byte == 0x5AU); }

    const auto cborPayloadMeasurement = Measure<Cbor>(cborPayload);
    assert(cborPayloadMeasurement.IsSuccessful());
    std::array<std::uint8_t, 2U> cborTooSmall{0xCCU, 0xCCU};
    const auto cborInsufficient = Serialise<Cbor>(
        cborPayload,
        std::as_writable_bytes(
            std::span{
                cborTooSmall.data(),
                cborTooSmall.size()
            }
        )
    );
    assert(cborInsufficient.Status == SerialisationStatus::OutputBufferTooSmall);
    assert(cborInsufficient.RequiredBytes == cborPayloadMeasurement.RequiredBytes);
    assert(cborInsufficient.BytesWritten == 0U);
    assert(cborTooSmall[0U] == 0xCCU && cborTooSmall[1U] == 0xCCU);

    // Decoding a map with Field 255 must not recurse through 256 absent
    // numeric-id candidates; absent Optional clears stale destination data.
    TestSupport::SparseOptionalFields decodedSparseCbor{};
    decodedSparseCbor.High = 11U;
    const auto absentHigh = DecodeCborBytes(
        std::array<std::uint8_t, 6U>{
            0xA2U, 0x00U, 0x01U, 0x18U, 0x64U, 0x03U
        },
        decodedSparseCbor
    );
    assert(absentHigh.IsSuccessful());
    assert(decodedSparseCbor.Low == 1U);
    assert(decodedSparseCbor.Middle == 3U);
    assert(!decodedSparseCbor.High.has_value());

    const auto presentHigh = DecodeCborBytes(
        std::array<std::uint8_t, 10U>{
            0xA3U, 0x00U, 0x01U, 0x18U, 0x64U, 0x03U,
            0x18U, 0xFFU, 0x18U, 0x18U
        },
        decodedSparseCbor
    );
    assert(presentHigh.IsSuccessful());
    assert(decodedSparseCbor.High.has_value());
    assert(*decodedSparseCbor.High == 24U);

    // CBOR exact-category/canonical decoding and transactionality.

    std::uint16_t cborUnsigned = 77U;
    auto cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x18U, 0x01U},
        cborUnsigned
    );
    assert(cborDecode.Status == DeserialisationStatus::MalformedRepresentation);
    assert(cborUnsigned == 77U);
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x18U, 0x18U},
        cborUnsigned
    );
    assert(cborDecode.IsSuccessful() && cborUnsigned == 24U);

    std::uint8_t cborUnsigned8 = 7U;
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 3U>{0x19U, 0x01U, 0x00U},
        cborUnsigned8
    );
    assert(cborDecode.Status == DeserialisationStatus::NumericOutOfRange);
    assert(cborUnsigned8 == 7U);

    bool cborBool = false;
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 1U>{0x01U},
        cborBool
    );
    assert(cborDecode.Status == DeserialisationStatus::TypeMismatch);
    assert(!cborBool);

    TestSupport::Mode cborMode = TestSupport::Mode::Off;
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 1U>{0x07U},
        cborMode
    );
    assert(cborDecode.IsSuccessful());
    assert(static_cast<std::uint8_t>(cborMode) == 7U);

    float cborFloat = 9.0F;
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 9U>{
            0xFBU, 0x3FU, 0xF0U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
        },
        cborFloat
    );
    assert(cborDecode.Status == DeserialisationStatus::TypeMismatch);
    assert(cborFloat == 9.0F);
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 3U>{0xF9U, 0x00U, 0x00U},
        cborFloat
    );
    assert(cborDecode.Status == DeserialisationStatus::TypeMismatch);
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 5U>{0xFAU, 0x7FU, 0x80U, 0x00U, 0x00U},
        cborFloat
    );
    assert(cborDecode.Status == DeserialisationStatus::NonFiniteNumber);
    assert(cborFloat == 9.0F);
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 5U>{0xFAU, 0x80U, 0x00U, 0x00U, 0x00U},
        cborFloat
    );
    assert(cborDecode.IsSuccessful());
    assert(cborFloat == 0.0F && std::signbit(cborFloat));

    double cborDouble = 9.0;
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 5U>{0xFAU, 0x3FU, 0x80U, 0x00U, 0x00U},
        cborDouble
    );
    assert(cborDecode.Status == DeserialisationStatus::TypeMismatch);
    assert(cborDouble == 9.0);
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 9U>{
            0xFBU, 0x3FU, 0xF8U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
        },
        cborDouble
    );
    assert(cborDecode.IsSuccessful());
    assert(cborDouble == 1.5);

    ESPressio::Bounded::String<4U> cborDecodedText{};
    assert(
        cborDecodedText.Assign("old") ==
        ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x41U, 0x41U},
        cborDecodedText
    );
    assert(cborDecode.Status == DeserialisationStatus::TypeMismatch);
    assert(cborDecodedText.View() == "old");
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 3U>{0x62U, 0xC0U, 0xAFU},
        cborDecodedText
    );
    assert(cborDecode.Status == DeserialisationStatus::InvalidUtf8);
    assert(cborDecodedText.View() == "old");
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x61U, 0x41U},
        cborDecodedText
    );
    assert(cborDecode.IsSuccessful());
    assert(cborDecodedText.View() == "A");
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 4U>{0x63U, 0xE2U, 0x82U, 0xACU},
        cborDecodedText
    );
    assert(cborDecode.IsSuccessful());
    assert(cborDecodedText.Size() == 3U);
    assert(static_cast<std::uint8_t>(cborDecodedText[0U]) == 0xE2U);
    assert(static_cast<std::uint8_t>(cborDecodedText[1U]) == 0x82U);
    assert(static_cast<std::uint8_t>(cborDecodedText[2U]) == 0xACU);

    ESPressio::Bounded::Bytes<2U> cborDecodedBytes{};
    assert(
        cborDecodedBytes.PushBack(9U) ==
        ESPressio::Bounded::BytesPushBackResult::Succeeded
    );
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x61U, 0x41U},
        cborDecodedBytes
    );
    assert(cborDecode.Status == DeserialisationStatus::TypeMismatch);
    assert(cborDecodedBytes.Size() == 1U && cborDecodedBytes[0U] == 9U);
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 3U>{0x42U, 0x01U, 0xFEU},
        cborDecodedBytes
    );
    assert(cborDecode.IsSuccessful());
    assert(cborDecodedBytes.Size() == 2U);
    assert(cborDecodedBytes[0U] == 0x01U && cborDecodedBytes[1U] == 0xFEU);

    std::array<std::uint16_t, 3U> cborDecodedArray{9U, 9U, 9U};
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 4U>{0x83U, 0x01U, 0x02U, 0x03U},
        cborDecodedArray
    );
    assert(cborDecode.IsSuccessful());
    assert((cborDecodedArray == std::array<std::uint16_t, 3U>{1U, 2U, 3U}));
    cborDecodedArray = {9U, 9U, 9U};
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 3U>{0x82U, 0x01U, 0x02U},
        cborDecodedArray
    );
    assert(cborDecode.Status == DeserialisationStatus::TypeMismatch);
    assert((cborDecodedArray == std::array<std::uint16_t, 3U>{9U, 9U, 9U}));
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 3U>{0x9FU, 0x01U, 0xFFU},
        cborDecodedArray
    );
    assert(cborDecode.Status == DeserialisationStatus::MalformedRepresentation);

    ESPressio::Bounded::Vector<std::uint16_t, 2U> cborDecodedVector{};
    assert(
        cborDecodedVector.PushBack(99U) ==
        ESPressio::Bounded::VectorPushBackResult::Succeeded
    );
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 3U>{0x82U, 0x06U, 0x07U},
        cborDecodedVector
    );
    assert(cborDecode.IsSuccessful());
    assert(cborDecodedVector.Size() == 2U);
    assert(cborDecodedVector[0U] == 6U && cborDecodedVector[1U] == 7U);
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 4U>{0x83U, 0x01U, 0x02U, 0x03U},
        cborDecodedVector
    );
    assert(cborDecode.Status == DeserialisationStatus::CapacityExceeded);
    assert(cborDecodedVector.Size() == 2U);
    assert(cborDecodedVector[0U] == 6U && cborDecodedVector[1U] == 7U);

    std::optional<std::uint32_t> cborOptional{55U};
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 1U>{0xF6U},
        cborOptional
    );
    assert(cborDecode.IsSuccessful() && !cborOptional.has_value());
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x18U, 0x49U},
        cborOptional
    );
    assert(cborDecode.IsSuccessful());
    assert(cborOptional.has_value() && cborOptional.value() == 73U);

    // Arbitrary schema Field order is accepted; omission/null semantics remain transactional.

    TestSupport::Payload cborDecodedPayload{};
    assert(
        cborDecodedPayload.Name.Assign("before") ==
        ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    cborDecodedPayload.Count = 88U;
    cborDecodedPayload.Child.Value = 99U;
    const std::array<std::uint8_t, 8U> cborPayloadOutOfOrder{
        0xA2U,
        0x07U, 0x61U, 0x41U,
        0x04U, 0xA1U, 0x09U, 0x0BU
    };
    cborDecode = DecodeCborBytes(
        cborPayloadOutOfOrder,
        cborDecodedPayload
    );
    assert(cborDecode.IsSuccessful());
    assert(cborDecodedPayload.Name.View() == "A");
    assert(!cborDecodedPayload.Count.has_value());
    assert(cborDecodedPayload.Child.Value == 11U);

    assert(
        cborDecodedPayload.Name.Assign("stable") ==
        ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    cborDecodedPayload.Count = 77U;
    cborDecodedPayload.Child.Value = 66U;
    const std::array<std::uint8_t, 9U> cborPayloadRangeFailure{
        0xA3U,
        0x02U, 0x01U,
        0x04U, 0xA1U, 0x09U, 0x1AU, 0x00U
    };
    cborDecode = DecodeCborBytes(
        cborPayloadRangeFailure,
        cborDecodedPayload
    );
    assert(cborDecode.Status == DeserialisationStatus::MalformedRepresentation);
    assert(cborDecodedPayload.Name.View() == "stable");
    assert(cborDecodedPayload.Count.has_value() && cborDecodedPayload.Count.value() == 77U);
    assert(cborDecodedPayload.Child.Value == 66U);

    const std::array<std::uint8_t, 1U> cborMissingRequired{0xA0U};
    cborDecode = DecodeCborBytes(
        cborMissingRequired,
        cborDecodedPayload
    );
    assert(cborDecode.Status == DeserialisationStatus::MissingRequiredField);
    assert(cborDecodedPayload.Child.Value == 66U);

    const std::array<std::uint8_t, 7U> cborDuplicateField{
        0xA2U,
        0x04U, 0xA1U, 0x09U, 0x01U,
        0x04U, 0xA0U
    };
    cborDecode = DecodeCborBytes(
        cborDuplicateField,
        cborDecodedPayload
    );
    assert(cborDecode.Status == DeserialisationStatus::DuplicateField);
    assert(cborDecodedPayload.Child.Value == 66U);

    const std::array<std::uint8_t, 8U> cborUnknownExact{
        0xA2U,
        0x0AU, 0x82U, 0x01U, 0x02U,
        0x04U, 0xA1U, 0x09U
    };
    cborDecode = DecodeCborBytes(
        cborUnknownExact,
        cborDecodedPayload
    );
    assert(cborDecode.Status == DeserialisationStatus::UnknownField);
    assert(cborDecodedPayload.Child.Value == 66U);

    const std::array<std::uint8_t, 12U> cborUnknownIgnored{
        0xA3U,
        0x0AU, 0x82U, 0x01U, 0x02U,
        0x04U, 0xA1U, 0x09U, 0x15U,
        0x07U, 0x61U, 0x58U
    };
    cborDecode = Deserialise<
        Cbor,
        RootProfile::KnownTypeBody,
        FieldProfile::Numeric,
        StrictnessPolicy::IgnoreUnknownFields
    >(
        std::as_bytes(
            std::span{
                cborUnknownIgnored.data(),
                cborUnknownIgnored.size()
            }
        ),
        cborDecodedPayload
    );
    assert(cborDecode.IsSuccessful());
    assert(cborDecodedPayload.Child.Value == 21U);
    assert(cborDecodedPayload.Name.View() == "X");

    cborDecodedPayload.Child.Value = 66U;
    cborDecode = Deserialise<
        Cbor,
        RootProfile::KnownTypeBody,
        FieldProfile::Numeric,
        StrictnessPolicy::IgnoreUnknownFields,
        ParserLimits<32U, 1U>
    >(
        std::as_bytes(
            std::span{
                cborUnknownIgnored.data(),
                cborUnknownIgnored.size()
            }
        ),
        cborDecodedPayload
    );
    assert(cborDecode.Status == DeserialisationStatus::ResourceLimitExceeded);
    assert(cborDecodedPayload.Child.Value == 66U);

    const std::array<std::uint8_t, 2U> cborTrailing{0x01U, 0x02U};
    cborUnsigned8 = 9U;
    cborDecode = DecodeCborBytes(
        cborTrailing,
        cborUnsigned8
    );
    assert(cborDecode.Status == DeserialisationStatus::TrailingData);
    assert(cborUnsigned8 == 9U);

    TestSupport::StrongCounter cborDecodedStrong{1U};
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x18U, 0x2CU},
        cborDecodedStrong
    );
    assert(cborDecode.IsSuccessful() && cborDecodedStrong.Value == 44U);

    TestSupport::FallibleStrong cborDecodedFallible{8U};
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 2U>{0x18U, 0x63U},
        cborDecodedFallible
    );
    assert(cborDecode.Status == DeserialisationStatus::AdaptationFailed);
    assert(cborDecodedFallible.Value == 8U);

    std::uint64_t cborMaximumUnsigned = 0U;
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 9U>{
            0x1BU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU
        },
        cborMaximumUnsigned
    );
    assert(cborDecode.IsSuccessful());
    assert(cborMaximumUnsigned == std::numeric_limits<std::uint64_t>::max());

    std::int64_t cborMinimumSigned = 0;
    cborDecode = DecodeCborBytes(
        std::array<std::uint8_t, 9U>{
            0x3BU, 0x7FU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU
        },
        cborMinimumSigned
    );
    assert(cborDecode.IsSuccessful());
    assert(cborMinimumSigned == std::numeric_limits<std::int64_t>::min());

    // CBOR Typed Envelope carries only version, exact 8-byte root identity, and body.

    TestSupport::Payload cborEnvelopeSource{};
    assert(
        cborEnvelopeSource.Name.Assign("T") ==
        ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    cborEnvelopeSource.Child.Value = 7U;
    const auto cborEnvelopeMeasurement = Measure<
        Cbor,
        RootProfile::TypedEnvelope
    >(cborEnvelopeSource);
    assert(cborEnvelopeMeasurement.IsSuccessful());
    std::array<std::uint8_t, 64U> cborEnvelopeOutput{};
    const auto cborEnvelopeSerialisation = Serialise<
        Cbor,
        RootProfile::TypedEnvelope
    >(
        cborEnvelopeSource,
        std::as_writable_bytes(
            std::span{
                cborEnvelopeOutput.data(),
                cborEnvelopeOutput.size()
            }
        )
    );
    assert(cborEnvelopeSerialisation.IsSuccessful());
    assert(cborEnvelopeOutput[0U] == 0x83U);
    assert(cborEnvelopeOutput[1U] == 0x01U);
    assert(cborEnvelopeOutput[2U] == 0x48U);
    const auto& cborExpectedIdentifier = TestSupport::Payload::Identifier.Bytes();
    for (std::size_t index = 0U; index < cborExpectedIdentifier.size(); ++index) {
        assert(cborEnvelopeOutput[3U + index] == cborExpectedIdentifier[index]);
    }

    TestSupport::Payload cborEnvelopeDestination{};
    cborEnvelopeDestination.Child.Value = 99U;
    auto cborEnvelopeDecode = Deserialise<
        Cbor,
        RootProfile::TypedEnvelope
    >(
        std::as_bytes(
            std::span{
                cborEnvelopeOutput.data(),
                cborEnvelopeSerialisation.BytesWritten
            }
        ),
        cborEnvelopeDestination
    );
    assert(cborEnvelopeDecode.IsSuccessful());
    assert(cborEnvelopeDestination.Child.Value == 7U);
    assert(cborEnvelopeDestination.Name.View() == "T");

    auto cborBadEnvelope = cborEnvelopeOutput;
    cborBadEnvelope[1U] = 0x02U;
    cborEnvelopeDestination.Child.Value = 99U;
    cborEnvelopeDecode = Deserialise<Cbor, RootProfile::TypedEnvelope>(
        std::as_bytes(
            std::span{
                cborBadEnvelope.data(),
                cborEnvelopeSerialisation.BytesWritten
            }
        ),
        cborEnvelopeDestination
    );
    assert(cborEnvelopeDecode.Status == DeserialisationStatus::UnsupportedEnvelopeVersion);
    assert(cborEnvelopeDestination.Child.Value == 99U);

    cborBadEnvelope = cborEnvelopeOutput;
    cborBadEnvelope[3U] ^= 0x01U;
    cborEnvelopeDestination.Child.Value = 99U;
    cborEnvelopeDecode = Deserialise<Cbor, RootProfile::TypedEnvelope>(
        std::as_bytes(
            std::span{
                cborBadEnvelope.data(),
                cborEnvelopeSerialisation.BytesWritten
            }
        ),
        cborEnvelopeDestination
    );
    assert(cborEnvelopeDecode.Status == DeserialisationStatus::TypeIdentifierMismatch);
    assert(cborEnvelopeDestination.Child.Value == 99U);

    const std::array<std::uint8_t, 3U> cborIndefiniteArray{0x9FU, 0x01U, 0xFFU};
    cborDecodedArray = {9U, 9U, 9U};
    cborDecode = DecodeCborBytes(
        cborIndefiniteArray,
        cborDecodedArray
    );
    assert(cborDecode.Status == DeserialisationStatus::MalformedRepresentation);
    assert((cborDecodedArray == std::array<std::uint16_t, 3U>{9U, 9U, 9U}));

    return 0;
}
