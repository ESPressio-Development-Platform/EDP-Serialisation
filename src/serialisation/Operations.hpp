#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "JsonDecoding.hpp"
#include "JsonEncoding.hpp"
#include "JsonEnvelope.hpp"
#include "JsonLocalisedText.hpp"
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
                case JsonEncodingStatus::InvalidLanguageMetadata:
                    return MeasurementStatus::InvalidLanguageMetadata;
                case JsonEncodingStatus::FieldNameCollision:
                    return MeasurementStatus::FieldNameCollision;
                case JsonEncodingStatus::LocalisationFailure:
                    return MeasurementStatus::LocalisationFailure;
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
                case JsonEncodingStatus::InvalidLanguageMetadata:
                    return SerialisationStatus::InvalidLanguageMetadata;
                case JsonEncodingStatus::FieldNameCollision:
                    return SerialisationStatus::FieldNameCollision;
                case JsonEncodingStatus::LocalisationFailure:
                    return SerialisationStatus::LocalisationFailure;
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
                case JsonDecodingStatus::UnsupportedEnvelopeVersion:
                    return DeserialisationStatus::UnsupportedEnvelopeVersion;
                case JsonDecodingStatus::TypeIdentifierMismatch:
                    return DeserialisationStatus::TypeIdentifierMismatch;
                case JsonDecodingStatus::InvalidLanguageMetadata:
                    return DeserialisationStatus::InvalidLanguageMetadata;
                case JsonDecodingStatus::FieldNameNotFound:
                    return DeserialisationStatus::FieldNameNotFound;
                case JsonDecodingStatus::FieldNameAmbiguous:
                    return DeserialisationStatus::FieldNameAmbiguous;
                case JsonDecodingStatus::LocalisationFailure:
                    return DeserialisationStatus::LocalisationFailure;
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
                TRootProfile == RootProfile::KnownTypeBody || TRootProfile == RootProfile::TypedEnvelope,
                "JSON decoding supports only the declared KnownTypeBody and TypedEnvelope root profiles"
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
                TRootProfile == RootProfile::KnownTypeBody || TRootProfile == RootProfile::TypedEnvelope,
                "JSON encoding supports only the declared KnownTypeBody and TypedEnvelope root profiles"
            );
            static_assert(
                TFieldProfile == FieldProfile::Numeric,
                "This implementation checkpoint currently provides encoding only for FieldProfile::Numeric"
            );
        }


        /// Validates the implemented JSON LocalisedText encoding profile selection.
        template<class TCodec, RootProfile TRootProfile, FieldProfile TFieldProfile>
        consteval void ValidateImplementedLocalisedEncodingProfile() {
            static_assert(
                std::is_same_v<TCodec, Json>,
                "LocalisedText encoding is implemented only for the Json codec"
            );
            static_assert(
                TRootProfile == RootProfile::KnownTypeBody || TRootProfile == RootProfile::TypedEnvelope,
                "JSON LocalisedText encoding supports KnownTypeBody and TypedEnvelope roots"
            );
            static_assert(
                TFieldProfile == FieldProfile::LocalisedText,
                "This overload implements only FieldProfile::LocalisedText"
            );
        }

        /// Validates the implemented JSON LocalisedText decoding profile selection.
        template<class TCodec, RootProfile TRootProfile, FieldProfile TFieldProfile>
        consteval void ValidateImplementedLocalisedDecodingProfile() {
            static_assert(
                std::is_same_v<TCodec, Json>,
                "LocalisedText decoding is implemented only for the Json codec"
            );
            static_assert(
                TRootProfile == RootProfile::KnownTypeBody || TRootProfile == RootProfile::TypedEnvelope,
                "JSON LocalisedText decoding supports KnownTypeBody and TypedEnvelope roots"
            );
            static_assert(
                TFieldProfile == FieldProfile::LocalisedText,
                "This overload implements only FieldProfile::LocalisedText"
            );
        }

        /// Runs transactional JSON decoding through one already constructed Field policy.
        ///
        /// @tparam TRootProfile Compile-time root profile.
        /// @tparam TStrictness Unknown-Field handling policy.
        /// @tparam TParserLimits Compile-time parser resource policy.
        /// @tparam TFieldPolicy Concrete schema Field-key policy shared across both replay passes.
        /// @tparam TValue Serialisable destination Type.
        /// @param input First immutable caller-owned JSON byte.
        /// @param length Complete caller-owned input length.
        /// @param destination Existing destination mutated only during the successful population pass.
        /// @param fieldPolicy Caller-bound Field-key policy retained by reference for both passes.
        /// @return Operation-specific transactional decode outcome.
        template<
            RootProfile TRootProfile,
            StrictnessPolicy TStrictness,
            class TParserLimits,
            class TFieldPolicy,
            SerialisableType TValue
        >
        DeserialisationResult DeserialiseJsonWithFieldPolicy(
            const std::uint8_t* input,
            std::size_t length,
            TValue& destination,
            TFieldPolicy& fieldPolicy
        ) noexcept {
            if (input == nullptr) {
                return {
                    DeserialisationStatus::InvalidArgument,
                    0U,
                    {}
                };
            }

            if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                static_assert(
                    System::IdentifiedType<TValue>,
                    "RootProfile::TypedEnvelope requires a root Type satisfying System::IdentifiedType"
                );
            }

            JsonInputCursor validationCursor{input, length};
            JsonSkipState validationSkipState{};
            Diagnostic validationDiagnostic{};
            const auto validationStatus = [&]() noexcept {
                if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                    return DecodeJsonTypedEnvelopeWithFieldPolicy<
                        false,
                        TStrictness,
                        TParserLimits
                    >(
                        fieldPolicy,
                        validationCursor,
                        &destination,
                        validationSkipState,
                        validationDiagnostic
                    );
                } else {
                    return DecodeJsonValueWithFieldPolicy<
                        false,
                        TStrictness,
                        TParserLimits
                    >(
                        fieldPolicy,
                        validationCursor,
                        &destination,
                        0U,
                        validationSkipState,
                        validationDiagnostic
                    );
                }
            }();

            if (validationStatus != JsonDecodingStatus::Succeeded) {
                return {
                    ToDeserialisationStatus(validationStatus),
                    0U,
                    validationDiagnostic
                };
            }

            SkipJsonWhitespace(validationCursor);
            if (!validationCursor.IsAtEnd()) {
                validationDiagnostic.ByteOffset = validationCursor.Position();
                return {
                    DeserialisationStatus::TrailingData,
                    0U,
                    validationDiagnostic
                };
            }

            JsonInputCursor populationCursor{input, length};
            JsonSkipState populationSkipState{};
            Diagnostic populationDiagnostic{};
            const auto populationStatus = [&]() noexcept {
                if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                    return DecodeJsonTypedEnvelopeWithFieldPolicy<
                        true,
                        TStrictness,
                        TParserLimits
                    >(
                        fieldPolicy,
                        populationCursor,
                        &destination,
                        populationSkipState,
                        populationDiagnostic
                    );
                } else {
                    return DecodeJsonValueWithFieldPolicy<
                        true,
                        TStrictness,
                        TParserLimits
                    >(
                        fieldPolicy,
                        populationCursor,
                        &destination,
                        0U,
                        populationSkipState,
                        populationDiagnostic
                    );
                }
            }();

            if (populationStatus != JsonDecodingStatus::Succeeded) {
                return {
                    ToDeserialisationStatus(populationStatus),
                    0U,
                    populationDiagnostic
                };
            }

            SkipJsonWhitespace(populationCursor);
            return {
                DeserialisationStatus::Succeeded,
                length,
                {}
            };
        }

    } // ESPressio::Serialisation::Detail

    /// Validates and measures one exact canonical encoded representation without retaining output bytes.
    ///
    /// The implemented JSON Numeric Field profile supports both KnownTypeBody and explicit TypedEnvelope roots.
    /// Other codec/Field-profile selections fail with focused diagnostics until their implementation slices land.
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

        if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
            static_assert(
                System::IdentifiedType<TValue>,
                "RootProfile::TypedEnvelope requires a root Type satisfying System::IdentifiedType"
            );
        }

        Detail::JsonCountingSink sink{};
        Diagnostic diagnostic{};
        const auto status = [&]() noexcept {
            if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                return Detail::EncodeJsonTypedEnvelope(
                    sink,
                    value,
                    diagnostic
                );
            } else {
                return Detail::EncodeJsonValue(
                    sink,
                    value,
                    diagnostic
                );
            }
        }();
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
        const auto status = [&]() noexcept {
            if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                return Detail::EncodeJsonTypedEnvelope(
                    sink,
                    value,
                    diagnostic
                );
            } else {
                return Detail::EncodeJsonValue(
                    sink,
                    value,
                    diagnostic
                );
            }
        }();

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
    /// The implemented JSON Numeric Field profile supports both KnownTypeBody and explicit TypedEnvelope roots.
    /// Other codec/Field-profile selections fail with focused diagnostics until their implementation slices land.
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
        if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
            static_assert(
                System::IdentifiedType<TValue>,
                "RootProfile::TypedEnvelope requires a root Type satisfying System::IdentifiedType"
            );
        }

        const auto validationStatus = [&]() noexcept {
            if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                return Detail::DecodeJsonTypedEnvelope<
                    false,
                    TStrictness,
                    TParserLimits
                >(
                    validationCursor,
                    &destination,
                    validationSkipState,
                    validationDiagnostic
                );
            } else {
                return Detail::DecodeJsonValue<
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
            }
        }();

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
        const auto populationStatus = [&]() noexcept {
            if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                return Detail::DecodeJsonTypedEnvelope<
                    true,
                    TStrictness,
                    TParserLimits
                >(
                    populationCursor,
                    &destination,
                    populationSkipState,
                    populationDiagnostic
                );
            } else {
                return Detail::DecodeJsonValue<
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
            }
        }();

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


    /// Measures canonical JSON using Localisation-resolved textual schema Field names.
    ///
    /// Caller-owned scratch buffers must be valid and non-overlapping. Resolver/context data must remain
    /// observationally stable for this complete traversal. Every schema object emits RFC5646 metadata first.
    ///
    /// @tparam TCodec Compile-time codec tag; V1 currently requires Json.
    /// @tparam TRootProfile Known-Type Body or explicit Typed Envelope root profile.
    /// @tparam TFieldProfile Compile-time Field-key profile; this overload requires LocalisedText.
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TValue Serialisable source Type.
    /// @param value Source value to validate and measure.
    /// @param resolver Caller-owned forward/reverse Localisation service.
    /// @param context Requested/terminal language policy applied to every represented schema object.
    /// @param fieldNameScratch Caller-owned UTF-8 scratch for the current resolved Field name.
    /// @param comparisonScratch Independent caller-owned UTF-8 scratch used for collision proof.
    /// @return Exact encoded size on success plus Localisation/resource failure information otherwise.
    template<
        class TCodec,
        RootProfile TRootProfile = RootProfile::KnownTypeBody,
        FieldProfile TFieldProfile = FieldProfile::LocalisedText,
        class TResolver,
        SerialisableType TValue
    >
    MeasurementResult Measure(
        const TValue& value,
        const TResolver& resolver,
        const Localisation::LocalisationContext& context,
        Localisation::WritableTextView fieldNameScratch,
        Localisation::WritableTextView comparisonScratch
    ) noexcept {
        Detail::ValidateImplementedLocalisedEncodingProfile<
            TCodec,
            TRootProfile,
            TFieldProfile
        >();

        if (
            !Detail::IsLocalisedScratchValid(fieldNameScratch) ||
            !Detail::IsLocalisedScratchValid(comparisonScratch) ||
            !Detail::AreLocalisedScratchViewsIndependent(fieldNameScratch, comparisonScratch)
        ) {
            return {MeasurementStatus::InvalidArgument, 0U, {}};
        }

        const auto contextValidation = Localisation::ValidateLocalisationContext(context);
        if (
            contextValidation.Status !=
            Localisation::LocalisationContextValidationStatus::Succeeded
        ) {
            return {MeasurementStatus::InvalidLanguageMetadata, 0U, {}};
        }
        const auto resolverValidation = resolver.ValidateContext(context);
        if (resolverValidation.Status != Localisation::ValidationStatus::Success) {
            return {
                resolverValidation.Status == Localisation::ValidationStatus::InvalidArgument
                    ? MeasurementStatus::InvalidLanguageMetadata
                    : MeasurementStatus::LocalisationFailure,
                0U,
                {}
            };
        }

        if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
            static_assert(
                System::IdentifiedType<TValue>,
                "RootProfile::TypedEnvelope requires a root Type satisfying System::IdentifiedType"
            );
        }

        Detail::JsonLocalisedFieldEncodingPolicy<TResolver> fieldPolicy{
            &resolver,
            &context,
            fieldNameScratch,
            comparisonScratch
        };
        Detail::JsonCountingSink sink{};
        Diagnostic diagnostic{};
        const auto status = [&]() noexcept {
            if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                return Detail::EncodeJsonTypedEnvelopeWithFieldPolicy(
                    fieldPolicy,
                    sink,
                    value,
                    diagnostic
                );
            } else {
                return Detail::EncodeJsonValueWithFieldPolicy(
                    fieldPolicy,
                    sink,
                    value,
                    diagnostic
                );
            }
        }();
        const auto publicStatus = Detail::ToMeasurementStatus(status);
        return {
            publicStatus,
            publicStatus == MeasurementStatus::Succeeded ? sink.Size() : 0U,
            diagnostic
        };
    }

    /// Serialises canonical JSON using Localisation-resolved textual schema Field names.
    ///
    /// The operation performs the same exact LocalisedText Measure preflight before writing, so invalid
    /// Localisation metadata, name collision, scratch exhaustion, or insufficient output cannot partially write.
    ///
    /// @tparam TCodec Compile-time codec tag; V1 currently requires Json.
    /// @tparam TRootProfile Known-Type Body or explicit Typed Envelope root profile.
    /// @tparam TFieldProfile Compile-time Field-key profile; this overload requires LocalisedText.
    /// @tparam TByteOperationsProvider Stateless EDP-Memory provider used for caller-buffer writes.
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TValue Serialisable source Type.
    /// @param value Source value to serialise.
    /// @param output First byte of caller-owned output storage.
    /// @param capacity Writable output capacity in bytes.
    /// @param resolver Caller-owned forward/reverse Localisation service.
    /// @param context Requested/terminal language policy applied to every represented schema object.
    /// @param fieldNameScratch Caller-owned UTF-8 scratch for the current resolved Field name.
    /// @param comparisonScratch Independent caller-owned UTF-8 scratch used for collision proof.
    /// @return Exact required/written byte counts and operation-specific outcome.
    template<
        class TCodec,
        RootProfile TRootProfile = RootProfile::KnownTypeBody,
        FieldProfile TFieldProfile = FieldProfile::LocalisedText,
        class TByteOperationsProvider = ESPressio::Platform::Portable::Memory::ByteOperationsProvider,
        class TResolver,
        SerialisableType TValue
    >
    SerialisationResult Serialise(
        const TValue& value,
        std::uint8_t* output,
        std::size_t capacity,
        const TResolver& resolver,
        const Localisation::LocalisationContext& context,
        Localisation::WritableTextView fieldNameScratch,
        Localisation::WritableTextView comparisonScratch
    ) noexcept {
        Detail::ValidateImplementedLocalisedEncodingProfile<
            TCodec,
            TRootProfile,
            TFieldProfile
        >();
        const auto measurement = Measure<
            TCodec,
            TRootProfile,
            TFieldProfile
        >(
            value,
            resolver,
            context,
            fieldNameScratch,
            comparisonScratch
        );
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

        Detail::JsonLocalisedFieldEncodingPolicy<TResolver> fieldPolicy{
            &resolver,
            &context,
            fieldNameScratch,
            comparisonScratch
        };
        Detail::JsonBufferSink<TByteOperationsProvider> sink{output, capacity};
        Diagnostic diagnostic{};
        const auto status = [&]() noexcept {
            if constexpr (TRootProfile == RootProfile::TypedEnvelope) {
                return Detail::EncodeJsonTypedEnvelopeWithFieldPolicy(
                    fieldPolicy,
                    sink,
                    value,
                    diagnostic
                );
            } else {
                return Detail::EncodeJsonValueWithFieldPolicy(
                    fieldPolicy,
                    sink,
                    value,
                    diagnostic
                );
            }
        }();
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

    /// Transactionally decodes LocalisedText JSON using an explicit caller language context.
    ///
    /// Embedded RFC5646 metadata has priority and must equal context.RequestedLanguage exactly when present.
    /// The Resolver/pack source must remain observationally stable across validation and population passes.
    ///
    /// @tparam TCodec Compile-time codec tag; V1 currently requires Json.
    /// @tparam TRootProfile Known-Type Body or explicit Typed Envelope root profile.
    /// @tparam TFieldProfile Compile-time Field-key profile; this overload requires LocalisedText.
    /// @tparam TStrictness Unknown textual Field handling policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TValue Serialisable destination Type.
    /// @param input First immutable caller-owned JSON byte.
    /// @param length Complete caller-owned input length.
    /// @param destination Existing destination populated only after complete validation succeeds.
    /// @param resolver Caller-owned forward/reverse Localisation service.
    /// @param context Requested/terminal language policy; embedded metadata must match RequestedLanguage.
    /// @param fieldNameScratch Caller-owned UTF-8 scratch used for one textual key at a time.
    /// @return Operation-specific transactional decode outcome.
    template<
        class TCodec,
        RootProfile TRootProfile = RootProfile::KnownTypeBody,
        FieldProfile TFieldProfile = FieldProfile::LocalisedText,
        StrictnessPolicy TStrictness = StrictnessPolicy::Exact,
        class TParserLimits = DefaultParserLimits,
        class TResolver,
        SerialisableType TValue
    >
    DeserialisationResult Deserialise(
        const std::uint8_t* input,
        std::size_t length,
        TValue& destination,
        const TResolver& resolver,
        const Localisation::LocalisationContext& context,
        Localisation::WritableTextView fieldNameScratch
    ) noexcept {
        Detail::ValidateImplementedLocalisedDecodingProfile<
            TCodec,
            TRootProfile,
            TFieldProfile
        >();
        if (!Detail::IsLocalisedScratchValid(fieldNameScratch)) {
            return {DeserialisationStatus::InvalidArgument, 0U, {}};
        }
        const auto contextValidation = Localisation::ValidateLocalisationContext(context);
        if (
            contextValidation.Status !=
            Localisation::LocalisationContextValidationStatus::Succeeded
        ) {
            return {DeserialisationStatus::InvalidLanguageMetadata, 0U, {}};
        }

        Detail::JsonLocalisedFieldDecodingPolicy<TResolver> fieldPolicy{
            &resolver,
            context.TerminalLanguage,
            context.RequestedLanguage,
            fieldNameScratch
        };
        return Detail::DeserialiseJsonWithFieldPolicy<
            TRootProfile,
            TStrictness,
            TParserLimits
        >(
            input,
            length,
            destination,
            fieldPolicy
        );
    }

    /// Transactionally decodes LocalisedText JSON without a caller-requested language.
    ///
    /// Embedded RFC5646 metadata, when present, is resolved through terminalLanguage. When metadata is absent,
    /// Field names are reverse-resolved across every generated supported language and ambiguity is a hard error.
    ///
    /// @tparam TCodec Compile-time codec tag; V1 currently requires Json.
    /// @tparam TRootProfile Known-Type Body or explicit Typed Envelope root profile.
    /// @tparam TFieldProfile Compile-time Field-key profile; this overload requires LocalisedText.
    /// @tparam TStrictness Unknown textual Field handling policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TValue Serialisable destination Type.
    /// @param input First immutable caller-owned JSON byte.
    /// @param length Complete caller-owned input length.
    /// @param destination Existing destination populated only after complete validation succeeds.
    /// @param resolver Caller-owned forward/reverse Localisation service.
    /// @param terminalLanguage Canonical ContractFamily terminal fallback language.
    /// @param fieldNameScratch Caller-owned UTF-8 scratch used for one textual key at a time.
    /// @return Operation-specific transactional decode outcome.
    template<
        class TCodec,
        RootProfile TRootProfile = RootProfile::KnownTypeBody,
        FieldProfile TFieldProfile = FieldProfile::LocalisedText,
        StrictnessPolicy TStrictness = StrictnessPolicy::Exact,
        class TParserLimits = DefaultParserLimits,
        class TResolver,
        SerialisableType TValue
    >
    DeserialisationResult Deserialise(
        const std::uint8_t* input,
        std::size_t length,
        TValue& destination,
        const TResolver& resolver,
        Localisation::LanguageIdentifierView terminalLanguage,
        Localisation::WritableTextView fieldNameScratch
    ) noexcept {
        Detail::ValidateImplementedLocalisedDecodingProfile<
            TCodec,
            TRootProfile,
            TFieldProfile
        >();
        if (!Detail::IsLocalisedScratchValid(fieldNameScratch)) {
            return {DeserialisationStatus::InvalidArgument, 0U, {}};
        }
        const auto terminalValidation = Localisation::LanguageIdentifierView::Validate(
            terminalLanguage.Data(),
            terminalLanguage.Length()
        );
        if (!terminalValidation.IsValuePresent) {
            return {DeserialisationStatus::InvalidLanguageMetadata, 0U, {}};
        }

        Detail::JsonLocalisedFieldDecodingPolicy<TResolver> fieldPolicy{
            &resolver,
            terminalValidation.Value,
            std::nullopt,
            fieldNameScratch
        };
        return Detail::DeserialiseJsonWithFieldPolicy<
            TRootProfile,
            TStrictness,
            TParserLimits
        >(
            input,
            length,
            destination,
            fieldPolicy
        );
    }

} // ESPressio::Serialisation
