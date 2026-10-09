#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

#include <ESPressio_Localisation.hpp>
#include <ESPressio_System.hpp>

#include "JsonDecoding.hpp"
#include "JsonEncoding.hpp"
#include "Results.hpp"

namespace ESPressio::Serialisation::Detail {

    /// Reserved JSON object member carrying the requested RFC5646 language identity.
    inline constexpr char JsonRfc5646MetadataKey[] = "RFC5646";

    /// LocalisedText encoding state bound to caller-owned Localisation services and scratch storage.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver specialization.
    template<class TResolver>
    struct JsonLocalisedFieldEncodingPolicy final {

        // Bound Localisation services.

        /// Resolver used for forward Field-name presentation.
        const TResolver* Resolver = nullptr;

        /// Requested-to-terminal language fallback policy used for every schema object.
        const Localisation::LocalisationContext* Context = nullptr;

        // Caller-owned scratch storage.

        /// Scratch retaining the current Field name while it is validated/emitted.
        Localisation::WritableTextView FieldNameScratch{};

        /// Independent scratch used for exact previous-Field collision comparison.
        Localisation::WritableTextView ComparisonScratch{};

    };

    /// LocalisedText decoding state bound to caller-owned Localisation services and scratch storage.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver specialization.
    template<class TResolver>
    struct JsonLocalisedFieldDecodingPolicy final {

        // Bound Localisation services.

        /// Resolver used for reverse Field-name resolution.
        const TResolver* Resolver = nullptr;

        /// ContractFamily terminal fallback language used when an effective requested language exists.
        Localisation::LanguageIdentifierView TerminalLanguage;

        /// Optional caller-requested language, lower priority than embedded RFC5646 metadata.
        std::optional<Localisation::LanguageIdentifierView> CallerLanguage{};

        // Caller-owned scratch storage.

        /// Scratch retaining one decoded textual schema member name at a time.
        Localisation::WritableTextView FieldNameScratch{};

    };

    /// Parser-limit adapter used for replay scans which are not unknown-Field skipping work.
    ///
    /// @tparam TParserLimits Caller-selected parser resource policy.
    template<class TParserLimits>
    struct JsonLocalisedReplayLimits final {

        /// Preserves the caller-selected syntactic nesting bound.
        inline static constexpr std::size_t MaximumNestingDepth =
            TParserLimits::MaximumNestingDepth;

        /// Replay work is bounded by input length rather than the unknown-Field skip budget.
        inline static constexpr std::size_t MaximumSkippedContainerItems =
            std::numeric_limits<std::size_t>::max();

    };

    /// Reports whether one caller-owned Localisation text buffer satisfies its pointer/capacity invariant.
    ///
    /// @param scratch Caller-owned writable text view being validated.
    /// @return true when zero capacity or a non-null writable pointer satisfies the view invariant.
    [[nodiscard]] constexpr bool IsLocalisedScratchValid(
        Localisation::WritableTextView scratch
    ) noexcept {
        return scratch.Capacity == 0U || scratch.Data != nullptr;
    }

    /// Reports whether two writable scratch ranges are independent and non-overlapping.
    ///
    /// @param left First caller-owned writable range.
    /// @param right Second caller-owned writable range.
    /// @return true only when the represented byte ranges cannot overlap.
    [[nodiscard]] inline bool AreLocalisedScratchViewsIndependent(
        Localisation::WritableTextView left,
        Localisation::WritableTextView right
    ) noexcept {
        if (left.Capacity == 0U || right.Capacity == 0U) { return true; }
        if (left.Data == nullptr || right.Data == nullptr) { return false; }

        const auto leftStart = reinterpret_cast<std::uintptr_t>(left.Data);
        const auto rightStart = reinterpret_cast<std::uintptr_t>(right.Data);
        if (
            left.Capacity > std::numeric_limits<std::uintptr_t>::max() - leftStart ||
            right.Capacity > std::numeric_limits<std::uintptr_t>::max() - rightStart
        ) {
            return false;
        }
        const auto leftEnd = leftStart + left.Capacity;
        const auto rightEnd = rightStart + right.Capacity;
        return leftEnd <= rightStart || rightEnd <= leftStart;
    }

