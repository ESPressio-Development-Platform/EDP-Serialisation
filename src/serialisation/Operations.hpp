#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "JsonDecoding.hpp"
#include "JsonEncoding.hpp"
#include "Profiles.hpp"
#include "Results.hpp"
#include "SerialisableType.hpp"

namespace ESPressio::Serialisation {

    namespace Detail {

        /// Maps one internal JSON encoder outcome to the public Measure outcome family.
        ///
        /// @param status Internal encoder outcome.
        /// @return Equivalent public MeasurementStatus.
        constexpr MeasurementStatus ToMeasurementStatus(
            JsonEncodingStatus status
        ) noexcept {
            switch (status) {
                case JsonEncodingStatus::Succeeded:
                    return MeasurementStatus::Succeeded;
                case JsonEncodingStatus::ResourceLimitExceeded:
                    return MeasurementStatus::ResourceLimitExceeded;
                case JsonEncodingStatus::NonFiniteNumber:
                    return MeasurementStatus::NonFiniteNumber;
                case JsonEncodingStatus::InvalidUtf8:
                    return MeasurementStatus::InvalidUtf8;
                case JsonEncodingStatus::AdaptationFailed:
                    return MeasurementStatus::AdaptationFailed;
            }

            return MeasurementStatus::ResourceLimitExceeded;
        }

        /// Maps one internal JSON encoder outcome to the public Serialise outcome family.
        ///
        /// @param status Internal encoder outcome.
        /// @return Equivalent public SerialisationStatus.
        constexpr SerialisationStatus ToSerialisationStatus(
            JsonEncodingStatus status
        ) noexcept {
            switch (status) {
                case JsonEncodingStatus::Succeeded:
                    return SerialisationStatus::Succeeded;
                case JsonEncodingStatus::ResourceLimitExceeded:
                    return SerialisationStatus::ResourceLimitExceeded;
                case JsonEncodingStatus::NonFiniteNumber:
                    return SerialisationStatus::NonFiniteNumber;
                case JsonEncodingStatus::InvalidUtf8:
                    return SerialisationStatus::InvalidUtf8;
                case JsonEncodingStatus::AdaptationFailed:
                    return SerialisationStatus::AdaptationFailed;
            }

            return SerialisationStatus::ResourceLimitExceeded;
        }

        /// Maps one failed Measure outcome to its equivalent Serialise preflight outcome.
        ///
        /// @param status Public measurement outcome.
        /// @return Equivalent SerialisationStatus.
        constexpr SerialisationStatus ToSerialisationStatus(
            MeasurementStatus status
        ) noexcept {
            switch (status) {
                case MeasurementStatus::Succeeded:
                    return SerialisationStatus::Succeeded;
                case MeasurementStatus::InvalidArgument:
                    return SerialisationStatus::InvalidArgument;
                case MeasurementStatus::ResourceLimitExceeded:
                    return SerialisationStatus::ResourceLimitExceeded;
                case MeasurementStatus::InvalidLanguageMetadata:
                    return SerialisationStatus::InvalidLanguageMetadata;
                case MeasurementStatus::FieldNameCollision:
                    return SerialisationStatus::FieldNameCollision;
                case MeasurementStatus::LocalisationFailure:
                    return SerialisationStatus::LocalisationFailure;
                case MeasurementStatus::NonFiniteNumber:
                    return SerialisationStatus::NonFiniteNumber;
                case MeasurementStatus::InvalidUtf8:
                    return SerialisationStatus::InvalidUtf8;
                case MeasurementStatus::AdaptationFailed:
                    return SerialisationStatus::AdaptationFailed;
            }

            return SerialisationStatus::ResourceLimitExceeded;
        }


        /// Maps one internal JSON decoder outcome to the public Deserialise outcome family.
        ///
        /// @param status Internal decoder outcome.
        /// @return Equivalent public DeserialisationStatus.
        constexpr DeserialisationStatus ToDeserialisationStatus(
            JsonDecodingStatus status
        ) noexcept {
            switch (status) {
                case JsonDecodingStatus::Succeeded:
                    return DeserialisationStatus::Succeeded;
                case JsonDecodingStatus::MalformedRepresentation:
                    return DeserialisationStatus::MalformedRepresentation;
                case JsonDecodingStatus::ResourceLimitExceeded:
                    return DeserialisationStatus::ResourceLimitExceeded;
                case JsonDecodingStatus::UnknownField:
                    return DeserialisationStatus::UnknownField;
                case JsonDecodingStatus::DuplicateField:
                    return DeserialisationStatus::DuplicateField;
                case JsonDecodingStatus::MissingRequiredField:
                    return DeserialisationStatus::MissingRequiredField;
                case JsonDecodingStatus::TypeMismatch:
                    return DeserialisationStatus::TypeMismatch;
                case JsonDecodingStatus::NumericOutOfRange:
                    return DeserialisationStatus::NumericOutOfRange;
                case JsonDecodingStatus::NumericUnderflow:
                    return DeserialisationStatus::NumericUnderflow;
                case JsonDecodingStatus::NonFiniteNumber:
                    return DeserialisationStatus::NonFiniteNumber;
                case JsonDecodingStatus::InvalidUtf8:
                    return DeserialisationStatus::InvalidUtf8;
                case JsonDecodingStatus::InvalidBase64:
                    return DeserialisationStatus::InvalidBase64;
                case JsonDecodingStatus::CapacityExceeded:
                    return DeserialisationStatus::CapacityExceeded;
                case JsonDecodingStatus::AdaptationFailed:
                    return DeserialisationStatus::AdaptationFailed;
            }

            return DeserialisationStatus::MalformedRepresentation;
        }

