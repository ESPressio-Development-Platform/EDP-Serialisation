#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <ESPressio_System.hpp>

#include "JsonDecoding.hpp"
#include "JsonEncoding.hpp"
#include "Profiles.hpp"
#include "Results.hpp"

namespace ESPressio::Serialisation::Detail {

    /// Identifies one member of the JSON Typed Envelope root object.
    enum class JsonEnvelopeRootMember : std::uint8_t {
        /// Reserved `$edp` metadata object.
        Edp = 0U,
        /// Wrapped root value body.
        Value = 1U,
        /// Decoded member name is not part of the V1 envelope contract.
        Unknown = 2U
    };

    /// Identifies one member of the JSON Typed Envelope metadata object.
    enum class JsonEnvelopeMetadataMember : std::uint8_t {
        /// Typed-envelope representation version member `v`.
        Version = 0U,
        /// Canonical root semantic TypeIdentifier member `type`.
        Type = 1U,
        /// Decoded metadata member name is not part of the V1 envelope contract.
        Unknown = 2U
    };

    /// Parses one Typed Envelope root-member key without retaining dynamic text.
    inline JsonDecodingStatus ParseJsonEnvelopeRootMember(
        JsonInputCursor& cursor,
        JsonEnvelopeRootMember& member,
        Diagnostic& diagnostic
    ) noexcept {
        constexpr char EdpName[] = "$edp";
        constexpr char ValueName[] = "value";
        std::size_t index = 0U;
        bool matchesEdp = true;
        bool matchesValue = true;

        const auto status = ParseJsonString(
            cursor,
            false,
            [&](std::uint8_t byte) noexcept {
                matchesEdp = matchesEdp && index < sizeof(EdpName) - 1U &&
                    byte == static_cast<std::uint8_t>(EdpName[index]);
                matchesValue = matchesValue && index < sizeof(ValueName) - 1U &&
                    byte == static_cast<std::uint8_t>(ValueName[index]);
                ++index;
                return JsonDecodingStatus::Succeeded;
            },
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }

        if (matchesEdp && index == sizeof(EdpName) - 1U) {
            member = JsonEnvelopeRootMember::Edp;
        } else if (matchesValue && index == sizeof(ValueName) - 1U) {
            member = JsonEnvelopeRootMember::Value;
        } else {
            member = JsonEnvelopeRootMember::Unknown;
        }
        return JsonDecodingStatus::Succeeded;
    }

    /// Parses one Typed Envelope metadata-member key without retaining dynamic text.
    inline JsonDecodingStatus ParseJsonEnvelopeMetadataMember(
        JsonInputCursor& cursor,
        JsonEnvelopeMetadataMember& member,
        Diagnostic& diagnostic
    ) noexcept {
        constexpr char VersionName[] = "v";
        constexpr char TypeName[] = "type";
        std::size_t index = 0U;
        bool matchesVersion = true;
        bool matchesType = true;

        const auto status = ParseJsonString(
            cursor,
            false,
            [&](std::uint8_t byte) noexcept {
                matchesVersion = matchesVersion && index < sizeof(VersionName) - 1U &&
                    byte == static_cast<std::uint8_t>(VersionName[index]);
                matchesType = matchesType && index < sizeof(TypeName) - 1U &&
                    byte == static_cast<std::uint8_t>(TypeName[index]);
                ++index;
                return JsonDecodingStatus::Succeeded;
            },
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }

        if (matchesVersion && index == sizeof(VersionName) - 1U) {
            member = JsonEnvelopeMetadataMember::Version;
        } else if (matchesType && index == sizeof(TypeName) - 1U) {
            member = JsonEnvelopeMetadataMember::Type;
        } else {
            member = JsonEnvelopeMetadataMember::Unknown;
        }
        return JsonDecodingStatus::Succeeded;
    }

    /// Emits one canonical lowercase hexadecimal TypeIdentifier JSON String.
    template<class TSink>
    JsonEncodingStatus EncodeJsonTypeIdentifier(
        TSink& sink,
        System::TypeIdentifier identifier,
        Diagnostic& diagnostic
    ) noexcept {
        constexpr char Hex[] = "0123456789abcdef";
        auto status = WriteJsonByte(sink, static_cast<std::uint8_t>('"'), diagnostic);
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        for (const auto byte : identifier.Bytes()) {
            const char encoded[] = {
                Hex[(byte >> 4U) & 0x0FU],
                Hex[byte & 0x0FU]
            };
            status = WriteJsonBytes(sink, encoded, sizeof(encoded), diagnostic);
            if (status != JsonEncodingStatus::Succeeded) { return status; }
        }
        return WriteJsonByte(sink, static_cast<std::uint8_t>('"'), diagnostic);
    }