    /// Compares one materialised UTF-8 text range against one fixed ASCII literal.
    ///
    /// @param data First UTF-8 byte of the materialised value.
    /// @param size Materialised byte count.
    /// @param expected First byte of the fixed comparison text.
    /// @param expectedSize Fixed comparison byte count.
    /// @return true only for exact byte equality.
    [[nodiscard]] inline bool IsLocalisedTextEqual(
        const char* data,
        std::size_t size,
        const char* expected,
        std::size_t expectedSize
    ) noexcept {
        if (size != expectedSize) { return false; }
        for (std::size_t index = 0U; index < size; ++index) {
            if (data[index] != expected[index]) { return false; }
        }
        return true;
    }

    /// Compares two materialised UTF-8 text ranges exactly by byte sequence.
    ///
    /// @param left First UTF-8 byte of the left value.
    /// @param leftSize Left byte count.
    /// @param right First UTF-8 byte of the right value.
    /// @param rightSize Right byte count.
    /// @return true only for exact byte equality.
    [[nodiscard]] inline bool AreLocalisedTextsEqual(
        const char* left,
        std::size_t leftSize,
        const char* right,
        std::size_t rightSize
    ) noexcept {
        if (leftSize != rightSize) { return false; }
        for (std::size_t index = 0U; index < leftSize; ++index) {
            if (left[index] != right[index]) { return false; }
        }
        return true;
    }

    /// Resolves one Field name into caller-owned scratch and normalizes Localisation failures.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @param policy Caller-bound Resolver/context/scratch policy.
    /// @param type Owning schema Type identity.
    /// @param field Type-local Field identity.
    /// @param scratch Caller-owned UTF-8 destination for the resolved name.
    /// @param size Receives the complete materialised name size on success.
    /// @return Succeeded, ResourceLimitExceeded for insufficient scratch, or LocalisationFailure.
    template<class TResolver>
    JsonEncodingStatus ResolveJsonFieldName(
        const JsonLocalisedFieldEncodingPolicy<TResolver>& policy,
        System::TypeIdentifier type,
        System::FieldIdentifier field,
        Localisation::WritableTextView scratch,
        std::size_t& size
    ) noexcept {
        const typename TResolver::FieldPresentationIdentifier identifier{
            type,
            field
        };
        const auto result = policy.Resolver->ResolveFieldName(
            *policy.Context,
            identifier,
            scratch,
            Localisation::TextOutputMode::RawUtf8
        );
        if (result.Status != Localisation::LocalisationStatus::Success) {
            size = 0U;
            return JsonEncodingStatus::LocalisationFailure;
        }
        if (
            result.Facts.IsSet(Localisation::LocalisationFact::BufferTooSmall) ||
            result.BytesWritten != result.RequiredBytes
        ) {
            size = 0U;
            return JsonEncodingStatus::ResourceLimitExceeded;
        }
        size = result.BytesWritten;
        return JsonEncodingStatus::Succeeded;
    }

