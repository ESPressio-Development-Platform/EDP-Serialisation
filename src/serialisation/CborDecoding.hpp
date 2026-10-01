#pragma once

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

#include "CborEncoding.hpp"
#include "JsonDecoding.hpp"
#include "ParserLimits.hpp"
#include "Profiles.hpp"
#include "Results.hpp"

namespace ESPressio::Serialisation::Detail {

    /// Internal outcomes produced by deterministic V1 CBOR decoding.
    enum class CborDecodingStatus : std::uint8_t {
        /// Complete CBOR validation/population succeeded.
        Succeeded = 0U,
        /// CBOR syntax, canonical head form, or required structural bytes are malformed.
        MalformedRepresentation = 1U,
        /// A caller-selected parser resource bound was exceeded.
        ResourceLimitExceeded = 2U,
        /// A schema map contains a numeric FieldIdentifier not present in the target schema.
        UnknownField = 3U,
        /// One schema map contains the same FieldIdentifier more than once.
        DuplicateField = 4U,
        /// A required non-Optional schema Field is absent.
        MissingRequiredField = 5U,
        /// The CBOR major/simple/float category does not match the target Type.
        TypeMismatch = 6U,
        /// An integer value lies outside the exact fixed-width target domain.
        NumericOutOfRange = 7U,
        /// A binary32/binary64 payload is NaN or infinite and therefore outside V1.
        NonFiniteNumber = 8U,
        /// A CBOR text string contains invalid V1 UTF-8.
        InvalidUtf8 = 9U,
        /// A bounded destination cannot retain the complete decoded value.
        CapacityExceeded = 10U,
        /// Reverse canonical strong-Type adaptation reported non-success.
        AdaptationFailed = 11U,
        /// Typed Envelope metadata carries an unsupported envelope version.
        UnsupportedEnvelopeVersion = 12U,
        /// Typed Envelope root identity differs from the compile-time target Type.
        TypeIdentifierMismatch = 13U
    };

    /// Immutable cursor over caller-owned contiguous CBOR input.
    class CborInputCursor final {
    private:

        // Caller-owned input state.

        /// First byte of the immutable caller-owned CBOR range.
        const std::uint8_t* _input = nullptr;

        /// Complete caller-owned input length in bytes.
        std::size_t _length = 0U;

        /// Current parser position within the caller-owned range.
        std::size_t _position = 0U;

    public:

        // Construction.

        /// Binds a cursor to one immutable caller-owned contiguous CBOR range.
        ///
        /// @param input First byte of the caller-owned range.
        /// @param length Complete range length in bytes.
        CborInputCursor(
            const std::uint8_t* input,
            std::size_t length
        ) noexcept :
            _input(input),
            _length(length) {
        }

        // Cursor inspection.

        /// Reports whether the cursor has consumed the complete input range.
        [[nodiscard]] constexpr bool IsAtEnd() const noexcept {
            return _position >= _length;
        }

        /// Returns the current byte offset from the beginning of the caller-owned range.
        [[nodiscard]] constexpr std::size_t Position() const noexcept {
            return _position;
        }

        /// Returns the complete caller-owned input length.
        [[nodiscard]] constexpr std::size_t Length() const noexcept {
            return _length;
        }

        /// Returns the number of unread bytes remaining from the current position.
        [[nodiscard]] constexpr std::size_t Remaining() const noexcept {
            return _position <= _length ? _length - _position : 0U;
        }

        /// Returns the first byte of the complete caller-owned input range.
        [[nodiscard]] constexpr const std::uint8_t* Data() const noexcept {
            return _input;
        }

        /// Returns the first unread byte at the current cursor position.
        [[nodiscard]] constexpr const std::uint8_t* CurrentData() const noexcept {
            return _input + _position;
        }

        /// Returns the current unread byte; callers must first prove the cursor is not at end.
        [[nodiscard]] constexpr std::uint8_t Current() const noexcept {
            return _input[_position];
        }

        // Cursor movement.

        /// Advances by one byte when input remains.
        void Advance() noexcept {
            if (_position < _length) { ++_position; }
        }

        /// Advances by a proven byte count; an excessive count clamps to end-of-input.
        ///
        /// @param count Number of bytes to advance.
        void Advance(
            std::size_t count
        ) noexcept {
            if (count <= Remaining()) { _position += count; }
            else { _position = _length; }
        }

    };

    /// Parsed canonical CBOR major-type header.
    struct CborHead final {

