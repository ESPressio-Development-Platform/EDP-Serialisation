#pragma once

#include <cstddef>
#include <cstdint>

#include <ESPressio_System.hpp>

#include "CborDecoding.hpp"
#include "CborEncoding.hpp"
#include "Profiles.hpp"

namespace ESPressio::Serialisation::Detail {

    /// Emits the explicit V1 CBOR Typed Envelope around one identified root value.
    ///
    /// @tparam TSink Shared codec-neutral output sink Type.
    /// @tparam TValue Identified serialisable root Type.
    /// @param sink Destination sink.
    /// @param value Source root value.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink, class TValue>
    CborEncodingStatus EncodeCborTypedEnvelope(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        static_assert(System::IdentifiedType<TValue>);

        auto status = EncodeCborHead(sink, 4U, 3U, diagnostic);
        if (status != CborEncodingStatus::Succeeded) { return status; }

        status = EncodeCborInteger(
            sink,
            static_cast<std::uint8_t>(TypedEnvelopeVersion),
            diagnostic
        );
        if (status != CborEncodingStatus::Succeeded) { return status; }

        status = EncodeCborHead(sink, 2U, 8U, diagnostic);
        if (status != CborEncodingStatus::Succeeded) { return status; }
        const auto& identifierBytes = System::TypeIdentifierOf<TValue>.Bytes();
        status = WriteCborBytes(
            sink,
            identifierBytes.data(),
            identifierBytes.size(),
            diagnostic
        );
        if (status != CborEncodingStatus::Succeeded) { return status; }

        return EncodeCborValue(sink, value, diagnostic);
    }

    /// Decodes the explicit V1 CBOR Typed Envelope and verifies root identity before body population.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Compile-time unknown-Field handling policy propagated into the body.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Identified serialisable destination root Type.
    /// @param cursor Immutable caller-input cursor.
    /// @param destination Destination root or validation seed.
    /// @param skipState Unknown-value skip accounting state propagated into the body.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    CborDecodingStatus DecodeCborTypedEnvelope(
        CborInputCursor& cursor,
        TValue* destination,
        CborSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        static_assert(System::IdentifiedType<TValue>);
        const auto start = cursor.Position();
        CborHead envelope{};
        auto status = ReadCborHead(cursor, envelope, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (envelope.MajorType != 4U || envelope.Argument != 3U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }
        status = EnterCborContainer<TParserLimits>(0U, start, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }

        const auto versionStart = cursor.Position();
        CborHead version{};
        status = ReadCborHead(cursor, version, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (version.MajorType != 0U) {
            diagnostic.ByteOffset = versionStart;
            return CborDecodingStatus::TypeMismatch;
        }
        if (version.Argument != TypedEnvelopeVersion) {
            diagnostic.ByteOffset = versionStart;
            return CborDecodingStatus::UnsupportedEnvelopeVersion;
        }

        const auto identifierStart = cursor.Position();
        CborHead identifierHead{};
        status = ReadCborHead(cursor, identifierHead, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (identifierHead.MajorType != 2U || identifierHead.Argument != 8U) {
            diagnostic.ByteOffset = identifierStart;
            return CborDecodingStatus::MalformedRepresentation;
        }
        if (!CborHasBytes(cursor, 8U)) {
            diagnostic.ByteOffset = identifierStart;
            return CborDecodingStatus::MalformedRepresentation;
        }
        const auto& expected = System::TypeIdentifierOf<TValue>.Bytes();
        bool matches = true;
        for (std::size_t index = 0U; index < expected.size(); ++index) {
            if (cursor.CurrentData()[index] != expected[index]) {
                matches = false;
                break;
            }
        }
        cursor.Advance(8U);
        if (!matches) {
            diagnostic.ByteOffset = identifierStart;
            diagnostic.Type = System::TypeIdentifierOf<TValue>;
            return CborDecodingStatus::TypeIdentifierMismatch;
        }

        return DecodeCborValue<TPopulate, TStrictness, TParserLimits>(
            cursor,
            destination,
            1U,
            skipState,
            diagnostic
        );
    }

} // ESPressio::Serialisation::Detail
