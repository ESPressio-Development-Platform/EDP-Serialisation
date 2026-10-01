#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
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
        output.data(),
        output.size()
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
        reinterpret_cast<const std::uint8_t*>(input),
        TInputSize - 1U,
        destination
    );
}

/// Runs the complete Serialisation foundation, JSON encoding, and JSON decoding host contract suite.
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
        untouched.data(),
        untouched.size()
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
        untouched.data(),
        untouched.size()
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
        untouched.data(),
        untouched.size()
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
        tooSmall.data(),
        tooSmall.size()
    );
    assert(insufficient.Status == SerialisationStatus::OutputBufferTooSmall);
    assert(insufficient.RequiredBytes == payloadMeasurement.RequiredBytes);
    assert(insufficient.BytesWritten == 0U);
    for (const auto byte : tooSmall) {
        assert(byte == 0xCCU);
    }

    const auto nullOutput = Serialise<Json>(
        payload,
        nullptr,
        payloadMeasurement.RequiredBytes
    );
    assert(nullOutput.Status == SerialisationStatus::InvalidArgument);
    assert(nullOutput.RequiredBytes == payloadMeasurement.RequiredBytes);
    assert(nullOutput.BytesWritten == 0U);


    // JSON Numeric/Known-Type transactional deserialisation.

    std::uint16_t decodedInteger = 9U;
    auto decodeResult = DecodeJsonLiteral("12.0", decodedInteger);
    assert(decodeResult.IsSuccessful());
    assert(decodeResult.BytesConsumed == 4U);
    assert(decodedInteger == 12U);

    decodeResult = DecodeJsonLiteral("1e2", decodedInteger);
    assert(decodeResult.IsSuccessful());
    assert(decodedInteger == 100U);

    decodeResult = DecodeJsonLiteral("100e-2", decodedInteger);
    assert(decodeResult.IsSuccessful());
    assert(decodedInteger == 1U);

    decodedInteger = 91U;
    decodeResult = DecodeJsonLiteral("1.5", decodedInteger);
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert(decodedInteger == 91U);

    std::uint8_t decodedUnsigned8 = 7U;
    decodeResult = DecodeJsonLiteral("255", decodedUnsigned8);
    assert(decodeResult.IsSuccessful());
    assert(decodedUnsigned8 == 255U);
    decodedUnsigned8 = 7U;
    decodeResult = DecodeJsonLiteral("256", decodedUnsigned8);
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedUnsigned8 == 7U);
    decodeResult = DecodeJsonLiteral("-1", decodedUnsigned8);
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedUnsigned8 == 7U);
    decodeResult = DecodeJsonLiteral("-0", decodedUnsigned8);
    assert(decodeResult.IsSuccessful());
    assert(decodedUnsigned8 == 0U);

    std::int8_t decodedSigned8 = 3;
    decodeResult = DecodeJsonLiteral("-128", decodedSigned8);
    assert(decodeResult.IsSuccessful());
    assert(decodedSigned8 == std::numeric_limits<std::int8_t>::min());
    decodedSigned8 = 3;
    decodeResult = DecodeJsonLiteral("-129", decodedSigned8);
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedSigned8 == 3);

    bool decodedBool = false;
    decodeResult = DecodeJsonLiteral("true", decodedBool);
    assert(decodeResult.IsSuccessful());
    assert(decodedBool);
    decodedBool = true;
    decodeResult = DecodeJsonLiteral("1", decodedBool);
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert(decodedBool);

    TestSupport::Mode decodedMode = TestSupport::Mode::Off;
    decodeResult = DecodeJsonLiteral("7", decodedMode);
    assert(decodeResult.IsSuccessful());
    assert(static_cast<std::uint8_t>(decodedMode) == 7U);

    double decodedDouble = 42.0;
    decodeResult = DecodeJsonLiteral("-0", decodedDouble);
    assert(decodeResult.IsSuccessful());
    assert(decodedDouble == 0.0);
    assert(std::signbit(decodedDouble));
    decodeResult = DecodeJsonLiteral("0e999999999999999999999", decodedDouble);
    assert(decodeResult.IsSuccessful());
    assert(decodedDouble == 0.0);
    decodedDouble = 42.0;
    decodeResult = DecodeJsonLiteral("1e-10000", decodedDouble);
    assert(decodeResult.Status == DeserialisationStatus::NumericUnderflow);
    assert(decodedDouble == 42.0);
    decodeResult = DecodeJsonLiteral("1e10000", decodedDouble);
    assert(decodeResult.Status == DeserialisationStatus::NumericOutOfRange);
    assert(decodedDouble == 42.0);

    ESPressio::Bounded::String<8U> decodedText{};
    assert(
        decodedText.Assign("old") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral("\"A\\u20ac\"", decodedText);
    assert(decodeResult.IsSuccessful());
    assert(decodedText.View() == std::string_view{"A€"});
    decodeResult = DecodeJsonLiteral("\"\\ud83d\\ude00\"", decodedText);
    assert(decodeResult.IsSuccessful());
    assert(decodedText.Size() == 4U);
    assert(static_cast<std::uint8_t>(decodedText[0U]) == 0xF0U);
    assert(static_cast<std::uint8_t>(decodedText[3U]) == 0x80U);

    assert(
        decodedText.Assign("keep") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral("\"\\u0000\"", decodedText);
    assert(decodeResult.Status == DeserialisationStatus::InvalidUtf8);
    assert(decodedText.View() == std::string_view{"keep"});

    const char invalidRawUtf8[]{'"', static_cast<char>(0xC0U), static_cast<char>(0xAFU), '"', '\0'};
    decodeResult = DecodeJsonLiteral(invalidRawUtf8, decodedText);
    assert(decodeResult.Status == DeserialisationStatus::InvalidUtf8);
    assert(decodedText.View() == std::string_view{"keep"});

    ESPressio::Bounded::String<3U> tinyText{};
    assert(
        tinyText.Assign("old") == ESPressio::Bounded::StringAssignmentResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral("\"abcd\"", tinyText);
    assert(decodeResult.Status == DeserialisationStatus::CapacityExceeded);
    assert(tinyText.View() == std::string_view{"old"});

    ESPressio::Bounded::Bytes<4U> decodedBytes{};
    assert(
        decodedBytes.PushBack(9U) == ESPressio::Bounded::BytesPushBackResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral("\"AQL+\"", decodedBytes);
    assert(decodeResult.IsSuccessful());
    assert(decodedBytes.Size() == 3U);
    assert(decodedBytes[0U] == 1U);
    assert(decodedBytes[1U] == 2U);
    assert(decodedBytes[2U] == 254U);

    ESPressio::Bounded::Bytes<2U> preservedBytes{};
    assert(
        preservedBytes.PushBack(0xAAU) == ESPressio::Bounded::BytesPushBackResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral("\"Zg\"", preservedBytes);
    assert(decodeResult.Status == DeserialisationStatus::InvalidBase64);
    assert(preservedBytes.Size() == 1U && preservedBytes[0U] == 0xAAU);
    decodeResult = DecodeJsonLiteral("\"Zh==\"", preservedBytes);
    assert(decodeResult.Status == DeserialisationStatus::InvalidBase64);
    assert(preservedBytes.Size() == 1U && preservedBytes[0U] == 0xAAU);
    decodeResult = DecodeJsonLiteral("\"Zm8=\"", preservedBytes);
    assert(decodeResult.IsSuccessful());
    assert(preservedBytes.Size() == 2U);
    assert(preservedBytes[0U] == static_cast<std::uint8_t>('f'));
    assert(preservedBytes[1U] == static_cast<std::uint8_t>('o'));

    std::array<std::uint16_t, 3U> decodedArray{9U, 9U, 9U};
    decodeResult = DecodeJsonLiteral("[1,2,3]", decodedArray);
    assert(decodeResult.IsSuccessful());
    assert((decodedArray == std::array<std::uint16_t, 3U>{1U, 2U, 3U}));
    decodedArray = {9U, 9U, 9U};
    decodeResult = DecodeJsonLiteral("[1,2]", decodedArray);
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert((decodedArray == std::array<std::uint16_t, 3U>{9U, 9U, 9U}));
    decodeResult = DecodeJsonLiteral("[1,2,3,4]", decodedArray);
    assert(decodeResult.Status == DeserialisationStatus::TypeMismatch);
    assert((decodedArray == std::array<std::uint16_t, 3U>{9U, 9U, 9U}));

    std::uint16_t decodedCArray[2U]{8U, 9U};
    decodeResult = DecodeJsonLiteral("[4,5]", decodedCArray);
    assert(decodeResult.IsSuccessful());
    assert(decodedCArray[0U] == 4U && decodedCArray[1U] == 5U);

    ESPressio::Bounded::Vector<std::uint16_t, 2U> decodedVector{};
    assert(
        decodedVector.PushBack(99U) == ESPressio::Bounded::VectorPushBackResult::Succeeded
    );
    decodeResult = DecodeJsonLiteral("[6,7]", decodedVector);
    assert(decodeResult.IsSuccessful());
    assert(decodedVector.Size() == 2U && decodedVector[0U] == 6U && decodedVector[1U] == 7U);
    decodeResult = DecodeJsonLiteral("[1,2,3]", decodedVector);
    assert(decodeResult.Status == DeserialisationStatus::CapacityExceeded);
    assert(decodedVector.Size() == 2U && decodedVector[0U] == 6U && decodedVector[1U] == 7U);

    std::optional<std::uint32_t> decodedOptional{55U};
    decodeResult = DecodeJsonLiteral("null", decodedOptional);
    assert(decodeResult.IsSuccessful());
    assert(!decodedOptional.has_value());
    decodeResult = DecodeJsonLiteral("73", decodedOptional);
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

    decodeResult = DecodeJsonLiteral("{\"7\":\"x\"}", decodedPayload);
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
        reinterpret_cast<const std::uint8_t*>(ignoredUnknown),
        sizeof(ignoredUnknown) - 1U,
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
        reinterpret_cast<const std::uint8_t*>(duplicateNestedUnknown),
        sizeof(duplicateNestedUnknown) - 1U,
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
        reinterpret_cast<const std::uint8_t*>(duplicateUnknown),
        sizeof(duplicateUnknown) - 1U,
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
        reinterpret_cast<const std::uint8_t*>(limitedUnknown),
        sizeof(limitedUnknown) - 1U,
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
        reinterpret_cast<const std::uint8_t*>(limitedDepth),
        sizeof(limitedDepth) - 1U,
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::ResourceLimitExceeded);

    const char trailingJson[] = "{\"4\":{\"9\":1},\"7\":\"x\"} trailing";
    decodeResult = Deserialise<Json>(
        reinterpret_cast<const std::uint8_t*>(trailingJson),
        sizeof(trailingJson) - 1U,
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::TrailingData);

    TestSupport::StrongCounter decodedStrong{1U};
    decodeResult = DecodeJsonLiteral("44", decodedStrong);
    assert(decodeResult.IsSuccessful());
    assert(decodedStrong.Value == 44U);

    TestSupport::FallibleStrong decodedFallible{8U};
    decodeResult = DecodeJsonLiteral("99", decodedFallible);
    assert(decodeResult.Status == DeserialisationStatus::AdaptationFailed);
    assert(decodedFallible.Value == 8U);

    ESPressio::System::FieldIdentifier decodedFieldIdentifier{1U};
    decodeResult = DecodeJsonLiteral("42", decodedFieldIdentifier);
    assert(decodeResult.IsSuccessful());
    assert(decodedFieldIdentifier.Value() == 42U);

    const char malformedJson[] = "{\"4\":{\"9\":1},\"7\":\"x\",}";
    decodeResult = Deserialise<Json>(
        reinterpret_cast<const std::uint8_t*>(malformedJson),
        sizeof(malformedJson) - 1U,
        decodedPayload
    );
    assert(decodeResult.Status == DeserialisationStatus::MalformedRepresentation);

    return 0;
}