        // Decoded header facts.

        /// CBOR major type extracted from the initial byte.
        std::uint8_t MajorType = 0U;

        /// Canonically decoded unsigned additional-information argument.
        std::uint64_t Argument = 0U;

    };

    /// Unknown-value skip accounting state.
    struct CborSkipState final {

        // Bounded skip accounting.

        /// Container items consumed solely while structurally skipping unknown Field values.
        std::size_t ContainerItems = 0U;

    };

    /// Reports whether the caller cursor retains a complete byte range.
    [[nodiscard]] inline bool CborHasBytes(
        const CborInputCursor& cursor,
        std::size_t count
    ) noexcept {
        return count <= cursor.Remaining();
    }

    /// Reads one canonical big-endian unsigned argument from 1/2/4/8 bytes.
    inline std::uint64_t ReadCborBigEndian(
        const std::uint8_t* source,
        std::size_t length
    ) noexcept {
        std::uint64_t value = 0U;
        for (std::size_t index = 0U; index < length; ++index) {
            value = static_cast<std::uint64_t>(
                (value << 8U) | source[index]
            );
        }
        return value;
    }

    /// Parses one canonical CBOR major-type header for major types 0 through 6.
    inline CborDecodingStatus ReadCborHead(
        CborInputCursor& cursor,
        CborHead& head,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }

        const auto initial = cursor.Current();
        cursor.Advance();
        head.MajorType = static_cast<std::uint8_t>(initial >> 5U);
        const auto additional = static_cast<std::uint8_t>(initial & 0x1FU);
        if (head.MajorType == 7U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }

        if (additional <= 23U) {
            head.Argument = additional;
            return CborDecodingStatus::Succeeded;
        }

        std::size_t width = 0U;
        std::uint64_t canonicalMinimum = 0U;
        switch (additional) {
            case 24U:
                width = 1U;
                canonicalMinimum = 24U;
                break;
            case 25U:
                width = 2U;
                canonicalMinimum = 0x100U;
                break;
            case 26U:
                width = 4U;
                canonicalMinimum = 0x10000U;
                break;
            case 27U:
                width = 8U;
                canonicalMinimum = 0x100000000ULL;
                break;
            default:
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::MalformedRepresentation;
        }