        /// Validates that this implementation checkpoint owns the requested decoding profile.
        ///
        /// @tparam TCodec Compile-time codec tag.
        /// @tparam TRootProfile Compile-time root profile.
        /// @tparam TFieldProfile Compile-time Field-key profile.
        template<class TCodec, RootProfile TRootProfile, FieldProfile TFieldProfile>
        consteval void ValidateImplementedDecodingProfile() {
            static_assert(
                std::is_same_v<TCodec, Json>,
                "This implementation checkpoint currently provides decoding only for the Json codec"
            );
            static_assert(
                TRootProfile == RootProfile::KnownTypeBody,
                "This implementation checkpoint currently provides decoding only for RootProfile::KnownTypeBody"
            );
            static_assert(
                TFieldProfile == FieldProfile::Numeric,
                "This implementation checkpoint currently provides decoding only for FieldProfile::Numeric"
            );
        }

        /// Validates that this implementation checkpoint owns the requested encoding profile.
        ///
        /// @tparam TCodec Compile-time codec tag.
        /// @tparam TRootProfile Compile-time root profile.
        /// @tparam TFieldProfile Compile-time Field-key profile.
        template<class TCodec, RootProfile TRootProfile, FieldProfile TFieldProfile>
        consteval void ValidateImplementedEncodingProfile() {
            static_assert(
                std::is_same_v<TCodec, Json>,
                "This implementation checkpoint currently provides encoding only for the Json codec"
            );
            static_assert(
                TRootProfile == RootProfile::KnownTypeBody,
                "This implementation checkpoint currently provides encoding only for RootProfile::KnownTypeBody"
            );
            static_assert(
                TFieldProfile == FieldProfile::Numeric,
                "This implementation checkpoint currently provides encoding only for FieldProfile::Numeric"
            );
        }

    } // ESPressio::Serialisation::Detail

    /// Validates and measures one exact canonical encoded representation without retaining output bytes.
    ///
    /// The currently implemented encoding profile is Json + KnownTypeBody + Numeric. Other compile-time
    /// profile selections fail with focused diagnostics until their corresponding implementation slices land.
    ///
    /// @tparam TCodec Compile-time codec tag.
    /// @tparam TRootProfile Compile-time root profile.
    /// @tparam TFieldProfile Compile-time Field-key profile.
    /// @tparam TValue Serialisable source Type.
    /// @param value Source value to validate and measure.
    /// @return Exact encoded size on success plus operation-specific failure information otherwise.
    template<
        class TCodec,
        RootProfile TRootProfile = RootProfile::KnownTypeBody,
        FieldProfile TFieldProfile = FieldProfile::Numeric,
        SerialisableType TValue
    >
    MeasurementResult Measure(
        const TValue& value
    ) noexcept {
        Detail::ValidateImplementedEncodingProfile<
            TCodec,
            TRootProfile,
            TFieldProfile
        >();

        Detail::JsonCountingSink sink{};
        Diagnostic diagnostic{};
        const auto status = Detail::EncodeJsonValue(
            sink,
            value,
            diagnostic
        );
        const auto publicStatus = Detail::ToMeasurementStatus(status);

        return {
            publicStatus,
            publicStatus == MeasurementStatus::Succeeded
                ? sink.Size()
                : 0U,
            diagnostic
        };
    }