    /// Emits one canonical JSON Typed Envelope around an already supported body value.
    template<class TSink, class TValue>
    JsonEncodingStatus EncodeJsonTypedEnvelope(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        static_assert(System::IdentifiedType<TValue>,
            "Typed Envelope roots must satisfy System::IdentifiedType");
        constexpr char Prefix[] = "{\"$edp\":{\"v\":1,\"type\":";
        constexpr char Middle[] = "},\"value\":";
        auto status = WriteJsonBytes(sink, Prefix, sizeof(Prefix) - 1U, diagnostic);
        if (status != JsonEncodingStatus::Succeeded) { return status; }
        status = EncodeJsonTypeIdentifier(sink, System::TypeIdentifierOf<TValue>, diagnostic);
        if (status != JsonEncodingStatus::Succeeded) { return status; }
        status = WriteJsonBytes(sink, Middle, sizeof(Middle) - 1U, diagnostic);
        if (status != JsonEncodingStatus::Succeeded) { return status; }
        status = EncodeJsonValue(sink, value, diagnostic);
        if (status != JsonEncodingStatus::Succeeded) { return status; }
        return WriteJsonByte(sink, static_cast<std::uint8_t>('}'), diagnostic);
    }

    /// Parses and verifies the Typed Envelope version member.
    inline JsonDecodingStatus DecodeJsonEnvelopeVersion(
        JsonInputCursor& cursor,
        Diagnostic& diagnostic
    ) noexcept {
        JsonNumberToken token{};
        auto status = ParseJsonNumberToken(cursor, token, diagnostic);
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        std::uint8_t version = 0U;
        status = ConvertJsonIntegerToken(cursor, token, version, diagnostic);
        if (status != JsonDecodingStatus::Succeeded || version != TypedEnvelopeVersion) {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::UnsupportedEnvelopeVersion;
        }
        return JsonDecodingStatus::Succeeded;
    }