        if (!CborHasBytes(cursor, width)) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }
        head.Argument = ReadCborBigEndian(cursor.CurrentData(), width);
        cursor.Advance(width);
        if (head.Argument < canonicalMinimum) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }
        return CborDecodingStatus::Succeeded;
    }

    /// Validates parser nesting before entering one CBOR array/map container.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @param depth Current syntactic container depth.
    /// @param offset Input offset reported on failure.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Succeeded or ResourceLimitExceeded.
    template<class TParserLimits>
    CborDecodingStatus EnterCborContainer(
        std::size_t depth,
        std::size_t offset,
        Diagnostic& diagnostic
    ) noexcept {
        if (depth >= TParserLimits::MaximumNestingDepth) {
            diagnostic.ByteOffset = offset;
            return CborDecodingStatus::ResourceLimitExceeded;
        }
        return CborDecodingStatus::Succeeded;
    }

    /// Accounts one container item traversed only to skip an unknown Field value.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @param state Unknown-value skip accounting state.
    /// @param offset Input offset reported on failure.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Succeeded or ResourceLimitExceeded.
    template<class TParserLimits>
    CborDecodingStatus AccountSkippedCborItem(
        CborSkipState& state,
        std::size_t offset,
        Diagnostic& diagnostic
    ) noexcept {
        if (state.ContainerItems >= TParserLimits::MaximumSkippedContainerItems) {
            diagnostic.ByteOffset = offset;
            return CborDecodingStatus::ResourceLimitExceeded;
        }
        ++state.ContainerItems;
        return CborDecodingStatus::Succeeded;
    }

    /// Reads one fixed-width CBOR floating representation and validates V1 finiteness.
    ///
    /// @tparam TValue `float` or `double` target Type determining required wire width.
    /// @param cursor Caller-input cursor advanced over the complete floating item.
    /// @param value Decoded floating value.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<class TValue>
    CborDecodingStatus ReadCborFloating(
        CborInputCursor& cursor,
        TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(IsSupportedFloatingPoint<Value>);
        const auto start = cursor.Position();
        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }

        if constexpr (std::is_same_v<Value, float>) {
            if (cursor.Current() != 0xFAU || cursor.Remaining() < 5U) {
                diagnostic.ByteOffset = start;
                return cursor.Current() == 0xFAU
                    ? CborDecodingStatus::MalformedRepresentation
                    : CborDecodingStatus::TypeMismatch;
            }
            cursor.Advance();
            const auto bits = static_cast<std::uint32_t>(
                ReadCborBigEndian(cursor.CurrentData(), 4U)
            );
            cursor.Advance(4U);
            value = std::bit_cast<float>(bits);
        } else {
            if (cursor.Current() != 0xFBU || cursor.Remaining() < 9U) {
                diagnostic.ByteOffset = start;
                return cursor.Current() == 0xFBU
                    ? CborDecodingStatus::MalformedRepresentation
                    : CborDecodingStatus::TypeMismatch;
            }
            cursor.Advance();
            const auto bits = ReadCborBigEndian(cursor.CurrentData(), 8U);
            cursor.Advance(8U);
            value = std::bit_cast<double>(bits);
        }

        if (!std::isfinite(value)) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::NonFiniteNumber;
        }
        return CborDecodingStatus::Succeeded;
    }

    /// Forward declaration for recursive CBOR value decoding.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Serialisable destination Type.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    CborDecodingStatus DecodeCborValue(
        CborInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        CborSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept;

    /// Structurally validates/skips one V1-supported CBOR value.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @param cursor Caller-input cursor.
    /// @param depth Current syntactic container depth.
    /// @param skipState Unknown-value skip accounting state.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<class TParserLimits>
    CborDecodingStatus SkipCborValue(
        CborInputCursor& cursor,
        std::size_t depth,
        CborSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }
        const auto major = static_cast<std::uint8_t>(cursor.Current() >> 5U);

        if (major <= 1U) {
            CborHead head{};
            return ReadCborHead(cursor, head, diagnostic);
        }

        if (major == 2U || major == 3U) {
            CborHead head{};
            auto status = ReadCborHead(cursor, head, diagnostic);
            if (status != CborDecodingStatus::Succeeded) { return status; }
            if (head.Argument > std::numeric_limits<std::size_t>::max()) {
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::ResourceLimitExceeded;
            }
            const auto length = static_cast<std::size_t>(head.Argument);
            if (!CborHasBytes(cursor, length)) {
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::MalformedRepresentation;
            }
            if (major == 3U) {
                const auto validation = ValidateUtf8(
                    reinterpret_cast<const char*>(cursor.CurrentData()),
                    length
                );
                if (!validation.IsSuccessful()) {
                    diagnostic.ByteOffset = cursor.Position() + validation.ByteOffset;
                    return CborDecodingStatus::InvalidUtf8;
                }
            }
            cursor.Advance(length);
            return CborDecodingStatus::Succeeded;
        }

        if (major == 4U) {
            CborHead head{};
            auto status = ReadCborHead(cursor, head, diagnostic);
            if (status != CborDecodingStatus::Succeeded) { return status; }
            status = EnterCborContainer<TParserLimits>(depth, start, diagnostic);
            if (status != CborDecodingStatus::Succeeded) { return status; }
            if (head.Argument > cursor.Remaining()) {
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::MalformedRepresentation;
            }
            for (std::uint64_t index = 0U; index < head.Argument; ++index) {
                status = AccountSkippedCborItem<TParserLimits>(
                    skipState,
                    cursor.Position(),
                    diagnostic
                );
                if (status != CborDecodingStatus::Succeeded) { return status; }
                status = SkipCborValue<TParserLimits>(
                    cursor,
                    depth + 1U,
                    skipState,
                    diagnostic
                );
                if (status != CborDecodingStatus::Succeeded) { return status; }
            }
            return CborDecodingStatus::Succeeded;
        }

        if (major == 5U) {
            CborHead head{};
            auto status = ReadCborHead(cursor, head, diagnostic);
            if (status != CborDecodingStatus::Succeeded) { return status; }
            status = EnterCborContainer<TParserLimits>(depth, start, diagnostic);
            if (status != CborDecodingStatus::Succeeded) { return status; }
            if (head.Argument > 256U) {
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::DuplicateField;
            }
            std::array<std::uint8_t, 32U> seen{};
            for (std::uint64_t index = 0U; index < head.Argument; ++index) {
                status = AccountSkippedCborItem<TParserLimits>(
                    skipState,
                    cursor.Position(),
                    diagnostic
                );
                if (status != CborDecodingStatus::Succeeded) { return status; }
                const auto keyStart = cursor.Position();
                CborHead key{};
                status = ReadCborHead(cursor, key, diagnostic);
                if (status != CborDecodingStatus::Succeeded) { return status; }
                if (key.MajorType != 0U || key.Argument > 255U) {
                    diagnostic.ByteOffset = keyStart;
                    return CborDecodingStatus::TypeMismatch;
                }
                const System::FieldIdentifier identifier{
                    static_cast<System::FieldIdentifier::Storage>(key.Argument)
                };
                if (IsFieldSeen(seen, identifier)) {
                    diagnostic.ByteOffset = keyStart;
                    diagnostic.Field = identifier;
                    return CborDecodingStatus::DuplicateField;
                }
                MarkFieldSeen(seen, identifier);
                status = SkipCborValue<TParserLimits>(
                    cursor,
                    depth + 1U,
                    skipState,
                    diagnostic
                );
                if (status != CborDecodingStatus::Succeeded) { return status; }
            }
            return CborDecodingStatus::Succeeded;
        }

        if (major == 6U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }

        const auto initial = cursor.Current();
        if (initial == 0xF4U || initial == 0xF5U || initial == 0xF6U) {
            cursor.Advance();
            return CborDecodingStatus::Succeeded;
        }
        if (initial == 0xFAU) {
            float value = 0.0F;
            return ReadCborFloating(cursor, value, diagnostic);
        }
        if (initial == 0xFBU) {
            double value = 0.0;
            return ReadCborFloating(cursor, value, diagnostic);
        }
        diagnostic.ByteOffset = start;
        return CborDecodingStatus::TypeMismatch;
    }

    /// Decodes one CBOR integer into an exact fixed-width target domain.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TValue Supported fixed-width integer target Type.
    /// @param cursor Caller-input cursor.
    /// @param destination Target storage or validation seed.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<bool TPopulate, class TValue>
    CborDecodingStatus DecodeCborInteger(
        CborInputCursor& cursor,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(IsFixedWidthInteger<Value>);
        const auto start = cursor.Position();
        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }
        const auto major = static_cast<std::uint8_t>(cursor.Current() >> 5U);
        if (major > 1U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }

        CborHead head{};
        auto status = ReadCborHead(cursor, head, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }

        Value decoded{};
        if (head.MajorType == 0U) {
            if (head.Argument > static_cast<std::uint64_t>(std::numeric_limits<Value>::max())) {
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::NumericOutOfRange;
            }
            decoded = static_cast<Value>(head.Argument);
        } else {
            if constexpr (!std::is_signed_v<Value>) {
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::NumericOutOfRange;
            } else {
                const auto minimum = static_cast<std::int64_t>(std::numeric_limits<Value>::min());
                const auto maximumArgument = static_cast<std::uint64_t>(-(minimum + 1));
                if (head.Argument > maximumArgument) {
                    diagnostic.ByteOffset = start;
                    return CborDecodingStatus::NumericOutOfRange;
                }
                decoded = static_cast<Value>(
                    -1 - static_cast<std::int64_t>(head.Argument)
                );
            }
        }

        if constexpr (TPopulate) { *destination = decoded; }
        return CborDecodingStatus::Succeeded;
    }

    /// Decodes one exact-width finite CBOR floating value.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TValue `float` or `double` target Type.
    /// @param cursor Caller-input cursor.
    /// @param destination Target storage or validation seed.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<bool TPopulate, class TValue>
    CborDecodingStatus DecodeCborFloating(
        CborInputCursor& cursor,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        TValue decoded{};
        const auto status = ReadCborFloating(cursor, decoded, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if constexpr (TPopulate) { *destination = decoded; }
        return CborDecodingStatus::Succeeded;
    }

    /// Decodes one bounded UTF-8 text string.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TValue Bounded String target Type.
    /// @param cursor Caller-input cursor.
    /// @param destination Target storage or validation seed.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<bool TPopulate, class TValue>
    CborDecodingStatus DecodeCborBoundedString(
        CborInputCursor& cursor,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        CborHead head{};
        auto status = ReadCborHead(cursor, head, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (head.MajorType != 3U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }
        if (head.Argument > BoundedStringTraits<TValue>::Capacity) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::CapacityExceeded;
        }
        const auto length = static_cast<std::size_t>(head.Argument);
        if (!CborHasBytes(cursor, length)) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }
        const auto validation = ValidateUtf8(
            reinterpret_cast<const char*>(cursor.CurrentData()),
            length
        );
        if (!validation.IsSuccessful()) {
            diagnostic.ByteOffset = cursor.Position() + validation.ByteOffset;
            return CborDecodingStatus::InvalidUtf8;
        }
        if constexpr (TPopulate) {
            const auto assign = destination->Assign(
                reinterpret_cast<const char*>(cursor.CurrentData()),
                length
            );
            if (assign != Bounded::StringAssignmentResult::Succeeded) {
                return CborDecodingStatus::CapacityExceeded;
            }
        }
        cursor.Advance(length);
        return CborDecodingStatus::Succeeded;
    }

    /// Decodes one bounded arbitrary byte string.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TValue Bounded Bytes target Type.
    /// @param cursor Caller-input cursor.
    /// @param destination Target storage or validation seed.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<bool TPopulate, class TValue>
    CborDecodingStatus DecodeCborBoundedBytes(
        CborInputCursor& cursor,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        CborHead head{};
        auto status = ReadCborHead(cursor, head, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (head.MajorType != 2U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }
        if (head.Argument > BoundedBytesTraits<TValue>::Capacity) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::CapacityExceeded;
        }
        const auto length = static_cast<std::size_t>(head.Argument);
        if (!CborHasBytes(cursor, length)) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }
        if constexpr (TPopulate) {
            const auto assign = destination->Assign(cursor.CurrentData(), length);
            if (assign != Bounded::BytesAssignmentResult::Succeeded) {
                return CborDecodingStatus::CapacityExceeded;
            }
        }
        cursor.Advance(length);
        return CborDecodingStatus::Succeeded;
    }

    /// Decodes one fixed-extent definite-length CBOR array.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TStrictness Compile-time unknown-Field policy propagated to elements.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TAccessor Indexed callable returning each destination element pointer.
    /// @param cursor Caller-input cursor.
    /// @param count Required compile-time-equivalent element count.
    /// @param accessor Indexed destination accessor.
    /// @param depth Current syntactic container depth.
    /// @param skipState Unknown-value skip accounting state.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TAccessor
    >
    CborDecodingStatus DecodeCborFixedArray(
        CborInputCursor& cursor,
        std::size_t count,
        TAccessor&& accessor,
        std::size_t depth,
        CborSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        CborHead head{};
        auto status = ReadCborHead(cursor, head, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (head.MajorType != 4U || head.Argument != count) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }
        status = EnterCborContainer<TParserLimits>(depth, start, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        for (std::size_t index = 0U; index < count; ++index) {
            status = DecodeCborValue<TPopulate, TStrictness, TParserLimits>(
                cursor,
                accessor(index),
                depth + 1U,
                skipState,
                diagnostic
            );
            if (status != CborDecodingStatus::Succeeded) { return status; }
        }
        return CborDecodingStatus::Succeeded;
    }

    /// Decodes one bounded variable-length definite CBOR array.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TStrictness Compile-time unknown-Field policy propagated to elements.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Bounded Vector target Type.
    /// @param cursor Caller-input cursor.
    /// @param destination Target vector or validation seed.
    /// @param depth Current syntactic container depth.
    /// @param skipState Unknown-value skip accounting state.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    CborDecodingStatus DecodeCborVector(
        CborInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        CborSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        CborHead head{};
        auto status = ReadCborHead(cursor, head, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (head.MajorType != 4U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }
        status = EnterCborContainer<TParserLimits>(depth, start, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (head.Argument > BoundedVectorTraits<TValue>::Capacity) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::CapacityExceeded;
        }
        if constexpr (TPopulate) { destination->Clear(); }
        using Element = typename BoundedVectorTraits<TValue>::Element;
        for (std::uint64_t index = 0U; index < head.Argument; ++index) {
            Element validationElement{};
            Element* element = &validationElement;
            if constexpr (TPopulate) {
                const auto emplace = destination->EmplaceBack();
                if (emplace != Bounded::VectorEmplaceBackResult::Succeeded) {
                    return CborDecodingStatus::CapacityExceeded;
                }
                element = &(*destination)[destination->Size() - 1U];
            }
            status = DecodeCborValue<TPopulate, TStrictness, TParserLimits>(
                cursor,
                element,
                depth + 1U,
                skipState,
                diagnostic
            );
            if (status != CborDecodingStatus::Succeeded) { return status; }
        }
        return CborDecodingStatus::Succeeded;
    }

    /// Dispatches one runtime CBOR FieldIdentifier to its compile-time schema binding.
    ///
    /// @tparam TIdentifier Current candidate one-byte FieldIdentifier.
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Serialisable schema target Type.
    /// @param cursor Caller-input cursor positioned at the Field value.
    /// @param identifier Runtime FieldIdentifier to dispatch.
    /// @param destination Target schema or validation seed.
    /// @param depth Current syntactic container depth.
    /// @param skipState Unknown-value skip accounting state.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @param knownField Set true when the schema owns the identifier.
    /// @return Complete internal CBOR decoding outcome.
    template<
        std::uint16_t TIdentifier,
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    CborDecodingStatus DecodeCborSchemaField(
        CborInputCursor& cursor,
        System::FieldIdentifier identifier,
        TValue* destination,
        std::size_t depth,
        CborSkipState& skipState,
        Diagnostic& diagnostic,
        bool& knownField
    ) noexcept {
        if constexpr (TIdentifier > 255U) {
            knownField = false;
            return CborDecodingStatus::Succeeded;
        } else {
            if (identifier.Value() != static_cast<System::FieldIdentifier::Storage>(TIdentifier)) {
                return DecodeCborSchemaField<
                    TIdentifier + 1U,
                    TPopulate,
                    TStrictness,
                    TParserLimits
                >(
                    cursor,
                    identifier,
                    destination,
                    depth,
                    skipState,
                    diagnostic,
                    knownField
                );
            }
            using Field = typename FieldBindingForIdentifier<
                System::FieldsOf<TValue>,
                static_cast<System::FieldIdentifier::Storage>(TIdentifier)
            >::Type;
            if constexpr (std::is_void_v<Field>) {
                knownField = false;
                return CborDecodingStatus::Succeeded;
            } else {
                knownField = true;
                using FieldValue = std::remove_cv_t<System::FieldValueOf<Field>>;
                if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                    if (!cursor.IsAtEnd() && cursor.Current() == 0xF6U) {
                        cursor.Advance();
                        if constexpr (TPopulate) { (destination->*Field::Member).reset(); }
                        return CborDecodingStatus::Succeeded;
                    }
                    using Element = typename OptionalValueTraits<FieldValue>::Element;
                    Element validationElement{};
                    Element* element = &validationElement;
                    if constexpr (TPopulate) {
                        auto& optional = destination->*Field::Member;
                        if (!optional.has_value()) { optional.emplace(); }
                        element = &optional.value();
                    } else if (destination != nullptr) {
                        auto& optional = destination->*Field::Member;
                        if (optional.has_value()) { element = &optional.value(); }
                    }
                    const auto fieldStatus = DecodeCborValue<
                        TPopulate,
                        TStrictness,
                        TParserLimits
                    >(
                        cursor,
                        element,
                        depth,
                        skipState,
                        diagnostic
                    );
                    if (fieldStatus != CborDecodingStatus::Succeeded) {
                        if (!diagnostic.Type.has_value()) { diagnostic.Type = TValue::Identifier; }
                        if (!diagnostic.Field.has_value()) { diagnostic.Field = Field::Identifier; }
                    }
                    return fieldStatus;
                } else {
                    FieldValue* field = destination == nullptr
                        ? nullptr
                        : &(destination->*Field::Member);
                    const auto fieldStatus = DecodeCborValue<
                        TPopulate,
                        TStrictness,
                        TParserLimits
                    >(
                        cursor,
                        field,
                        depth,
                        skipState,
                        diagnostic
                    );
                    if (fieldStatus != CborDecodingStatus::Succeeded) {
                        if (!diagnostic.Type.has_value()) { diagnostic.Type = TValue::Identifier; }
                        if (!diagnostic.Field.has_value()) { diagnostic.Field = Field::Identifier; }
                    }
                    return fieldStatus;
                }
            }
        }
    }

    /// Validates required Field presence and resets omitted Optional Fields during population.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TValue Serialisable schema target Type.
    /// @param seen Fixed one-bit-per-FieldIdentifier presence map.
    /// @param destination Target schema or validation seed.
    /// @param diagnostic Diagnostic payload populated on missing required Field.
    /// @return Complete internal CBOR decoding outcome.
    template<bool TPopulate, class TValue>
    CborDecodingStatus FinaliseCborSchemaPresence(
        const std::array<std::uint8_t, 32U>& seen,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        CborDecodingStatus status = CborDecodingStatus::Succeeded;
        System::ForEachField<TValue>([&]<class TField>() constexpr {
            if (status != CborDecodingStatus::Succeeded) { return; }
            using FieldValue = std::remove_cv_t<System::FieldValueOf<TField>>;
            const bool present = IsFieldSeen(seen, TField::Identifier);
            if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                if constexpr (TPopulate) {
                    if (!present) { (destination->*TField::Member).reset(); }
                }
            } else if (!present) {
                diagnostic.Type = TValue::Identifier;
                diagnostic.Field = TField::Identifier;
                status = CborDecodingStatus::MissingRequiredField;
            }
        });
        return status;
    }

    /// Decodes one numeric-key schema map in arbitrary input Field order.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Serialisable schema target Type.
    /// @param cursor Caller-input cursor.
    /// @param destination Target schema or validation seed.
    /// @param depth Current syntactic container depth.
    /// @param skipState Unknown-value skip accounting state.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    CborDecodingStatus DecodeCborSchema(
        CborInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        CborSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        CborHead head{};
        auto status = ReadCborHead(cursor, head, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (head.MajorType != 5U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::TypeMismatch;
        }
        status = EnterCborContainer<TParserLimits>(depth, start, diagnostic);
        if (status != CborDecodingStatus::Succeeded) { return status; }
        if (head.Argument > 256U) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::DuplicateField;
        }
        std::array<std::uint8_t, 32U> seen{};
        for (std::uint64_t index = 0U; index < head.Argument; ++index) {
            const auto keyStart = cursor.Position();
            CborHead key{};
            status = ReadCborHead(cursor, key, diagnostic);
            if (status != CborDecodingStatus::Succeeded) { return status; }
            if (key.MajorType != 0U || key.Argument > 255U) {
                diagnostic.ByteOffset = keyStart;
                return CborDecodingStatus::TypeMismatch;
            }
            const System::FieldIdentifier identifier{
                static_cast<System::FieldIdentifier::Storage>(key.Argument)
            };
            if (IsFieldSeen(seen, identifier)) {
                diagnostic.ByteOffset = keyStart;
                diagnostic.Type = TValue::Identifier;
                diagnostic.Field = identifier;
                return CborDecodingStatus::DuplicateField;
            }
            MarkFieldSeen(seen, identifier);

            bool knownField = false;
            status = DecodeCborSchemaField<
                0U,
                TPopulate,
                TStrictness,
                TParserLimits
            >(
                cursor,
                identifier,
                destination,
                depth + 1U,
                skipState,
                diagnostic,
                knownField
            );
            if (status != CborDecodingStatus::Succeeded) { return status; }
            if (!knownField) {
                diagnostic.Type = TValue::Identifier;
                diagnostic.Field = identifier;
                if constexpr (TStrictness == StrictnessPolicy::Exact) {
                    diagnostic.ByteOffset = keyStart;
                    return CborDecodingStatus::UnknownField;
                } else {
                    status = SkipCborValue<TParserLimits>(
                        cursor,
                        depth + 1U,
                        skipState,
                        diagnostic
                    );
                    if (status != CborDecodingStatus::Succeeded) { return status; }
                }
            }
        }
        return FinaliseCborSchemaPresence<TPopulate>(seen, destination, diagnostic);
    }

    /// Decodes one value from the complete V1 CBOR Type universe.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Serialisable destination Type.
    /// @param cursor Caller-input cursor.
    /// @param destination Target value or validation seed.
    /// @param depth Current syntactic container depth.
    /// @param skipState Unknown-value skip accounting state.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR decoding outcome.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    CborDecodingStatus DecodeCborValue(
        CborInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        CborSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(IsSerialisableType<Value>, "CBOR decoding requires a SerialisableType target value");
        const auto start = cursor.Position();
        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = start;
            return CborDecodingStatus::MalformedRepresentation;
        }

        if constexpr (std::is_same_v<Value, bool>) {
            const auto initial = cursor.Current();
            if (initial != 0xF4U && initial != 0xF5U) {
                diagnostic.ByteOffset = start;
                return CborDecodingStatus::TypeMismatch;
            }
            cursor.Advance();
            if constexpr (TPopulate) { *destination = initial == 0xF5U; }
            return CborDecodingStatus::Succeeded;
        } else if constexpr (IsFixedWidthInteger<Value>) {
            return DecodeCborInteger<TPopulate>(cursor, destination, diagnostic);
        } else if constexpr (IsSupportedFloatingPoint<Value>) {
            return DecodeCborFloating<TPopulate>(cursor, destination, diagnostic);
        } else if constexpr (std::is_enum_v<Value>) {
            using Underlying = typename EnumSerialisationTraits<Value>::UnderlyingType;
            Underlying underlying{};
            const auto status = DecodeCborInteger<true>(cursor, &underlying, diagnostic);
            if (status != CborDecodingStatus::Succeeded) { return status; }
            if constexpr (TPopulate) { *destination = static_cast<Value>(underlying); }
            return CborDecodingStatus::Succeeded;
        } else if constexpr (OptionalValueTraits<Value>::IsValue) {
            if (cursor.Current() == 0xF6U) {
                cursor.Advance();
                if constexpr (TPopulate) { destination->reset(); }
                return CborDecodingStatus::Succeeded;
            }
            using Element = typename OptionalValueTraits<Value>::Element;
            Element validationElement{};
            Element* element = &validationElement;
            if constexpr (TPopulate) {
                if (!destination->has_value()) { destination->emplace(); }
                element = &destination->value();
            } else if (destination != nullptr && destination->has_value()) {
                element = &destination->value();
            }
            return DecodeCborValue<TPopulate, TStrictness, TParserLimits>(
                cursor,
                element,
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (StandardArrayTraits<Value>::IsValue) {
            return DecodeCborFixedArray<TPopulate, TStrictness, TParserLimits>(
                cursor,
                StandardArrayTraits<Value>::Count,
                [&](std::size_t index) noexcept -> typename StandardArrayTraits<Value>::Element* {
                    return destination == nullptr ? nullptr : &(*destination)[index];
                },
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (std::is_array_v<Value>) {
            using Element = std::remove_extent_t<Value>;
            return DecodeCborFixedArray<TPopulate, TStrictness, TParserLimits>(
                cursor,
                std::extent_v<Value>,
                [&](std::size_t index) noexcept -> Element* {
                    return destination == nullptr ? nullptr : &((*destination)[index]);
                },
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (BoundedStringTraits<Value>::IsValue) {
            return DecodeCborBoundedString<TPopulate>(cursor, destination, diagnostic);
        } else if constexpr (BoundedBytesTraits<Value>::IsValue) {
            return DecodeCborBoundedBytes<TPopulate>(cursor, destination, diagnostic);
        } else if constexpr (BoundedVectorTraits<Value>::IsValue) {
            return DecodeCborVector<TPopulate, TStrictness, TParserLimits>(
                cursor,
                destination,
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (PotentialSchemaType<Value>) {
            static_assert(System::SchemaType<Value>);
            return DecodeCborSchema<TPopulate, TStrictness, TParserLimits>(
                cursor,
                destination,
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (HasCanonicalRepresentation<Value>) {
            using Representation = std::remove_cv_t<typename CanonicalRepresentation<Value>::Type>;
            Representation representation{};
            auto status = DecodeCborValue<true, TStrictness, TParserLimits>(
                cursor,
                &representation,
                depth,
                skipState,
                diagnostic
            );
            if (status != CborDecodingStatus::Succeeded) { return status; }
            using Adapter = Bounded::TypeConversionAdapter<Representation, Value>;
            if constexpr (TPopulate) {
                const auto result = Adapter::Convert(representation, *destination);
                if (!Bounded::IsTypeConversionSuccessful<Representation, Value>(result)) {
                    return CborDecodingStatus::AdaptationFailed;
                }
            } else if constexpr (std::is_nothrow_default_constructible_v<Value>) {
                Value validationTarget{};
                const auto result = Adapter::Convert(representation, validationTarget);
                if (!Bounded::IsTypeConversionSuccessful<Representation, Value>(result)) {
                    return CborDecodingStatus::AdaptationFailed;
                }
            } else {
                if (destination == nullptr) { return CborDecodingStatus::AdaptationFailed; }
                Value validationTarget{*destination};
                const auto result = Adapter::Convert(representation, validationTarget);
                if (!Bounded::IsTypeConversionSuccessful<Representation, Value>(result)) {
                    return CborDecodingStatus::AdaptationFailed;
                }
            }
            return CborDecodingStatus::Succeeded;
        } else {
            static_assert(IsSerialisableType<Value>);
            return CborDecodingStatus::TypeMismatch;
        }
    }

} // ESPressio::Serialisation::Detail