    /// Serialises one complete canonical representation into caller-owned contiguous storage.
    ///
    /// The operation always performs the same exact Measure validation before writing. Invalid source data,
    /// adaptation failure, or insufficient output capacity therefore leaves caller output untouched. The
    /// source and any canonical forward adapters must remain observationally stable for the duration of the
    /// call so the validated second traversal represents the same value measured by the preflight pass.
    ///
    /// @tparam TCodec Compile-time codec tag.
    /// @tparam TRootProfile Compile-time root profile.
    /// @tparam TFieldProfile Compile-time Field-key profile.
    /// @tparam TByteOperationsProvider Stateless EDP-Memory byte-copy provider used for caller-buffer writes.
    /// @tparam TValue Serialisable source Type.
    /// @param value Source value to serialise.
    /// @param output First byte of caller-owned output storage.
    /// @param capacity Writable output capacity in bytes.
    /// @return Exact required/written byte counts and operation-specific outcome.
    template<
        class TCodec,
        RootProfile TRootProfile = RootProfile::KnownTypeBody,
        FieldProfile TFieldProfile = FieldProfile::Numeric,
        class TByteOperationsProvider = ESPressio::Platform::Portable::Memory::ByteOperationsProvider,
        SerialisableType TValue
    >
    SerialisationResult Serialise(
        const TValue& value,
        std::uint8_t* output,
        std::size_t capacity
    ) noexcept {
        Detail::ValidateImplementedEncodingProfile<
            TCodec,
            TRootProfile,
            TFieldProfile
        >();

        const auto measurement = Measure<
            TCodec,
            TRootProfile,
            TFieldProfile
        >(value);

        if (!measurement.IsSuccessful()) {
            return {
                Detail::ToSerialisationStatus(measurement.Status),
                0U,
                0U,
                measurement.Detail
            };
        }

        if (output == nullptr) {
            return {
                SerialisationStatus::InvalidArgument,
                measurement.RequiredBytes,
                0U,
                {}
            };
        }

        if (capacity < measurement.RequiredBytes) {
            return {
                SerialisationStatus::OutputBufferTooSmall,
                measurement.RequiredBytes,
                0U,
                {}
            };
        }

        Detail::JsonBufferSink<TByteOperationsProvider> sink{
            output,
            capacity
        };
        Diagnostic diagnostic{};
        const auto status = Detail::EncodeJsonValue(
            sink,
            value,
            diagnostic
        );

        if (status != Detail::JsonEncodingStatus::Succeeded) {
            return {
                Detail::ToSerialisationStatus(status),
                measurement.RequiredBytes,
                0U,
                diagnostic
            };
        }

        return {
            SerialisationStatus::Succeeded,
            measurement.RequiredBytes,
            sink.Size(),
            {}
        };
    }

    /// Transactionally deserialises one complete JSON representation from caller-owned contiguous input.
    ///
    /// The operation first validates the complete replayable input without modifying destination state. Only
    /// after the first pass succeeds does it replay the same bytes to populate the destination. Canonical reverse
    /// adapters must therefore remain deterministic for an identical surrogate value throughout one call.
    ///
    /// The currently implemented decoding profile is Json + KnownTypeBody + Numeric. Other compile-time profile
    /// selections fail with focused diagnostics until their corresponding implementation slices land.
    ///
    /// @tparam TCodec Compile-time codec tag.
    /// @tparam TRootProfile Compile-time root profile.
    /// @tparam TFieldProfile Compile-time Field-key profile.
    /// @tparam TStrictness Unknown-Field handling policy.
    /// @tparam TParserLimits Compile-time nesting and unknown-skip resource policy.
    /// @tparam TValue Serialisable destination Type.
    /// @param input First byte of caller-owned immutable JSON input.
    /// @param length Number of bytes in the complete caller-owned input range.
    /// @param destination Existing destination object populated only after complete validation succeeds.
    /// @return Operation-specific outcome; BytesConsumed equals length only on success.
    template<
        class TCodec,
        RootProfile TRootProfile = RootProfile::KnownTypeBody,
        FieldProfile TFieldProfile = FieldProfile::Numeric,
        StrictnessPolicy TStrictness = StrictnessPolicy::Exact,
        class TParserLimits = DefaultParserLimits,
        SerialisableType TValue
    >
    DeserialisationResult Deserialise(
        const std::uint8_t* input,
        std::size_t length,
        TValue& destination
    ) noexcept {
        Detail::ValidateImplementedDecodingProfile<
            TCodec,
            TRootProfile,
            TFieldProfile
        >();

        if (input == nullptr) {
            return {
                DeserialisationStatus::InvalidArgument,
                0U,
                {}
            };
        }

        Detail::JsonInputCursor validationCursor{
            input,
            length
        };
        Detail::JsonSkipState validationSkipState{};
        Diagnostic validationDiagnostic{};
        const auto validationStatus = Detail::DecodeJsonValue<
            false,
            TStrictness,
            TParserLimits
        >(
            validationCursor,
            &destination,
            0U,
            validationSkipState,
            validationDiagnostic
        );

        if (validationStatus != Detail::JsonDecodingStatus::Succeeded) {
            return {
                Detail::ToDeserialisationStatus(validationStatus),
                0U,
                validationDiagnostic
            };
        }

        Detail::SkipJsonWhitespace(validationCursor);
        if (!validationCursor.IsAtEnd()) {
            validationDiagnostic.ByteOffset = validationCursor.Position();
            return {
                DeserialisationStatus::TrailingData,
                0U,
                validationDiagnostic
            };
        }

        Detail::JsonInputCursor populationCursor{
            input,
            length
        };
        Detail::JsonSkipState populationSkipState{};
        Diagnostic populationDiagnostic{};
        const auto populationStatus = Detail::DecodeJsonValue<
            true,
            TStrictness,
            TParserLimits
        >(
            populationCursor,
            &destination,
            0U,
            populationSkipState,
            populationDiagnostic
        );

        if (populationStatus != Detail::JsonDecodingStatus::Succeeded) {
            return {
                Detail::ToDeserialisationStatus(populationStatus),
                0U,
                populationDiagnostic
            };
        }

        Detail::SkipJsonWhitespace(populationCursor);
        return {
            DeserialisationStatus::Succeeded,
            length,
            {}
        };
    }

} // ESPressio::Serialisation
