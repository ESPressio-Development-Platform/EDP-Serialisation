#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

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

} // ESPressio::Serialisation