    /// Checks the current materialised Field name against every earlier emitted schema Field.
    ///
    /// @tparam TIdentifier Current compile-time prior FieldIdentifier candidate.
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TValue Schema owner Type.
    /// @param policy Caller-bound LocalisedText encoding policy.
    /// @param value Source schema object used to determine which Optional Fields are emitted.
    /// @param currentIdentifier Current Field identity whose resolved name is being checked.
    /// @param currentName First byte of the current materialised name.
    /// @param currentNameSize Current materialised name byte count.
    /// @return Succeeded when unique; otherwise collision/resource/Localisation failure.
    template<
        std::uint16_t TIdentifier,
        class TResolver,
        class TValue
    >
    JsonEncodingStatus CheckEarlierLocalisedFieldNameCollision(
        JsonLocalisedFieldEncodingPolicy<TResolver>& policy,
        const TValue& value,
        System::FieldIdentifier currentIdentifier,
        const char* currentName,
        std::size_t currentNameSize
    ) noexcept {
        if constexpr (TIdentifier > 255U) {
            static_cast<void>(policy);
            static_cast<void>(value);
            static_cast<void>(currentIdentifier);
            static_cast<void>(currentName);
            static_cast<void>(currentNameSize);
            return JsonEncodingStatus::Succeeded;
        } else {
            if (TIdentifier >= currentIdentifier.Value()) {
                return JsonEncodingStatus::Succeeded;
            }

            using PreviousField = typename FieldBindingForIdentifier<
                System::FieldsOf<TValue>,
                static_cast<System::FieldIdentifier::Storage>(TIdentifier)
            >::Type;

            if constexpr (!std::is_void_v<PreviousField>) {
                using PreviousValue = std::remove_cv_t<System::FieldValueOf<PreviousField>>;
                const auto& previousValue = value.*PreviousField::Member;
                bool previousEmitted = true;
                if constexpr (OptionalValueTraits<PreviousValue>::IsValue) {
                    previousEmitted = previousValue.has_value();
                }

                if (previousEmitted) {
                    std::size_t previousNameSize = 0U;
                    const auto status = ResolveJsonFieldName(
                        policy,
                        System::TypeIdentifierOf<TValue>,
                        PreviousField::Identifier,
                        policy.ComparisonScratch,
                        previousNameSize
                    );
                    if (status != JsonEncodingStatus::Succeeded) { return status; }
                    if (AreLocalisedTextsEqual(
                        currentName,
                        currentNameSize,
                        policy.ComparisonScratch.Data,
                        previousNameSize
                    )) {
                        return JsonEncodingStatus::FieldNameCollision;
                    }
                }
            }

            return CheckEarlierLocalisedFieldNameCollision<TIdentifier + 1U>(
                policy,
                value,
                currentIdentifier,
                currentName,
                currentNameSize
            );
        }
    }