    /// Parses and verifies the canonical lowercase hexadecimal root TypeIdentifier.
    template<class TValue>
    JsonDecodingStatus DecodeJsonEnvelopeTypeIdentifier(
        JsonInputCursor& cursor,
        Diagnostic& diagnostic
    ) noexcept {
        static_assert(System::IdentifiedType<TValue>,
            "Typed Envelope roots must satisfy System::IdentifiedType");
        const auto start = cursor.Position();
        const auto& expected = System::TypeIdentifierOf<TValue>.Bytes();
        std::size_t index = 0U;
        bool malformed = false;
        bool mismatch = false;

        const auto status = ParseJsonString(
            cursor,
            false,
            [&](std::uint8_t byte) noexcept {
                if (index >= 16U) {
                    malformed = true;
                    ++index;
                    return JsonDecodingStatus::Succeeded;
                }
                std::uint8_t nibble = 0U;
                if (byte >= static_cast<std::uint8_t>('0') && byte <= static_cast<std::uint8_t>('9')) {
                    nibble = static_cast<std::uint8_t>(byte - static_cast<std::uint8_t>('0'));
                } else if (byte >= static_cast<std::uint8_t>('a') && byte <= static_cast<std::uint8_t>('f')) {
                    nibble = static_cast<std::uint8_t>(10U + byte - static_cast<std::uint8_t>('a'));
                } else {
                    malformed = true;
                    ++index;
                    return JsonDecodingStatus::Succeeded;
                }
                const auto expectedByte = expected[index / 2U];
                const auto expectedNibble = (index % 2U == 0U)
                    ? static_cast<std::uint8_t>(expectedByte >> 4U)
                    : static_cast<std::uint8_t>(expectedByte & 0x0FU);
                mismatch = mismatch || nibble != expectedNibble;
                ++index;
                return JsonDecodingStatus::Succeeded;
            },
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        if (malformed || index != 16U) {
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::MalformedRepresentation;
        }
        if (mismatch) {
            diagnostic.ByteOffset = start;
            diagnostic.Type = System::TypeIdentifierOf<TValue>;
            return JsonDecodingStatus::TypeIdentifierMismatch;
        }
        return JsonDecodingStatus::Succeeded;
    }

    /// Parses the complete `$edp` Typed Envelope metadata object.
    template<class TParserLimits, class TValue>
    JsonDecodingStatus DecodeJsonEnvelopeMetadata(
        JsonInputCursor& cursor,
        std::size_t depth,
        Diagnostic& diagnostic
    ) noexcept {
        auto status = EnterJsonContainer<TParserLimits>(depth, diagnostic, cursor.Position());
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>('{'), diagnostic);
        if (status != JsonDecodingStatus::Succeeded) { return JsonDecodingStatus::TypeMismatch; }
        SkipJsonWhitespace(cursor);
        bool seenVersion = false;
        bool seenType = false;
        bool first = true;

        while (!cursor.IsAtEnd() && cursor.Current() != static_cast<std::uint8_t>('}')) {
            if (!first) {
                status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(','), diagnostic);
                if (status != JsonDecodingStatus::Succeeded) { return status; }
                SkipJsonWhitespace(cursor);
            }
            first = false;
            JsonEnvelopeMetadataMember member = JsonEnvelopeMetadataMember::Unknown;
            status = ParseJsonEnvelopeMetadataMember(cursor, member, diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            if (member == JsonEnvelopeMetadataMember::Unknown) {
                return JsonDecodingStatus::MalformedRepresentation;
            }
            bool* seen = member == JsonEnvelopeMetadataMember::Version ? &seenVersion : &seenType;
            if (*seen) { return JsonDecodingStatus::DuplicateField; }
            *seen = true;
            SkipJsonWhitespace(cursor);
            status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(':'), diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
            status = member == JsonEnvelopeMetadataMember::Version
                ? DecodeJsonEnvelopeVersion(cursor, diagnostic)
                : DecodeJsonEnvelopeTypeIdentifier<TValue>(cursor, diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
        }
        status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>('}'), diagnostic);
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        if (!seenVersion || !seenType) { return JsonDecodingStatus::MalformedRepresentation; }
        return JsonDecodingStatus::Succeeded;
    }

    /// Parses one complete JSON Typed Envelope and delegates its body to the existing value decoder.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    JsonDecodingStatus DecodeJsonTypedEnvelope(
        JsonInputCursor& cursor,
        TValue* destination,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        static_assert(System::IdentifiedType<TValue>,
            "Typed Envelope roots must satisfy System::IdentifiedType");
        SkipJsonWhitespace(cursor);
        auto status = EnterJsonContainer<TParserLimits>(0U, diagnostic, cursor.Position());
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>('{'), diagnostic);
        if (status != JsonDecodingStatus::Succeeded) { return JsonDecodingStatus::TypeMismatch; }
        SkipJsonWhitespace(cursor);
        bool seenEdp = false;
        bool seenValue = false;
        bool first = true;

        while (!cursor.IsAtEnd() && cursor.Current() != static_cast<std::uint8_t>('}')) {
            if (!first) {
                status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(','), diagnostic);
                if (status != JsonDecodingStatus::Succeeded) { return status; }
                SkipJsonWhitespace(cursor);
            }
            first = false;
            JsonEnvelopeRootMember member = JsonEnvelopeRootMember::Unknown;
            status = ParseJsonEnvelopeRootMember(cursor, member, diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            if (member == JsonEnvelopeRootMember::Unknown) {
                return JsonDecodingStatus::MalformedRepresentation;
            }
            bool* seen = member == JsonEnvelopeRootMember::Edp ? &seenEdp : &seenValue;
            if (*seen) { return JsonDecodingStatus::DuplicateField; }
            *seen = true;
            SkipJsonWhitespace(cursor);
            status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(':'), diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
            status = member == JsonEnvelopeRootMember::Edp
                ? DecodeJsonEnvelopeMetadata<TParserLimits, TValue>(cursor, 1U, diagnostic)
                : DecodeJsonValue<TPopulate, TStrictness, TParserLimits>(
                    cursor,
                    destination,
                    1U,
                    skipState,
                    diagnostic
                );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
        }
        status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>('}'), diagnostic);
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        if (!seenEdp || !seenValue) { return JsonDecodingStatus::MalformedRepresentation; }
        return JsonDecodingStatus::Succeeded;
    }

} // ESPressio::Serialisation::Detail