    /// Emits the reserved RFC5646 metadata member at the start of every LocalisedText schema object.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TSink JSON output sink Type.
    /// @tparam TValue Schema owner Type.
    /// @param policy Caller-bound LocalisedText encoding policy.
    /// @param sink Destination JSON sink.
    /// @param value Source schema object; not inspected for metadata emission.
    /// @param firstField Comma-state flag set false after successful metadata emission.
    /// @param diagnostic Diagnostic payload populated on encoding failure.
    /// @return Complete internal encoding outcome.
    template<class TResolver, class TSink, class TValue>
    JsonEncodingStatus EncodeJsonSchemaPrefix(
        JsonLocalisedFieldEncodingPolicy<TResolver>& policy,
        TSink& sink,
        const TValue& value,
        bool& firstField,
        Diagnostic& diagnostic
    ) noexcept {
        static_cast<void>(value);
        constexpr char MetadataPrefix[] = "\"RFC5646\":";
        auto status = WriteJsonBytes(
            sink,
            MetadataPrefix,
            sizeof(MetadataPrefix) - 1U,
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        status = EncodeJsonString(
            sink,
            policy.Context->RequestedLanguage.Data(),
            policy.Context->RequestedLanguage.Length(),
            diagnostic
        );
        if (status == JsonEncodingStatus::Succeeded) {
            firstField = false;
        }
        return status;
    }

    /// Resolves, collision-checks, and emits one LocalisedText schema Field key.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TSink JSON output sink Type.
    /// @tparam TValue Schema owner Type.
    /// @param policy Caller-bound LocalisedText encoding policy.
    /// @param sink Destination JSON sink.
    /// @param value Source schema object used for Optional/collision checks.
    /// @param identifier Stable FieldIdentifier being represented.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TResolver, class TSink, class TValue>
    JsonEncodingStatus EncodeJsonSchemaFieldKey(
        JsonLocalisedFieldEncodingPolicy<TResolver>& policy,
        TSink& sink,
        const TValue& value,
        System::FieldIdentifier identifier,
        Diagnostic& diagnostic
    ) noexcept {
        std::size_t nameSize = 0U;
        auto status = ResolveJsonFieldName(
            policy,
            System::TypeIdentifierOf<TValue>,
            identifier,
            policy.FieldNameScratch,
            nameSize
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        if (IsLocalisedTextEqual(
            policy.FieldNameScratch.Data,
            nameSize,
            JsonRfc5646MetadataKey,
            sizeof(JsonRfc5646MetadataKey) - 1U
        )) {
            return JsonEncodingStatus::FieldNameCollision;
        }

        status = CheckEarlierLocalisedFieldNameCollision<0U>(
            policy,
            value,
            identifier,
            policy.FieldNameScratch.Data,
            nameSize
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        status = EncodeJsonString(
            sink,
            policy.FieldNameScratch.Data,
            nameSize,
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }
        return WriteJsonByte(
            sink,
            static_cast<std::uint8_t>(':'),
            diagnostic
        );
    }

    /// Parses one JSON String into bounded caller-owned scratch.
    ///
    /// @param cursor Immutable caller-input cursor advanced over the complete String.
    /// @param scratch Caller-owned destination receiving decoded UTF-8 bytes.
    /// @param size Receives the decoded byte count.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal decoding outcome.
    inline JsonDecodingStatus ParseJsonStringToScratch(
        JsonInputCursor& cursor,
        Localisation::WritableTextView scratch,
        std::size_t& size,
        Diagnostic& diagnostic
    ) noexcept {
        size = 0U;
        return ParseJsonString(
            cursor,
            true,
            [&](std::uint8_t byte) noexcept {
                if (size >= scratch.Capacity) {
                    return JsonDecodingStatus::ResourceLimitExceeded;
                }
                scratch.Data[size++] = static_cast<char>(byte);
                return JsonDecodingStatus::Succeeded;
            },
            diagnostic
        );
    }

    /// Discovers and validates the optional RFC5646 metadata member without consuming the caller cursor.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @param policy Caller-bound LocalisedText decoding policy.
    /// @param source Replayable cursor positioned at the start of the schema object.
    /// @param depth Current syntactic nesting depth.
    /// @param hasEmbeddedLanguage Receives whether RFC5646 metadata was present.
    /// @param embeddedLanguage Receives decoded canonical-language candidate bytes.
    /// @param embeddedLanguageSize Receives the decoded metadata byte count.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal replay/discovery outcome.
    template<class TResolver, class TParserLimits>
    JsonDecodingStatus DiscoverJsonLocalisedLanguage(
        JsonLocalisedFieldDecodingPolicy<TResolver>& policy,
        const JsonInputCursor& source,
        std::size_t depth,
        bool& hasEmbeddedLanguage,
        std::array<
            char,
            std::numeric_limits<std::uint8_t>::max()
        >& embeddedLanguage,
        std::size_t& embeddedLanguageSize,
        Diagnostic& diagnostic
    ) noexcept {
        JsonInputCursor cursor{source.Data(), source.Length()};
        cursor.Seek(source.Position());
        hasEmbeddedLanguage = false;
        embeddedLanguageSize = 0U;

        auto status = EnterJsonContainer<TParserLimits>(
            depth,
            diagnostic,
            cursor.Position()
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        status = ConsumeJsonByte(
            cursor,
            static_cast<std::uint8_t>('{'),
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return JsonDecodingStatus::TypeMismatch; }
        SkipJsonWhitespace(cursor);
        bool first = true;
        JsonSkipState replaySkipState{};

        while (!cursor.IsAtEnd() && cursor.Current() != static_cast<std::uint8_t>('}')) {
            if (!first) {
                status = ConsumeJsonByte(
                    cursor,
                    static_cast<std::uint8_t>(','),
                    diagnostic
                );
                if (status != JsonDecodingStatus::Succeeded) { return status; }
                SkipJsonWhitespace(cursor);
            }
            first = false;

            std::size_t keySize = 0U;
            status = ParseJsonStringToScratch(
                cursor,
                policy.FieldNameScratch,
                keySize,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            const bool isMetadata = IsLocalisedTextEqual(
                policy.FieldNameScratch.Data,
                keySize,
                JsonRfc5646MetadataKey,
                sizeof(JsonRfc5646MetadataKey) - 1U
            );

            SkipJsonWhitespace(cursor);
            status = ConsumeJsonByte(
                cursor,
                static_cast<std::uint8_t>(':'),
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);

            if (isMetadata) {
                if (hasEmbeddedLanguage) {
                    return JsonDecodingStatus::DuplicateField;
                }
                hasEmbeddedLanguage = true;
                embeddedLanguageSize = 0U;
                status = ParseJsonString(
                    cursor,
                    true,
                    [&](std::uint8_t byte) noexcept {
                        if (embeddedLanguageSize >= embeddedLanguage.size()) {
                            return JsonDecodingStatus::InvalidLanguageMetadata;
                        }
                        embeddedLanguage[embeddedLanguageSize++] = static_cast<char>(byte);
                        return JsonDecodingStatus::Succeeded;
                    },
                    diagnostic
                );
                if (status != JsonDecodingStatus::Succeeded) {
                    return JsonDecodingStatus::InvalidLanguageMetadata;
                }
            } else {
                status = SkipJsonValue<JsonLocalisedReplayLimits<TParserLimits>>(
                    cursor,
                    depth + 1U,
                    replaySkipState,
                    diagnostic
                );
                if (status != JsonDecodingStatus::Succeeded) { return status; }
            }
            SkipJsonWhitespace(cursor);
        }

        status = ConsumeJsonByte(
            cursor,
            static_cast<std::uint8_t>('}'),
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        return JsonDecodingStatus::Succeeded;
    }

    /// Selects how one LocalisedText schema object resolves textual Field names.
    enum class JsonLocalisedLanguageSource : std::uint8_t {
        /// The schema object embeds canonical RFC5646 metadata.
        Embedded = 0U,
        /// No metadata exists and the caller supplied a requested language.
        Caller = 1U,
        /// Neither payload nor caller supplies a requested language; scan all supported packs.
        AcrossLanguages = 2U
    };

    /// Validates the effective language source for one LocalisedText schema object.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @param policy Caller-bound LocalisedText decoding policy.
    /// @param source Replayable cursor positioned at the schema object.
    /// @param depth Current syntactic nesting depth.
    /// @param languageSource Receives Embedded, Caller, or AcrossLanguages selection.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Succeeded when the selected language policy is valid; otherwise metadata/Localisation failure.
    template<class TResolver, class TParserLimits>
    JsonDecodingStatus ValidateJsonLocalisedLanguageSource(
        JsonLocalisedFieldDecodingPolicy<TResolver>& policy,
        const JsonInputCursor& source,
        std::size_t depth,
        JsonLocalisedLanguageSource& languageSource,
        Diagnostic& diagnostic
    ) noexcept {
        std::array<char, std::numeric_limits<std::uint8_t>::max()> embeddedLanguage{};
        std::size_t embeddedLanguageSize = 0U;
        bool hasEmbeddedLanguage = false;
        auto status = DiscoverJsonLocalisedLanguage<TResolver, TParserLimits>(
            policy,
            source,
            depth,
            hasEmbeddedLanguage,
            embeddedLanguage,
            embeddedLanguageSize,
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }

        if (hasEmbeddedLanguage) {
            const auto languageValidation = Localisation::LanguageIdentifierView::Validate(
                embeddedLanguage.data(),
                embeddedLanguageSize
            );
            if (!languageValidation.IsValuePresent) {
                return JsonDecodingStatus::InvalidLanguageMetadata;
            }
            if (
                policy.CallerLanguage.has_value() &&
                !languageValidation.Value.IsEqualTo(*policy.CallerLanguage)
            ) {
                return JsonDecodingStatus::InvalidLanguageMetadata;
            }
            const Localisation::LocalisationContext context{
                languageValidation.Value,
                policy.TerminalLanguage
            };
            const auto validation = policy.Resolver->ValidateContext(context);
            if (validation.Status != Localisation::ValidationStatus::Success) {
                return validation.Status == Localisation::ValidationStatus::InvalidArgument
                    ? JsonDecodingStatus::InvalidLanguageMetadata
                    : JsonDecodingStatus::LocalisationFailure;
            }
            languageSource = JsonLocalisedLanguageSource::Embedded;
            return JsonDecodingStatus::Succeeded;
        }

        if (policy.CallerLanguage.has_value()) {
            const Localisation::LocalisationContext context{
                *policy.CallerLanguage,
                policy.TerminalLanguage
            };
            const auto validation = policy.Resolver->ValidateContext(context);
            if (validation.Status != Localisation::ValidationStatus::Success) {
                return validation.Status == Localisation::ValidationStatus::InvalidArgument
                    ? JsonDecodingStatus::InvalidLanguageMetadata
                    : JsonDecodingStatus::LocalisationFailure;
            }
            languageSource = JsonLocalisedLanguageSource::Caller;
            return JsonDecodingStatus::Succeeded;
        }

        languageSource = JsonLocalisedLanguageSource::AcrossLanguages;
        return JsonDecodingStatus::Succeeded;
    }

    /// Resolves one textual Field name using the already validated language-source choice.
    ///
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @param policy Caller-bound LocalisedText decoding policy.
    /// @param source Replayable schema-object source used to recover embedded metadata when selected.
    /// @param depth Current syntactic nesting depth.
    /// @param languageSource Previously validated resolution mode.
    /// @param type Owning schema Type identity.
    /// @param fieldName Decoded textual Field name.
    /// @param diagnostic Diagnostic payload used if metadata replay fails.
    /// @return Raw EDP-Localisation reverse-resolution result.
    template<class TResolver, class TParserLimits>
    Localisation::FieldIdentifierResolutionResult ResolveJsonLocalisedFieldIdentifier(
        JsonLocalisedFieldDecodingPolicy<TResolver>& policy,
        const JsonInputCursor& source,
        std::size_t depth,
        JsonLocalisedLanguageSource languageSource,
        System::TypeIdentifier type,
        Localisation::TextView fieldName,
        Diagnostic& diagnostic
    ) noexcept {
        if (languageSource == JsonLocalisedLanguageSource::AcrossLanguages) {
            return policy.Resolver->ResolveFieldIdentifierAcrossLanguages(
                type,
                fieldName
            );
        }
        if (languageSource == JsonLocalisedLanguageSource::Caller) {
            const Localisation::LocalisationContext context{
                *policy.CallerLanguage,
                policy.TerminalLanguage
            };
            return policy.Resolver->ResolveFieldIdentifier(
                context,
                type,
                fieldName
            );
        }

        std::array<char, std::numeric_limits<std::uint8_t>::max()> embeddedLanguage{};
        std::size_t embeddedLanguageSize = 0U;
        bool hasEmbeddedLanguage = false;
        const auto discoveryStatus = DiscoverJsonLocalisedLanguage<TResolver, TParserLimits>(
            policy,
            source,
            depth,
            hasEmbeddedLanguage,
            embeddedLanguage,
            embeddedLanguageSize,
            diagnostic
        );
        if (discoveryStatus != JsonDecodingStatus::Succeeded || !hasEmbeddedLanguage) {
            return {
                Localisation::FieldIdentifierResolutionStatus::InvalidDataset,
                std::nullopt
            };
        }
        const auto languageValidation = Localisation::LanguageIdentifierView::Validate(
            embeddedLanguage.data(),
            embeddedLanguageSize
        );
        if (!languageValidation.IsValuePresent) {
            return {
                Localisation::FieldIdentifierResolutionStatus::InvalidArgument,
                std::nullopt
            };
        }
        const Localisation::LocalisationContext context{
            languageValidation.Value,
            policy.TerminalLanguage
        };
        return policy.Resolver->ResolveFieldIdentifier(
            context,
            type,
            fieldName
        );
    }

    /// Maps one Localisation reverse-resolution outcome into the JSON decoder vocabulary.
    ///
    /// @param status EDP-Localisation reverse-resolution outcome.
    /// @return Equivalent internal JSON decoding outcome, preserving NotFound/Ambiguous distinctions.
    inline JsonDecodingStatus MapLocalisedFieldResolutionStatus(
        Localisation::FieldIdentifierResolutionStatus status
    ) noexcept {
        switch (status) {
            case Localisation::FieldIdentifierResolutionStatus::Success:
                return JsonDecodingStatus::Succeeded;
            case Localisation::FieldIdentifierResolutionStatus::NotFound:
                return JsonDecodingStatus::FieldNameNotFound;
            case Localisation::FieldIdentifierResolutionStatus::Ambiguous:
                return JsonDecodingStatus::FieldNameAmbiguous;
            case Localisation::FieldIdentifierResolutionStatus::InvalidArgument:
            case Localisation::FieldIdentifierResolutionStatus::LanguagePackUnavailable:
            case Localisation::FieldIdentifierResolutionStatus::ProviderUnavailable:
            case Localisation::FieldIdentifierResolutionStatus::UnsupportedFormatVersion:
            case Localisation::FieldIdentifierResolutionStatus::IncompatibleLanguagePack:
            case Localisation::FieldIdentifierResolutionStatus::InvalidDataset:
            case Localisation::FieldIdentifierResolutionStatus::ReadFailure:
                return JsonDecodingStatus::LocalisationFailure;
        }
        return JsonDecodingStatus::LocalisationFailure;
    }

    /// Decodes one LocalisedText schema object in arbitrary JSON member order.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Unknown textual Field handling policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TResolver Concrete EDP-Localisation Resolver-compatible Type.
    /// @tparam TValue Schema owner Type.
    /// @param policy Caller-bound LocalisedText decoding policy.
    /// @param cursor Immutable caller-input cursor.
    /// @param destination Destination schema or validation seed.
    /// @param depth Current syntactic nesting depth.
    /// @param skipState Unknown-value skip accounting state.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal LocalisedText schema decoding outcome.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TResolver,
        class TValue
    >
    JsonDecodingStatus DecodeJsonSchemaWithFieldPolicy(
        JsonLocalisedFieldDecodingPolicy<TResolver>& policy,
        JsonInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        const JsonInputCursor schemaSource = cursor;
        JsonLocalisedLanguageSource languageSource =
            JsonLocalisedLanguageSource::AcrossLanguages;
        auto status = ValidateJsonLocalisedLanguageSource<TResolver, TParserLimits>(
            policy,
            schemaSource,
            depth,
            languageSource,
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) {
            diagnostic.ByteOffset = start;
            return status;
        }

        status = EnterJsonContainer<TParserLimits>(
            depth,
            diagnostic,
            start
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        status = ConsumeJsonByte(
            cursor,
            static_cast<std::uint8_t>('{'),
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return JsonDecodingStatus::TypeMismatch; }
        SkipJsonWhitespace(cursor);
        const auto objectContentStart = cursor.Position();
        SchemaFieldPresenceSet seen{};
        bool first = true;

        while (!cursor.IsAtEnd() && cursor.Current() != static_cast<std::uint8_t>('}')) {
            if (!first) {
                status = ConsumeJsonByte(
                    cursor,
                    static_cast<std::uint8_t>(','),
                    diagnostic
                );
                if (status != JsonDecodingStatus::Succeeded) { return status; }
                SkipJsonWhitespace(cursor);
            }
            first = false;
            const auto keyStart = cursor.Position();

            bool duplicateTextKey = false;
            status = HasEarlierEquivalentJsonObjectKey<JsonLocalisedReplayLimits<TParserLimits>>(
                cursor,
                objectContentStart,
                keyStart,
                depth,
                duplicateTextKey,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            if (duplicateTextKey) {
                diagnostic.ByteOffset = keyStart;
                diagnostic.Type = System::TypeIdentifierOf<TValue>;
                return JsonDecodingStatus::DuplicateField;
            }

            std::size_t keySize = 0U;
            status = ParseJsonStringToScratch(
                cursor,
                policy.FieldNameScratch,
                keySize,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            const bool isMetadata = IsLocalisedTextEqual(
                policy.FieldNameScratch.Data,
                keySize,
                JsonRfc5646MetadataKey,
                sizeof(JsonRfc5646MetadataKey) - 1U
            );

            SkipJsonWhitespace(cursor);
            status = ConsumeJsonByte(
                cursor,
                static_cast<std::uint8_t>(':'),
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);

            if (isMetadata) {
                status = ParseJsonString(
                    cursor,
                    true,
                    [](std::uint8_t byte) noexcept {
                        static_cast<void>(byte);
                        return JsonDecodingStatus::Succeeded;
                    },
                    diagnostic
                );
                if (status != JsonDecodingStatus::Succeeded) { return status; }
            } else {
                const Localisation::TextView fieldName{
                    policy.FieldNameScratch.Data,
                    keySize
                };
                const auto resolution = ResolveJsonLocalisedFieldIdentifier<TResolver, TParserLimits>(
                    policy,
                    schemaSource,
                    depth,
                    languageSource,
                    System::TypeIdentifierOf<TValue>,
                    fieldName,
                    diagnostic
                );
                const auto resolutionStatus = MapLocalisedFieldResolutionStatus(resolution.Status);

                if (resolutionStatus == JsonDecodingStatus::FieldNameNotFound) {
                    diagnostic.ByteOffset = keyStart;
                    diagnostic.Type = System::TypeIdentifierOf<TValue>;
                    if constexpr (TStrictness == StrictnessPolicy::Exact) {
                        return JsonDecodingStatus::FieldNameNotFound;
                    } else {
                        status = SkipJsonValue<TParserLimits>(
                            cursor,
                            depth + 1U,
                            skipState,
                            diagnostic
                        );
                        if (status != JsonDecodingStatus::Succeeded) { return status; }
                    }
                } else if (resolutionStatus != JsonDecodingStatus::Succeeded) {
                    diagnostic.ByteOffset = keyStart;
                    diagnostic.Type = System::TypeIdentifierOf<TValue>;
                    return resolutionStatus;
                } else {
                    if (!resolution.Field.has_value()) {
                        return JsonDecodingStatus::LocalisationFailure;
                    }
                    const auto identifier = *resolution.Field;
                    if (IsSchemaFieldSeen(
                        seen,
                        identifier
                    )) {
                        diagnostic.ByteOffset = keyStart;
                        diagnostic.Type = System::TypeIdentifierOf<TValue>;
                        diagnostic.Field = identifier;
                        return JsonDecodingStatus::DuplicateField;
                    }
                    MarkSchemaFieldSeen(
                        seen,
                        identifier
                    );

                    bool knownField = false;
                    status = DecodeJsonSchemaField<
                        TPopulate,
                        TStrictness,
                        TParserLimits
                    >(
                        policy,
                        cursor,
                        identifier,
                        destination,
                        depth + 1U,
                        skipState,
                        diagnostic,
                        knownField
                    );
                    if (status != JsonDecodingStatus::Succeeded) { return status; }
                    if (!knownField) {
                        diagnostic.Type = System::TypeIdentifierOf<TValue>;
                        diagnostic.Field = identifier;
                        if constexpr (TStrictness == StrictnessPolicy::Exact) {
                            diagnostic.ByteOffset = keyStart;
                            return JsonDecodingStatus::UnknownField;
                        } else {
                            status = SkipJsonValue<TParserLimits>(
                                cursor,
                                depth + 1U,
                                skipState,
                                diagnostic
                            );
                            if (status != JsonDecodingStatus::Succeeded) { return status; }
                        }
                    }
                }
            }

            SkipJsonWhitespace(cursor);
        }

        status = ConsumeJsonByte(
            cursor,
            static_cast<std::uint8_t>('}'),
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        return FinaliseJsonSchemaPresence<TPopulate>(
            seen,
            destination,
            diagnostic
        );
    }

} // ESPressio::Serialisation::Detail
