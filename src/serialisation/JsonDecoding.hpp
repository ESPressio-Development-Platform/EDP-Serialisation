#pragma once

#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <system_error>
#include <type_traits>

#include <ESPressio_BoundedTypes.hpp>
#include <ESPressio_System.hpp>

#include "JsonEncoding.hpp"
#include "ParserLimits.hpp"
#include "Profiles.hpp"
#include "Results.hpp"
#include "SerialisableType.hpp"

namespace ESPressio::Serialisation::Detail {

    /// Internal outcomes produced by allocation-free JSON validation and population.
    enum class JsonDecodingStatus : std::uint8_t {
        /// Complete validation or population succeeded.
        Succeeded = 0U,
        /// Input does not form structurally valid JSON for the selected profile.
        MalformedRepresentation = 1U,
        /// Compile-time parser resource limits were exceeded.
        ResourceLimitExceeded = 2U,
        /// Exact strictness encountered a schema Field unknown to the target Type.
        UnknownField = 3U,
        /// The same logical schema Field appeared more than once.
        DuplicateField = 4U,
        /// A required non-Optional schema Field was absent.
        MissingRequiredField = 5U,
        /// JSON value category or exact mathematical shape does not match the target Type.
        TypeMismatch = 6U,
        /// Numeric input lies outside the target fixed-width numeric domain.
        NumericOutOfRange = 7U,
        /// A non-zero floating token underflowed completely to zero.
        NumericUnderflow = 8U,
        /// A floating result is not finite.
        NonFiniteNumber = 9U,
        /// Text is malformed UTF-8, contains an invalid surrogate escape, or contains forbidden U+0000.
        InvalidUtf8 = 10U,
        /// Byte text is not canonical padded RFC 4648 Base64.
        InvalidBase64 = 11U,
        /// Decoded bounded content exceeds the target Type capacity.
        CapacityExceeded = 12U,
        /// Canonical reverse adaptation rejected the decoded surrogate.
        AdaptationFailed = 13U,
        /// Typed-envelope metadata uses a version not understood by V1.
        UnsupportedEnvelopeVersion = 14U,
        /// Typed-envelope semantic identity does not equal the compile-time target Type.
        TypeIdentifierMismatch = 15U
    };

    /// Lightweight immutable cursor over caller-owned JSON input.
    class JsonInputCursor final {
    private:

        // Caller-owned input.

        /// First byte of the immutable input range.
        const std::uint8_t* _input = nullptr;

        /// Total number of bytes in the immutable input range.
        std::size_t _length = 0U;

        /// Current parser position relative to the first input byte.
        std::size_t _position = 0U;

    public:

        /// Binds the cursor to one caller-owned immutable JSON byte range.
        JsonInputCursor(
            const std::uint8_t* input,
            std::size_t length
        ) noexcept :
            _input(input),
            _length(length) {
        }

        /// Reports whether the cursor has consumed the complete range.
        [[nodiscard]] constexpr bool IsAtEnd() const noexcept {
            return _position >= _length;
        }

        /// Returns the current parser byte offset.
        [[nodiscard]] constexpr std::size_t Position() const noexcept {
            return _position;
        }

        /// Returns the total caller-owned input length.
        [[nodiscard]] constexpr std::size_t Length() const noexcept {
            return _length;
        }

        /// Returns the immutable first byte of the represented range.
        [[nodiscard]] constexpr const std::uint8_t* Data() const noexcept {
            return _input;
        }

        /// Returns the current byte without advancing; callers must first establish !IsAtEnd().
        [[nodiscard]] constexpr std::uint8_t Current() const noexcept {
            return _input[_position];
        }

        /// Advances by one byte when input remains.
        void Advance() noexcept {
            if (_position < _length) { ++_position; }
        }

        /// Advances by an already validated byte count.
        void Advance(
            std::size_t count
        ) noexcept {
            _position += count;
        }

        /// Resets the cursor to one previously validated byte offset.
        void Seek(
            std::size_t position
        ) noexcept {
            _position = position;
        }

    };

    /// Captures one syntactically valid JSON number token without converting its mathematical value.
    struct JsonNumberToken final {

        // Token bounds.

        /// First byte offset of the numeric token.
        std::size_t Start = 0U;

        /// One-past-final byte offset of the numeric token.
        std::size_t End = 0U;

        // Mantissa layout.

        /// First integer digit offset.
        std::size_t IntegerStart = 0U;

        /// One-past-final integer digit offset.
        std::size_t IntegerEnd = 0U;

        /// First fraction digit offset, or IntegerEnd when no fraction exists.
        std::size_t FractionStart = 0U;

        /// One-past-final fraction digit offset, or FractionStart when no fraction exists.
        std::size_t FractionEnd = 0U;

        // Sign and exponent metadata.

        /// Indicates that the JSON token begins with a minus sign.
        bool IsNegative = false;

        /// Indicates that an exponent marker is present.
        bool HasExponent = false;

        /// Indicates that the explicit exponent is negative.
        bool IsExponentNegative = false;

        /// Absolute exponent magnitude, saturated on textual overflow.
        std::uint64_t ExponentMagnitude = 0U;

        /// Indicates that the textual exponent magnitude exceeded uint64_t.
        bool IsExponentMagnitudeOverflow = false;

        /// Returns the number of integer mantissa digits.
        [[nodiscard]] constexpr std::size_t IntegerDigits() const noexcept {
            return IntegerEnd - IntegerStart;
        }

        /// Returns the number of fractional mantissa digits.
        [[nodiscard]] constexpr std::size_t FractionDigits() const noexcept {
            return FractionEnd - FractionStart;
        }

        /// Returns the total mantissa digit count, excluding punctuation/sign/exponent.
        [[nodiscard]] constexpr std::size_t MantissaDigits() const noexcept {
            return IntegerDigits() + FractionDigits();
        }

    };

    /// Mutable bounded state shared only while structurally skipping unknown JSON values.
    struct JsonSkipState final {

        /// Total array/object items traversed solely for unknown-value skipping.
        std::size_t ContainerItems = 0U;

    };

    /// Reports whether one byte is JSON whitespace.
    [[nodiscard]] constexpr bool IsJsonWhitespace(
        std::uint8_t value
    ) noexcept {
        return
            value == static_cast<std::uint8_t>(' ') ||
            value == static_cast<std::uint8_t>('\t') ||
            value == static_cast<std::uint8_t>('\n') ||
            value == static_cast<std::uint8_t>('\r');
    }

    /// Advances over every contiguous JSON whitespace byte.
    inline void SkipJsonWhitespace(
        JsonInputCursor& cursor
    ) noexcept {
        while (!cursor.IsAtEnd() && IsJsonWhitespace(cursor.Current())) {
            cursor.Advance();
        }
    }

    /// Consumes one exact byte after optional outer whitespace handling.
    inline JsonDecodingStatus ConsumeJsonByte(
        JsonInputCursor& cursor,
        std::uint8_t expected,
        Diagnostic& diagnostic
    ) noexcept {
        if (cursor.IsAtEnd() || cursor.Current() != expected) {
            diagnostic.ByteOffset = cursor.Position();
            return JsonDecodingStatus::MalformedRepresentation;
        }

        cursor.Advance();
        return JsonDecodingStatus::Succeeded;
    }

    /// Consumes one exact ASCII literal without accepting prefixes or alternate case.
    inline JsonDecodingStatus ConsumeJsonLiteral(
        JsonInputCursor& cursor,
        const char* literal,
        std::size_t length,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();

        for (std::size_t index = 0U; index < length; ++index) {
            if (
                cursor.IsAtEnd() ||
                cursor.Current() != static_cast<std::uint8_t>(literal[index])
            ) {
                diagnostic.ByteOffset = start;
                return JsonDecodingStatus::MalformedRepresentation;
            }
            cursor.Advance();
        }

        return JsonDecodingStatus::Succeeded;
    }

    /// Converts one hexadecimal digit to its numeric nibble value.
    [[nodiscard]] constexpr bool TryHexValue(
        std::uint8_t value,
        std::uint8_t& result
    ) noexcept {
        if (value >= static_cast<std::uint8_t>('0') && value <= static_cast<std::uint8_t>('9')) {
            result = static_cast<std::uint8_t>(value - static_cast<std::uint8_t>('0'));
            return true;
        }
        if (value >= static_cast<std::uint8_t>('a') && value <= static_cast<std::uint8_t>('f')) {
            result = static_cast<std::uint8_t>(10U + value - static_cast<std::uint8_t>('a'));
            return true;
        }
        if (value >= static_cast<std::uint8_t>('A') && value <= static_cast<std::uint8_t>('F')) {
            result = static_cast<std::uint8_t>(10U + value - static_cast<std::uint8_t>('A'));
            return true;
        }

        result = 0U;
        return false;
    }

    /// Parses exactly four hexadecimal digits used by one JSON Unicode escape.
    inline JsonDecodingStatus ParseJsonHexQuad(
        JsonInputCursor& cursor,
        std::uint16_t& value,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        std::uint16_t result = 0U;

        for (std::size_t index = 0U; index < 4U; ++index) {
            if (cursor.IsAtEnd()) {
                diagnostic.ByteOffset = start;
                return JsonDecodingStatus::MalformedRepresentation;
            }

            std::uint8_t nibble = 0U;
            if (!TryHexValue(cursor.Current(), nibble)) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::MalformedRepresentation;
            }

            result = static_cast<std::uint16_t>((result << 4U) | nibble);
            cursor.Advance();
        }

        value = result;
        return JsonDecodingStatus::Succeeded;
    }

    /// Emits one Unicode scalar value as canonical UTF-8 bytes to a consumer callable.
    ///
    /// @tparam TConsumer Callable receiving each decoded UTF-8 byte.
    template<class TConsumer>
    JsonDecodingStatus EmitUtf8Scalar(
        std::uint32_t scalar,
        bool rejectNull,
        TConsumer&& consumer,
        Diagnostic& diagnostic,
        std::size_t sourceOffset
    ) noexcept {
        if (scalar == 0U && rejectNull) {
            diagnostic.ByteOffset = sourceOffset;
            return JsonDecodingStatus::InvalidUtf8;
        }
        if (scalar > 0x10FFFFU || (scalar >= 0xD800U && scalar <= 0xDFFFU)) {
            diagnostic.ByteOffset = sourceOffset;
            return JsonDecodingStatus::InvalidUtf8;
        }

        std::array<std::uint8_t, 4U> bytes{};
        std::size_t count = 0U;

        if (scalar <= 0x7FU) {
            bytes[0U] = static_cast<std::uint8_t>(scalar);
            count = 1U;
        } else if (scalar <= 0x7FFU) {
            bytes[0U] = static_cast<std::uint8_t>(0xC0U | (scalar >> 6U));
            bytes[1U] = static_cast<std::uint8_t>(0x80U | (scalar & 0x3FU));
            count = 2U;
        } else if (scalar <= 0xFFFFU) {
            bytes[0U] = static_cast<std::uint8_t>(0xE0U | (scalar >> 12U));
            bytes[1U] = static_cast<std::uint8_t>(0x80U | ((scalar >> 6U) & 0x3FU));
            bytes[2U] = static_cast<std::uint8_t>(0x80U | (scalar & 0x3FU));
            count = 3U;
        } else {
            bytes[0U] = static_cast<std::uint8_t>(0xF0U | (scalar >> 18U));
            bytes[1U] = static_cast<std::uint8_t>(0x80U | ((scalar >> 12U) & 0x3FU));
            bytes[2U] = static_cast<std::uint8_t>(0x80U | ((scalar >> 6U) & 0x3FU));
            bytes[3U] = static_cast<std::uint8_t>(0x80U | (scalar & 0x3FU));
            count = 4U;
        }

        for (std::size_t index = 0U; index < count; ++index) {
            const auto status = consumer(bytes[index]);
            if (status != JsonDecodingStatus::Succeeded) {
                if (diagnostic.ByteOffset == 0U) { diagnostic.ByteOffset = sourceOffset; }
                return status;
            }
        }

        return JsonDecodingStatus::Succeeded;
    }

    /// Parses one JSON String and streams its decoded UTF-8 bytes to a consumer.
    ///
    /// @tparam TConsumer Callable receiving each decoded UTF-8 byte.
    template<class TConsumer>
    JsonDecodingStatus ParseJsonString(
        JsonInputCursor& cursor,
        bool rejectNull,
        TConsumer&& consumer,
        Diagnostic& diagnostic
    ) noexcept {
        const auto stringStart = cursor.Position();
        auto status = ConsumeJsonByte(
            cursor,
            static_cast<std::uint8_t>('"'),
            diagnostic
        );
        if (status != JsonDecodingStatus::Succeeded) { return JsonDecodingStatus::TypeMismatch; }

        while (!cursor.IsAtEnd()) {
            const auto sourceOffset = cursor.Position();
            const auto byte = cursor.Current();

            if (byte == static_cast<std::uint8_t>('"')) {
                cursor.Advance();
                return JsonDecodingStatus::Succeeded;
            }

            if (byte < 0x20U) {
                diagnostic.ByteOffset = sourceOffset;
                return JsonDecodingStatus::MalformedRepresentation;
            }

            if (byte == static_cast<std::uint8_t>('\\')) {
                cursor.Advance();
                if (cursor.IsAtEnd()) {
                    diagnostic.ByteOffset = sourceOffset;
                    return JsonDecodingStatus::MalformedRepresentation;
                }

                const auto escape = cursor.Current();
                cursor.Advance();

                switch (escape) {
                    case static_cast<std::uint8_t>('"'):
                    case static_cast<std::uint8_t>('\\'):
                    case static_cast<std::uint8_t>('/'):
                        status = consumer(escape);
                        break;
                    case static_cast<std::uint8_t>('b'):
                        status = consumer(0x08U);
                        break;
                    case static_cast<std::uint8_t>('f'):
                        status = consumer(0x0CU);
                        break;
                    case static_cast<std::uint8_t>('n'):
                        status = consumer(0x0AU);
                        break;
                    case static_cast<std::uint8_t>('r'):
                        status = consumer(0x0DU);
                        break;
                    case static_cast<std::uint8_t>('t'):
                        status = consumer(0x09U);
                        break;
                    case static_cast<std::uint8_t>('u'): {
                        std::uint16_t firstUnit = 0U;
                        status = ParseJsonHexQuad(
                            cursor,
                            firstUnit,
                            diagnostic
                        );
                        if (status != JsonDecodingStatus::Succeeded) { return status; }

                        std::uint32_t scalar = firstUnit;
                        if (firstUnit >= 0xD800U && firstUnit <= 0xDBFFU) {
                            if (
                                cursor.IsAtEnd() ||
                                cursor.Current() != static_cast<std::uint8_t>('\\')
                            ) {
                                diagnostic.ByteOffset = sourceOffset;
                                return JsonDecodingStatus::InvalidUtf8;
                            }
                            cursor.Advance();
                            if (
                                cursor.IsAtEnd() ||
                                cursor.Current() != static_cast<std::uint8_t>('u')
                            ) {
                                diagnostic.ByteOffset = sourceOffset;
                                return JsonDecodingStatus::InvalidUtf8;
                            }
                            cursor.Advance();

                            std::uint16_t secondUnit = 0U;
                            status = ParseJsonHexQuad(
                                cursor,
                                secondUnit,
                                diagnostic
                            );
                            if (status != JsonDecodingStatus::Succeeded) { return status; }
                            if (secondUnit < 0xDC00U || secondUnit > 0xDFFFU) {
                                diagnostic.ByteOffset = sourceOffset;
                                return JsonDecodingStatus::InvalidUtf8;
                            }

                            scalar = 0x10000U +
                                ((static_cast<std::uint32_t>(firstUnit) - 0xD800U) << 10U) +
                                (static_cast<std::uint32_t>(secondUnit) - 0xDC00U);
                        } else if (firstUnit >= 0xDC00U && firstUnit <= 0xDFFFU) {
                            diagnostic.ByteOffset = sourceOffset;
                            return JsonDecodingStatus::InvalidUtf8;
                        }

                        status = EmitUtf8Scalar(
                            scalar,
                            rejectNull,
                            consumer,
                            diagnostic,
                            sourceOffset
                        );
                        break;
                    }
                    default:
                        diagnostic.ByteOffset = sourceOffset;
                        return JsonDecodingStatus::MalformedRepresentation;
                }

                if (status != JsonDecodingStatus::Succeeded) {
                    if (diagnostic.ByteOffset == 0U) { diagnostic.ByteOffset = sourceOffset; }
                    return status;
                }
                continue;
            }

            if (byte <= 0x7FU) {
                if (byte == 0U && rejectNull) {
                    diagnostic.ByteOffset = sourceOffset;
                    return JsonDecodingStatus::InvalidUtf8;
                }
                status = consumer(byte);
                if (status != JsonDecodingStatus::Succeeded) {
                    if (diagnostic.ByteOffset == 0U) { diagnostic.ByteOffset = sourceOffset; }
                    return status;
                }
                cursor.Advance();
                continue;
            }

            std::size_t sequenceLength = 0U;
            if (byte >= 0xC2U && byte <= 0xDFU) sequenceLength = 2U;
            else if (byte >= 0xE0U && byte <= 0xEFU) sequenceLength = 3U;
            else if (byte >= 0xF0U && byte <= 0xF4U) sequenceLength = 4U;
            else {
                diagnostic.ByteOffset = sourceOffset;
                return JsonDecodingStatus::InvalidUtf8;
            }

            if (sequenceLength > cursor.Length() - cursor.Position()) {
                diagnostic.ByteOffset = sourceOffset;
                return JsonDecodingStatus::InvalidUtf8;
            }

            const auto validation = ValidateUtf8(
                reinterpret_cast<const char*>(cursor.Data() + cursor.Position()),
                sequenceLength
            );
            if (!validation.IsSuccessful()) {
                diagnostic.ByteOffset = sourceOffset + validation.ByteOffset;
                return JsonDecodingStatus::InvalidUtf8;
            }

            for (std::size_t index = 0U; index < sequenceLength; ++index) {
                status = consumer(cursor.Data()[cursor.Position() + index]);
                if (status != JsonDecodingStatus::Succeeded) {
                    if (diagnostic.ByteOffset == 0U) { diagnostic.ByteOffset = sourceOffset; }
                    return status;
                }
            }
            cursor.Advance(sequenceLength);
        }

        diagnostic.ByteOffset = stringStart;
        return JsonDecodingStatus::MalformedRepresentation;
    }

    /// Incremental decoded-byte reader used only for exact JSON object-key comparison.
    struct JsonDecodedStringReader final {

        // Source traversal state.

        /// Cursor replaying the caller-owned JSON input.
        JsonInputCursor Cursor;

        /// Indicates whether the opening quote has been consumed.
        bool Started = false;

        /// Indicates whether the closing quote has been consumed.
        bool Finished = false;

        // Pending UTF-8 expansion state.

        /// Canonical UTF-8 bytes produced by one escaped/raw scalar.
        std::array<std::uint8_t, 4U> Pending{};

        /// Number of valid bytes currently retained in Pending.
        std::size_t PendingCount = 0U;

        /// Next Pending byte to return.
        std::size_t PendingIndex = 0U;

        /// Creates one reader positioned at the opening quote of a JSON String.
        JsonDecodedStringReader(
            const std::uint8_t* input,
            std::size_t length,
            std::size_t start
        ) noexcept :
            Cursor(input, length) {
            Cursor.Seek(start);
        }

    };

    /// Emits one scalar into one decoded-string reader's fixed pending buffer.
    inline JsonDecodingStatus QueueDecodedStringScalar(
        JsonDecodedStringReader& reader,
        std::uint32_t scalar,
        Diagnostic& diagnostic,
        std::size_t sourceOffset
    ) noexcept {
        reader.PendingCount = 0U;
        reader.PendingIndex = 0U;

        if (scalar > 0x10FFFFU || (scalar >= 0xD800U && scalar <= 0xDFFFU)) {
            diagnostic.ByteOffset = sourceOffset;
            return JsonDecodingStatus::InvalidUtf8;
        }

        if (scalar <= 0x7FU) {
            reader.Pending[0U] = static_cast<std::uint8_t>(scalar);
            reader.PendingCount = 1U;
        } else if (scalar <= 0x7FFU) {
            reader.Pending[0U] = static_cast<std::uint8_t>(0xC0U | (scalar >> 6U));
            reader.Pending[1U] = static_cast<std::uint8_t>(0x80U | (scalar & 0x3FU));
            reader.PendingCount = 2U;
        } else if (scalar <= 0xFFFFU) {
            reader.Pending[0U] = static_cast<std::uint8_t>(0xE0U | (scalar >> 12U));
            reader.Pending[1U] = static_cast<std::uint8_t>(0x80U | ((scalar >> 6U) & 0x3FU));
            reader.Pending[2U] = static_cast<std::uint8_t>(0x80U | (scalar & 0x3FU));
            reader.PendingCount = 3U;
        } else {
            reader.Pending[0U] = static_cast<std::uint8_t>(0xF0U | (scalar >> 18U));
            reader.Pending[1U] = static_cast<std::uint8_t>(0x80U | ((scalar >> 12U) & 0x3FU));
            reader.Pending[2U] = static_cast<std::uint8_t>(0x80U | ((scalar >> 6U) & 0x3FU));
            reader.Pending[3U] = static_cast<std::uint8_t>(0x80U | (scalar & 0x3FU));
            reader.PendingCount = 4U;
        }

        return JsonDecodingStatus::Succeeded;
    }

    /// Returns the next decoded UTF-8 byte from one JSON String key without allocating storage.
    inline JsonDecodingStatus NextDecodedJsonStringByte(
        JsonDecodedStringReader& reader,
        std::uint8_t& value,
        bool& hasValue,
        Diagnostic& diagnostic
    ) noexcept {
        hasValue = false;

        if (reader.PendingIndex < reader.PendingCount) {
            value = reader.Pending[reader.PendingIndex++];
            hasValue = true;
            return JsonDecodingStatus::Succeeded;
        }

        reader.PendingCount = 0U;
        reader.PendingIndex = 0U;

        if (reader.Finished) { return JsonDecodingStatus::Succeeded; }

        if (!reader.Started) {
            if (
                reader.Cursor.IsAtEnd() ||
                reader.Cursor.Current() != static_cast<std::uint8_t>('"')
            ) {
                diagnostic.ByteOffset = reader.Cursor.Position();
                return JsonDecodingStatus::MalformedRepresentation;
            }
            reader.Cursor.Advance();
            reader.Started = true;
        }

        if (reader.Cursor.IsAtEnd()) {
            diagnostic.ByteOffset = reader.Cursor.Position();
            return JsonDecodingStatus::MalformedRepresentation;
        }

        const auto sourceOffset = reader.Cursor.Position();
        const auto byte = reader.Cursor.Current();
        if (byte == static_cast<std::uint8_t>('"')) {
            reader.Cursor.Advance();
            reader.Finished = true;
            return JsonDecodingStatus::Succeeded;
        }
        if (byte < 0x20U) {
            diagnostic.ByteOffset = sourceOffset;
            return JsonDecodingStatus::MalformedRepresentation;
        }

        if (byte == static_cast<std::uint8_t>('\\')) {
            reader.Cursor.Advance();
            if (reader.Cursor.IsAtEnd()) {
                diagnostic.ByteOffset = sourceOffset;
                return JsonDecodingStatus::MalformedRepresentation;
            }

            const auto escape = reader.Cursor.Current();
            reader.Cursor.Advance();
            switch (escape) {
                case static_cast<std::uint8_t>('"'):
                case static_cast<std::uint8_t>('\\'):
                case static_cast<std::uint8_t>('/'):
                    reader.Pending[0U] = escape;
                    reader.PendingCount = 1U;
                    break;
                case static_cast<std::uint8_t>('b'):
                    reader.Pending[0U] = 0x08U;
                    reader.PendingCount = 1U;
                    break;
                case static_cast<std::uint8_t>('f'):
                    reader.Pending[0U] = 0x0CU;
                    reader.PendingCount = 1U;
                    break;
                case static_cast<std::uint8_t>('n'):
                    reader.Pending[0U] = 0x0AU;
                    reader.PendingCount = 1U;
                    break;
                case static_cast<std::uint8_t>('r'):
                    reader.Pending[0U] = 0x0DU;
                    reader.PendingCount = 1U;
                    break;
                case static_cast<std::uint8_t>('t'):
                    reader.Pending[0U] = 0x09U;
                    reader.PendingCount = 1U;
                    break;
                case static_cast<std::uint8_t>('u'): {
                    std::uint16_t firstUnit = 0U;
                    auto status = ParseJsonHexQuad(
                        reader.Cursor,
                        firstUnit,
                        diagnostic
                    );
                    if (status != JsonDecodingStatus::Succeeded) { return status; }

                    std::uint32_t scalar = firstUnit;
                    if (firstUnit >= 0xD800U && firstUnit <= 0xDBFFU) {
                        if (
                            reader.Cursor.IsAtEnd() ||
                            reader.Cursor.Current() != static_cast<std::uint8_t>('\\')
                        ) {
                            diagnostic.ByteOffset = sourceOffset;
                            return JsonDecodingStatus::InvalidUtf8;
                        }
                        reader.Cursor.Advance();
                        if (
                            reader.Cursor.IsAtEnd() ||
                            reader.Cursor.Current() != static_cast<std::uint8_t>('u')
                        ) {
                            diagnostic.ByteOffset = sourceOffset;
                            return JsonDecodingStatus::InvalidUtf8;
                        }
                        reader.Cursor.Advance();

                        std::uint16_t secondUnit = 0U;
                        status = ParseJsonHexQuad(
                            reader.Cursor,
                            secondUnit,
                            diagnostic
                        );
                        if (status != JsonDecodingStatus::Succeeded) { return status; }
                        if (secondUnit < 0xDC00U || secondUnit > 0xDFFFU) {
                            diagnostic.ByteOffset = sourceOffset;
                            return JsonDecodingStatus::InvalidUtf8;
                        }
                        scalar = 0x10000U +
                            ((static_cast<std::uint32_t>(firstUnit) - 0xD800U) << 10U) +
                            (static_cast<std::uint32_t>(secondUnit) - 0xDC00U);
                    } else if (firstUnit >= 0xDC00U && firstUnit <= 0xDFFFU) {
                        diagnostic.ByteOffset = sourceOffset;
                        return JsonDecodingStatus::InvalidUtf8;
                    }

                    status = QueueDecodedStringScalar(
                        reader,
                        scalar,
                        diagnostic,
                        sourceOffset
                    );
                    if (status != JsonDecodingStatus::Succeeded) { return status; }
                    break;
                }
                default:
                    diagnostic.ByteOffset = sourceOffset;
                    return JsonDecodingStatus::MalformedRepresentation;
            }
        } else if (byte <= 0x7FU) {
            reader.Pending[0U] = byte;
            reader.PendingCount = 1U;
            reader.Cursor.Advance();
        } else {
            std::size_t sequenceLength = 0U;
            if (byte >= 0xC2U && byte <= 0xDFU) sequenceLength = 2U;
            else if (byte >= 0xE0U && byte <= 0xEFU) sequenceLength = 3U;
            else if (byte >= 0xF0U && byte <= 0xF4U) sequenceLength = 4U;
            else {
                diagnostic.ByteOffset = sourceOffset;
                return JsonDecodingStatus::InvalidUtf8;
            }
            if (sequenceLength > reader.Cursor.Length() - reader.Cursor.Position()) {
                diagnostic.ByteOffset = sourceOffset;
                return JsonDecodingStatus::InvalidUtf8;
            }
            const auto validation = ValidateUtf8(
                reinterpret_cast<const char*>(reader.Cursor.Data() + reader.Cursor.Position()),
                sequenceLength
            );
            if (!validation.IsSuccessful()) {
                diagnostic.ByteOffset = sourceOffset + validation.ByteOffset;
                return JsonDecodingStatus::InvalidUtf8;
            }
            for (std::size_t index = 0U; index < sequenceLength; ++index) {
                reader.Pending[index] = reader.Cursor.Data()[reader.Cursor.Position() + index];
            }
            reader.PendingCount = sequenceLength;
            reader.Cursor.Advance(sequenceLength);
        }

        value = reader.Pending[reader.PendingIndex++];
        hasValue = true;
        return JsonDecodingStatus::Succeeded;
    }

    /// Compares two JSON String keys by their fully decoded Unicode/UTF-8 byte sequence.
    inline JsonDecodingStatus AreDecodedJsonStringsEqual(
        const std::uint8_t* input,
        std::size_t length,
        std::size_t leftStart,
        std::size_t rightStart,
        bool& equal,
        Diagnostic& diagnostic
    ) noexcept {
        JsonDecodedStringReader left{input, length, leftStart};
        JsonDecodedStringReader right{input, length, rightStart};
        equal = true;

        while (true) {
            std::uint8_t leftValue = 0U;
            std::uint8_t rightValue = 0U;
            bool hasLeft = false;
            bool hasRight = false;
            auto status = NextDecodedJsonStringByte(
                left,
                leftValue,
                hasLeft,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            status = NextDecodedJsonStringByte(
                right,
                rightValue,
                hasRight,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }

            if (hasLeft != hasRight) {
                equal = false;
                return JsonDecodingStatus::Succeeded;
            }
            if (!hasLeft) { return JsonDecodingStatus::Succeeded; }
            if (leftValue != rightValue) {
                equal = false;
                return JsonDecodingStatus::Succeeded;
            }
        }
    }

    /// Parses one canonical numeric FieldIdentifier JSON object key.
    inline JsonDecodingStatus ParseJsonNumericFieldKey(
        JsonInputCursor& cursor,
        System::FieldIdentifier& identifier,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        if (cursor.IsAtEnd() || cursor.Current() != static_cast<std::uint8_t>('"')) {
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::MalformedRepresentation;
        }
        cursor.Advance();

        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::MalformedRepresentation;
        }

        std::uint16_t value = 0U;
        std::size_t digits = 0U;
        bool leadingZero = false;

        while (!cursor.IsAtEnd() && cursor.Current() != static_cast<std::uint8_t>('"')) {
            const auto byte = cursor.Current();
            if (byte < static_cast<std::uint8_t>('0') || byte > static_cast<std::uint8_t>('9')) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::MalformedRepresentation;
            }
            if (digits == 0U) { leadingZero = byte == static_cast<std::uint8_t>('0'); }
            if (leadingZero && digits > 0U) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::MalformedRepresentation;
            }
            if (digits >= 3U) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::MalformedRepresentation;
            }

            value = static_cast<std::uint16_t>(value * 10U + byte - static_cast<std::uint8_t>('0'));
            ++digits;
            cursor.Advance();
        }

        if (digits == 0U || cursor.IsAtEnd() || value > 255U) {
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::MalformedRepresentation;
        }

        cursor.Advance();
        identifier = System::FieldIdentifier{static_cast<System::FieldIdentifier::Storage>(value)};
        return JsonDecodingStatus::Succeeded;
    }

    /// Parses one complete JSON number token according to RFC 8259 grammar.
    inline JsonDecodingStatus ParseJsonNumberToken(
        JsonInputCursor& cursor,
        JsonNumberToken& token,
        Diagnostic& diagnostic
    ) noexcept {
        token = {};
        token.Start = cursor.Position();

        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::TypeMismatch;
        }

        if (cursor.Current() == static_cast<std::uint8_t>('-')) {
            token.IsNegative = true;
            cursor.Advance();
            if (cursor.IsAtEnd()) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::MalformedRepresentation;
            }
        }

        token.IntegerStart = cursor.Position();
        if (cursor.Current() == static_cast<std::uint8_t>('0')) {
            cursor.Advance();
            if (
                !cursor.IsAtEnd() &&
                cursor.Current() >= static_cast<std::uint8_t>('0') &&
                cursor.Current() <= static_cast<std::uint8_t>('9')
            ) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::MalformedRepresentation;
            }
        } else if (
            cursor.Current() >= static_cast<std::uint8_t>('1') &&
            cursor.Current() <= static_cast<std::uint8_t>('9')
        ) {
            do {
                cursor.Advance();
            } while (
                !cursor.IsAtEnd() &&
                cursor.Current() >= static_cast<std::uint8_t>('0') &&
                cursor.Current() <= static_cast<std::uint8_t>('9')
            );
        } else {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::TypeMismatch;
        }
        token.IntegerEnd = cursor.Position();
        token.FractionStart = token.IntegerEnd;
        token.FractionEnd = token.IntegerEnd;

        if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>('.')) {
            cursor.Advance();
            token.FractionStart = cursor.Position();
            while (
                !cursor.IsAtEnd() &&
                cursor.Current() >= static_cast<std::uint8_t>('0') &&
                cursor.Current() <= static_cast<std::uint8_t>('9')
            ) {
                cursor.Advance();
            }
            token.FractionEnd = cursor.Position();
            if (token.FractionEnd == token.FractionStart) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::MalformedRepresentation;
            }
        }

        if (
            !cursor.IsAtEnd() &&
            (cursor.Current() == static_cast<std::uint8_t>('e') || cursor.Current() == static_cast<std::uint8_t>('E'))
        ) {
            token.HasExponent = true;
            cursor.Advance();
            if (cursor.IsAtEnd()) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::MalformedRepresentation;
            }
            if (
                cursor.Current() == static_cast<std::uint8_t>('+') ||
                cursor.Current() == static_cast<std::uint8_t>('-')
            ) {
                token.IsExponentNegative = cursor.Current() == static_cast<std::uint8_t>('-');
                cursor.Advance();
            }

            const auto exponentStart = cursor.Position();
            while (
                !cursor.IsAtEnd() &&
                cursor.Current() >= static_cast<std::uint8_t>('0') &&
                cursor.Current() <= static_cast<std::uint8_t>('9')
            ) {
                const auto digit = static_cast<std::uint8_t>(cursor.Current() - static_cast<std::uint8_t>('0'));
                if (
                    token.ExponentMagnitude >
                    (std::numeric_limits<std::uint64_t>::max() - digit) / 10U
                ) {
                    token.IsExponentMagnitudeOverflow = true;
                } else if (!token.IsExponentMagnitudeOverflow) {
                    token.ExponentMagnitude = token.ExponentMagnitude * 10U + digit;
                }
                cursor.Advance();
            }
            if (cursor.Position() == exponentStart) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::MalformedRepresentation;
            }
        }

        token.End = cursor.Position();
        if (!cursor.IsAtEnd()) {
            const auto next = cursor.Current();
            if (
                !IsJsonWhitespace(next) &&
                next != static_cast<std::uint8_t>(',') &&
                next != static_cast<std::uint8_t>(']') &&
                next != static_cast<std::uint8_t>('}')
            ) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::MalformedRepresentation;
            }
        }

        return JsonDecodingStatus::Succeeded;
    }

    /// Returns one mantissa digit by logical index across integer and fraction spans.
    [[nodiscard]] inline std::uint8_t JsonMantissaDigitAt(
        const JsonInputCursor& cursor,
        const JsonNumberToken& token,
        std::size_t index
    ) noexcept {
        const auto integerDigits = token.IntegerDigits();
        const auto sourceIndex = index < integerDigits
            ? token.IntegerStart + index
            : token.FractionStart + index - integerDigits;
        return static_cast<std::uint8_t>(cursor.Data()[sourceIndex] - static_cast<std::uint8_t>('0'));
    }

    /// Converts an exactly integral JSON number token into one fixed-width integer target.
    ///
    /// @tparam TValue Supported fixed-width destination integer Type.
    template<class TValue>
    JsonDecodingStatus ConvertJsonIntegerToken(
        const JsonInputCursor& cursor,
        const JsonNumberToken& token,
        TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        static_assert(IsFixedWidthInteger<TValue>);

        const auto totalDigits = token.MantissaDigits();
        bool allZero = true;
        for (std::size_t index = 0U; index < totalDigits; ++index) {
            if (JsonMantissaDigitAt(cursor, token, index) != 0U) {
                allZero = false;
                break;
            }
        }

        if (allZero) {
            value = static_cast<TValue>(0);
            return JsonDecodingStatus::Succeeded;
        }

        if (token.IsExponentMagnitudeOverflow) {
            diagnostic.ByteOffset = token.Start;
            return token.IsExponentNegative
                ? JsonDecodingStatus::TypeMismatch
                : JsonDecodingStatus::NumericOutOfRange;
        }

        const auto fractionDigits = token.FractionDigits();
        std::uint64_t removeDigits = 0U;
        std::uint64_t appendZeros = 0U;

        if (token.IsExponentNegative) {
            if (token.ExponentMagnitude > std::numeric_limits<std::uint64_t>::max() - fractionDigits) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::TypeMismatch;
            }
            removeDigits = token.ExponentMagnitude + fractionDigits;
        } else if (token.ExponentMagnitude >= fractionDigits) {
            appendZeros = token.ExponentMagnitude - fractionDigits;
        } else {
            removeDigits = fractionDigits - token.ExponentMagnitude;
        }

        if (removeDigits > totalDigits) {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::TypeMismatch;
        }

        for (std::uint64_t index = 0U; index < removeDigits; ++index) {
            const auto logicalIndex = totalDigits - 1U - static_cast<std::size_t>(index);
            if (JsonMantissaDigitAt(cursor, token, logicalIndex) != 0U) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::TypeMismatch;
            }
        }

        const auto retainedDigits = totalDigits - static_cast<std::size_t>(removeDigits);
        std::uint64_t magnitudeLimit = 0U;
        if constexpr (std::is_signed_v<TValue>) {
            const auto positiveLimit = static_cast<std::uint64_t>(std::numeric_limits<TValue>::max());
            magnitudeLimit = token.IsNegative
                ? positiveLimit + 1U
                : positiveLimit;
        } else {
            magnitudeLimit = static_cast<std::uint64_t>(std::numeric_limits<TValue>::max());
        }

        std::uint64_t magnitude = 0U;
        for (std::size_t index = 0U; index < retainedDigits; ++index) {
            const auto digit = JsonMantissaDigitAt(cursor, token, index);
            if (magnitude > (magnitudeLimit - digit) / 10U) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::NumericOutOfRange;
            }
            magnitude = magnitude * 10U + digit;
        }

        if (appendZeros > 20U && magnitude != 0U) {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::NumericOutOfRange;
        }
        for (std::uint64_t index = 0U; index < appendZeros; ++index) {
            if (magnitude > magnitudeLimit / 10U) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::NumericOutOfRange;
            }
            magnitude *= 10U;
        }

        if constexpr (std::is_unsigned_v<TValue>) {
            if (token.IsNegative && magnitude != 0U) {
                diagnostic.ByteOffset = token.Start;
                return JsonDecodingStatus::NumericOutOfRange;
            }
            value = static_cast<TValue>(magnitude);
        } else {
            if (token.IsNegative) {
                const auto positiveLimit = static_cast<std::uint64_t>(std::numeric_limits<TValue>::max());
                if (magnitude == positiveLimit + 1U) {
                    value = std::numeric_limits<TValue>::min();
                } else {
                    value = static_cast<TValue>(-static_cast<std::int64_t>(magnitude));
                }
            } else {
                value = static_cast<TValue>(magnitude);
            }
        }

        return JsonDecodingStatus::Succeeded;
    }

    /// Determines whether one numeric token's mathematical mantissa is exactly zero.
    [[nodiscard]] inline bool IsJsonNumberZero(
        const JsonInputCursor& cursor,
        const JsonNumberToken& token
    ) noexcept {
        for (std::size_t index = 0U; index < token.MantissaDigits(); ++index) {
            if (JsonMantissaDigitAt(cursor, token, index) != 0U) { return false; }
        }
        return true;
    }

    /// Returns the token's approximate decimal scientific exponent for overflow/underflow classification.
    inline std::int64_t JsonScientificExponent(
        const JsonInputCursor& cursor,
        const JsonNumberToken& token
    ) noexcept {
        std::size_t firstNonZero = token.MantissaDigits();
        for (std::size_t index = 0U; index < token.MantissaDigits(); ++index) {
            if (JsonMantissaDigitAt(cursor, token, index) != 0U) {
                firstNonZero = index;
                break;
            }
        }
        if (firstNonZero == token.MantissaDigits()) { return 0; }

        const auto integerDigits = token.IntegerDigits();
        std::int64_t base = 0;
        if (firstNonZero < integerDigits) {
            const auto distance = integerDigits - firstNonZero - 1U;
            base = distance > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())
                ? std::numeric_limits<std::int64_t>::max()
                : static_cast<std::int64_t>(distance);
        } else {
            const auto distance = firstNonZero - integerDigits + 1U;
            base = distance > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())
                ? std::numeric_limits<std::int64_t>::min()
                : -static_cast<std::int64_t>(distance);
        }

        if (!token.HasExponent) { return base; }
        if (token.IsExponentMagnitudeOverflow) {
            return token.IsExponentNegative
                ? std::numeric_limits<std::int64_t>::min()
                : std::numeric_limits<std::int64_t>::max();
        }

        const auto magnitude = token.ExponentMagnitude;
        if (token.IsExponentNegative) {
            if (magnitude > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
                return std::numeric_limits<std::int64_t>::min();
            }
            const auto exponent = static_cast<std::int64_t>(magnitude);
            if (base < std::numeric_limits<std::int64_t>::min() + exponent) {
                return std::numeric_limits<std::int64_t>::min();
            }
            return base - exponent;
        }

        if (magnitude > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) {
            return std::numeric_limits<std::int64_t>::max();
        }
        const auto exponent = static_cast<std::int64_t>(magnitude);
        if (base > std::numeric_limits<std::int64_t>::max() - exponent) {
            return std::numeric_limits<std::int64_t>::max();
        }
        return base + exponent;
    }

    /// Converts one syntactically valid JSON number token to binary32/binary64.
    ///
    /// @tparam TValue Supported binary32 or binary64 floating Type.
    template<class TValue>
    JsonDecodingStatus ConvertJsonFloatingToken(
        const JsonInputCursor& cursor,
        const JsonNumberToken& token,
        TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        static_assert(IsSupportedFloatingPoint<TValue>);

        if (IsJsonNumberZero(cursor, token)) {
            value = token.IsNegative
                ? -static_cast<TValue>(0)
                : static_cast<TValue>(0);
            return JsonDecodingStatus::Succeeded;
        }

        const auto* begin = reinterpret_cast<const char*>(cursor.Data() + token.Start);
        const auto* end = reinterpret_cast<const char*>(cursor.Data() + token.End);
        TValue converted = static_cast<TValue>(0);
        const auto result = std::from_chars(
            begin,
            end,
            converted
        );

        if (result.ec == std::errc::result_out_of_range) {
            diagnostic.ByteOffset = token.Start;
            return JsonScientificExponent(cursor, token) < 0
                ? JsonDecodingStatus::NumericUnderflow
                : JsonDecodingStatus::NumericOutOfRange;
        }
        if (result.ec != std::errc{} || result.ptr != end) {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::MalformedRepresentation;
        }
        if (!std::isfinite(converted)) {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::NonFiniteNumber;
        }
        if (converted == static_cast<TValue>(0)) {
            diagnostic.ByteOffset = token.Start;
            return JsonDecodingStatus::NumericUnderflow;
        }

        value = converted;
        return JsonDecodingStatus::Succeeded;
    }

    /// Validates one compile-time parser nesting level before entering a container.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    template<class TParserLimits>
    JsonDecodingStatus EnterJsonContainer(
        std::size_t depth,
        Diagnostic& diagnostic,
        std::size_t sourceOffset
    ) noexcept {
        if (depth >= TParserLimits::MaximumNestingDepth) {
            diagnostic.ByteOffset = sourceOffset;
            return JsonDecodingStatus::ResourceLimitExceeded;
        }
        return JsonDecodingStatus::Succeeded;
    }

    /// Accounts for one container item traversed only because its owning Field is unknown.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    template<class TParserLimits>
    JsonDecodingStatus AccountSkippedContainerItem(
        JsonSkipState& skipState,
        Diagnostic& diagnostic,
        std::size_t sourceOffset
    ) noexcept {
        if (skipState.ContainerItems >= TParserLimits::MaximumSkippedContainerItems) {
            diagnostic.ByteOffset = sourceOffset;
            return JsonDecodingStatus::ResourceLimitExceeded;
        }
        ++skipState.ContainerItems;
        return JsonDecodingStatus::Succeeded;
    }

    /// Forward declaration for recursive unknown-value structural validation.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    template<class TParserLimits>
    JsonDecodingStatus SkipJsonValue(
        JsonInputCursor& cursor,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept;

    /// Determines whether one unknown-object key duplicates an earlier decoded key by replaying prior members.
    ///
    /// The scan retains no key table. It intentionally spends bounded CPU and replay work so arbitrary textual
    /// unknown keys do not require permanent or nesting-proportional key storage.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy applied while replaying prior values.
    template<class TParserLimits>
    JsonDecodingStatus HasEarlierEquivalentJsonObjectKey(
        const JsonInputCursor& source,
        std::size_t objectContentStart,
        std::size_t currentKeyStart,
        std::size_t depth,
        bool& duplicate,
        Diagnostic& diagnostic
    ) noexcept {
        duplicate = false;
        JsonInputCursor replay{
            source.Data(),
            source.Length()
        };
        replay.Seek(objectContentStart);
        SkipJsonWhitespace(replay);

        while (replay.Position() < currentKeyStart) {
            const auto priorKeyStart = replay.Position();
            if (
                replay.IsAtEnd() ||
                replay.Current() != static_cast<std::uint8_t>('"')
            ) {
                diagnostic.ByteOffset = priorKeyStart;
                return JsonDecodingStatus::MalformedRepresentation;
            }

            auto status = ParseJsonString(
                replay,
                false,
                [](std::uint8_t) noexcept {
                    return JsonDecodingStatus::Succeeded;
                },
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }

            bool equal = false;
            status = AreDecodedJsonStringsEqual(
                source.Data(),
                source.Length(),
                priorKeyStart,
                currentKeyStart,
                equal,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            if (equal) {
                duplicate = true;
                return JsonDecodingStatus::Succeeded;
            }

            SkipJsonWhitespace(replay);
            status = ConsumeJsonByte(
                replay,
                static_cast<std::uint8_t>(':'),
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(replay);

            JsonSkipState replaySkipState{};
            status = SkipJsonValue<TParserLimits>(
                replay,
                depth + 1U,
                replaySkipState,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(replay);

            if (replay.Position() >= currentKeyStart) { break; }
            status = ConsumeJsonByte(
                replay,
                static_cast<std::uint8_t>(','),
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(replay);
        }

        return JsonDecodingStatus::Succeeded;
    }

    /// Structurally validates and skips one arbitrary JSON array.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    template<class TParserLimits>
    JsonDecodingStatus SkipJsonArray(
        JsonInputCursor& cursor,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        auto status = EnterJsonContainer<TParserLimits>(
            depth,
            diagnostic,
            start
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        cursor.Advance();
        SkipJsonWhitespace(cursor);
        if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>(']')) {
            cursor.Advance();
            return JsonDecodingStatus::Succeeded;
        }

        while (true) {
            status = AccountSkippedContainerItem<TParserLimits>(
                skipState,
                diagnostic,
                cursor.Position()
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }

            status = SkipJsonValue<TParserLimits>(
                cursor,
                depth + 1U,
                skipState,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);

            if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>(']')) {
                cursor.Advance();
                return JsonDecodingStatus::Succeeded;
            }
            status = ConsumeJsonByte(
                cursor,
                static_cast<std::uint8_t>(','),
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
        }
    }

    /// Structurally validates and skips one arbitrary JSON object.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    template<class TParserLimits>
    JsonDecodingStatus SkipJsonObject(
        JsonInputCursor& cursor,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        auto status = EnterJsonContainer<TParserLimits>(
            depth,
            diagnostic,
            start
        );
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        cursor.Advance();
        const auto objectContentStart = cursor.Position();
        SkipJsonWhitespace(cursor);
        if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>('}')) {
            cursor.Advance();
            return JsonDecodingStatus::Succeeded;
        }

        while (true) {
            status = AccountSkippedContainerItem<TParserLimits>(
                skipState,
                diagnostic,
                cursor.Position()
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }

            const auto keyStart = cursor.Position();
            if (cursor.IsAtEnd() || cursor.Current() != static_cast<std::uint8_t>('"')) {
                diagnostic.ByteOffset = keyStart;
                return JsonDecodingStatus::MalformedRepresentation;
            }

            bool duplicateKey = false;
            status = HasEarlierEquivalentJsonObjectKey<TParserLimits>(
                cursor,
                objectContentStart,
                keyStart,
                depth,
                duplicateKey,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            if (duplicateKey) {
                diagnostic.ByteOffset = keyStart;
                return JsonDecodingStatus::DuplicateField;
            }

            status = ParseJsonString(
                cursor,
                false,
                [](std::uint8_t) noexcept {
                    return JsonDecodingStatus::Succeeded;
                },
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
            status = ConsumeJsonByte(
                cursor,
                static_cast<std::uint8_t>(':'),
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
            status = SkipJsonValue<TParserLimits>(
                cursor,
                depth + 1U,
                skipState,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);

            if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>('}')) {
                cursor.Advance();
                return JsonDecodingStatus::Succeeded;
            }
            status = ConsumeJsonByte(
                cursor,
                static_cast<std::uint8_t>(','),
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
        }
    }

    /// Structurally validates and skips one arbitrary unknown JSON value.
    ///
    /// @tparam TParserLimits Compile-time parser resource policy.
    template<class TParserLimits>
    JsonDecodingStatus SkipJsonValue(
        JsonInputCursor& cursor,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        SkipJsonWhitespace(cursor);
        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = cursor.Position();
            return JsonDecodingStatus::MalformedRepresentation;
        }

        switch (cursor.Current()) {
            case static_cast<std::uint8_t>('"'):
                return ParseJsonString(
                    cursor,
                    false,
                    [](std::uint8_t) noexcept {
                        return JsonDecodingStatus::Succeeded;
                    },
                    diagnostic
                );
            case static_cast<std::uint8_t>('{'):
                return SkipJsonObject<TParserLimits>(
                    cursor,
                    depth,
                    skipState,
                    diagnostic
                );
            case static_cast<std::uint8_t>('['):
                return SkipJsonArray<TParserLimits>(
                    cursor,
                    depth,
                    skipState,
                    diagnostic
                );
            case static_cast<std::uint8_t>('t'):
                return ConsumeJsonLiteral(cursor, "true", 4U, diagnostic);
            case static_cast<std::uint8_t>('f'):
                return ConsumeJsonLiteral(cursor, "false", 5U, diagnostic);
            case static_cast<std::uint8_t>('n'):
                return ConsumeJsonLiteral(cursor, "null", 4U, diagnostic);
            default: {
                if (
                    cursor.Current() != static_cast<std::uint8_t>('-') &&
                    (cursor.Current() < static_cast<std::uint8_t>('0') || cursor.Current() > static_cast<std::uint8_t>('9'))
                ) {
                    diagnostic.ByteOffset = cursor.Position();
                    return JsonDecodingStatus::MalformedRepresentation;
                }
                JsonNumberToken token{};
                return ParseJsonNumberToken(
                    cursor,
                    token,
                    diagnostic
                );
            }
        }
    }

    /// Streams one decoded bounded String, optionally retaining bytes in the destination.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TValue Concrete bounded String Type.
    template<bool TPopulate, class TValue>
    JsonDecodingStatus DecodeJsonBoundedString(
        JsonInputCursor& cursor,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        std::size_t decodedBytes = 0U;
        if constexpr (TPopulate) { destination->Clear(); }

        const auto status = ParseJsonString(
            cursor,
            true,
            [&](std::uint8_t byte) noexcept {
                if (decodedBytes >= BoundedStringTraits<TValue>::Capacity) {
                    return JsonDecodingStatus::CapacityExceeded;
                }
                ++decodedBytes;
                if constexpr (TPopulate) {
                    const auto result = destination->PushBack(static_cast<char>(byte));
                    if (result != Bounded::StringPushBackResult::Succeeded) {
                        return JsonDecodingStatus::CapacityExceeded;
                    }
                }
                return JsonDecodingStatus::Succeeded;
            },
            diagnostic
        );

        return status;
    }

    /// Maps one canonical Base64 character to its six-bit value.
    [[nodiscard]] constexpr int Base64Value(
        std::uint8_t value
    ) noexcept {
        if (value >= static_cast<std::uint8_t>('A') && value <= static_cast<std::uint8_t>('Z'))
            return static_cast<int>(value - static_cast<std::uint8_t>('A'));
        if (value >= static_cast<std::uint8_t>('a') && value <= static_cast<std::uint8_t>('z'))
            return static_cast<int>(26U + value - static_cast<std::uint8_t>('a'));
        if (value >= static_cast<std::uint8_t>('0') && value <= static_cast<std::uint8_t>('9'))
            return static_cast<int>(52U + value - static_cast<std::uint8_t>('0'));
        if (value == static_cast<std::uint8_t>('+')) return 62;
        if (value == static_cast<std::uint8_t>('/')) return 63;
        return -1;
    }

    /// Streams one strict padded RFC 4648 Base64 String into bounded Bytes.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TValue Concrete bounded Bytes Type.
    template<bool TPopulate, class TValue>
    JsonDecodingStatus DecodeJsonBoundedBytes(
        JsonInputCursor& cursor,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        if constexpr (TPopulate) { destination->Clear(); }

        std::array<std::uint8_t, 4U> quartet{};
        std::size_t quartetSize = 0U;
        std::size_t decodedBytes = 0U;
        bool finalQuartetSeen = false;

        const auto status = ParseJsonString(
            cursor,
            false,
            [&](std::uint8_t byte) noexcept {
                if (finalQuartetSeen) { return JsonDecodingStatus::InvalidBase64; }
                if (byte > 0x7FU) { return JsonDecodingStatus::InvalidBase64; }

                quartet[quartetSize++] = byte;
                if (quartetSize != 4U) { return JsonDecodingStatus::Succeeded; }

                const auto first = Base64Value(quartet[0U]);
                const auto second = Base64Value(quartet[1U]);
                if (first < 0 || second < 0) { return JsonDecodingStatus::InvalidBase64; }

                std::array<std::uint8_t, 3U> output{};
                std::size_t outputCount = 0U;
                if (quartet[2U] == static_cast<std::uint8_t>('=')) {
                    if (quartet[3U] != static_cast<std::uint8_t>('=') || (second & 0x0F) != 0) {
                        return JsonDecodingStatus::InvalidBase64;
                    }
                    output[0U] = static_cast<std::uint8_t>((first << 2U) | (second >> 4U));
                    outputCount = 1U;
                    finalQuartetSeen = true;
                } else {
                    const auto third = Base64Value(quartet[2U]);
                    if (third < 0) { return JsonDecodingStatus::InvalidBase64; }
                    output[0U] = static_cast<std::uint8_t>((first << 2U) | (second >> 4U));
                    output[1U] = static_cast<std::uint8_t>((second << 4U) | (third >> 2U));
                    if (quartet[3U] == static_cast<std::uint8_t>('=')) {
                        if ((third & 0x03) != 0) { return JsonDecodingStatus::InvalidBase64; }
                        outputCount = 2U;
                        finalQuartetSeen = true;
                    } else {
                        const auto fourth = Base64Value(quartet[3U]);
                        if (fourth < 0) { return JsonDecodingStatus::InvalidBase64; }
                        output[2U] = static_cast<std::uint8_t>((third << 6U) | fourth);
                        outputCount = 3U;
                    }
                }

                if (outputCount > BoundedBytesTraits<TValue>::Capacity - decodedBytes) {
                    return JsonDecodingStatus::CapacityExceeded;
                }
                decodedBytes += outputCount;

                if constexpr (TPopulate) {
                    for (std::size_t index = 0U; index < outputCount; ++index) {
                        const auto append = destination->PushBack(output[index]);
                        if (append != Bounded::BytesPushBackResult::Succeeded) {
                            return JsonDecodingStatus::CapacityExceeded;
                        }
                    }
                }

                quartetSize = 0U;
                return JsonDecodingStatus::Succeeded;
            },
            diagnostic
        );

        if (status != JsonDecodingStatus::Succeeded) { return status; }
        if (quartetSize != 0U) { return JsonDecodingStatus::InvalidBase64; }
        return JsonDecodingStatus::Succeeded;
    }

    /// Forward declaration for recursive schema/container value decoding.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Serialisable target value Type.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    JsonDecodingStatus DecodeJsonValue(
        JsonInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept;

    /// Decodes one exact-length fixed array representation.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Compile-time unknown-Field policy propagated to nested values.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Fixed-array target Type.
    /// @tparam TAccessor Callable returning the destination element pointer for one valid index.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue,
        class TAccessor
    >
    JsonDecodingStatus DecodeJsonFixedArray(
        JsonInputCursor& cursor,
        std::size_t count,
        TAccessor&& accessor,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        auto status = EnterJsonContainer<TParserLimits>(depth, diagnostic, start);
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        if (cursor.IsAtEnd() || cursor.Current() != static_cast<std::uint8_t>('[')) {
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::TypeMismatch;
        }
        cursor.Advance();
        SkipJsonWhitespace(cursor);

        for (std::size_t index = 0U; index < count; ++index) {
            if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>(']')) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::TypeMismatch;
            }
            if (index != 0U) {
                status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(','), diagnostic);
                if (status != JsonDecodingStatus::Succeeded) { return status; }
                SkipJsonWhitespace(cursor);
            }

            status = DecodeJsonValue<TPopulate, TStrictness, TParserLimits>(
                cursor,
                accessor(index),
                depth + 1U,
                skipState,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
        }

        if (cursor.IsAtEnd() || cursor.Current() != static_cast<std::uint8_t>(']')) {
            diagnostic.ByteOffset = cursor.Position();
            return JsonDecodingStatus::TypeMismatch;
        }
        cursor.Advance();
        return JsonDecodingStatus::Succeeded;
    }

    /// Decodes one bounded variable-length array representation.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Compile-time unknown-Field policy propagated to elements.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Concrete bounded Vector target Type.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    JsonDecodingStatus DecodeJsonVector(
        JsonInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        auto status = EnterJsonContainer<TParserLimits>(depth, diagnostic, start);
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        if (cursor.IsAtEnd() || cursor.Current() != static_cast<std::uint8_t>('[')) {
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::TypeMismatch;
        }
        cursor.Advance();
        SkipJsonWhitespace(cursor);
        if constexpr (TPopulate) { destination->Clear(); }
        if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>(']')) {
            cursor.Advance();
            return JsonDecodingStatus::Succeeded;
        }

        std::size_t count = 0U;
        while (true) {
            if (count >= BoundedVectorTraits<TValue>::Capacity) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::CapacityExceeded;
            }

            using Element = typename BoundedVectorTraits<TValue>::Element;
            Element* element = nullptr;
            if constexpr (TPopulate) {
                const auto emplace = destination->EmplaceBack();
                if (emplace != Bounded::VectorEmplaceBackResult::Succeeded) {
                    return JsonDecodingStatus::CapacityExceeded;
                }
                element = &(*destination)[destination->Size() - 1U];
            }

            status = DecodeJsonValue<TPopulate, TStrictness, TParserLimits>(
                cursor,
                element,
                depth + 1U,
                skipState,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            ++count;
            SkipJsonWhitespace(cursor);

            if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>(']')) {
                cursor.Advance();
                return JsonDecodingStatus::Succeeded;
            }
            status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(','), diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
        }
    }

    /// Reports whether a schema Field identifier bit has already been observed.
    [[nodiscard]] inline bool IsFieldSeen(
        const std::array<std::uint8_t, 32U>& seen,
        System::FieldIdentifier identifier
    ) noexcept {
        const auto value = identifier.Value();
        return (seen[value / 8U] & static_cast<std::uint8_t>(1U << (value % 8U))) != 0U;
    }

    /// Marks one schema Field identifier as observed.
    inline void MarkFieldSeen(
        std::array<std::uint8_t, 32U>& seen,
        System::FieldIdentifier identifier
    ) noexcept {
        const auto value = identifier.Value();
        seen[value / 8U] = static_cast<std::uint8_t>(
            seen[value / 8U] | static_cast<std::uint8_t>(1U << (value % 8U))
        );
    }

    /// Dispatches one runtime FieldIdentifier to its compile-time schema binding.
    ///
    /// @tparam TIdentifier Current compile-time numeric FieldIdentifier candidate.
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Schema owner Type.
    template<
        std::uint16_t TIdentifier,
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    JsonDecodingStatus DecodeJsonSchemaField(
        JsonInputCursor& cursor,
        System::FieldIdentifier identifier,
        TValue* destination,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic,
        bool& knownField
    ) noexcept {
        if constexpr (TIdentifier > 255U) {
            knownField = false;
            return JsonDecodingStatus::Succeeded;
        } else {
            if (identifier.Value() != static_cast<System::FieldIdentifier::Storage>(TIdentifier)) {
                return DecodeJsonSchemaField<
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
                return JsonDecodingStatus::Succeeded;
            } else {
                knownField = true;
                using FieldValue = std::remove_cv_t<System::FieldValueOf<Field>>;
                if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                    SkipJsonWhitespace(cursor);
                    if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>('n')) {
                        const auto nullStart = cursor.Position();
                        const auto nullStatus = ConsumeJsonLiteral(cursor, "null", 4U, diagnostic);
                        if (nullStatus != JsonDecodingStatus::Succeeded) {
                            diagnostic.ByteOffset = nullStart;
                            diagnostic.Type = TValue::Identifier;
                            diagnostic.Field = Field::Identifier;
                            return nullStatus;
                        }
                        if constexpr (TPopulate) {
                            (destination->*Field::Member).reset();
                        }
                        return JsonDecodingStatus::Succeeded;
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
                    const auto fieldStatus = DecodeJsonValue<TPopulate, TStrictness, TParserLimits>(
                        cursor,
                        element,
                        depth,
                        skipState,
                        diagnostic
                    );
                    if (fieldStatus != JsonDecodingStatus::Succeeded) {
                        if (!diagnostic.Type.has_value()) { diagnostic.Type = TValue::Identifier; }
                        if (!diagnostic.Field.has_value()) { diagnostic.Field = Field::Identifier; }
                    }
                    return fieldStatus;
                } else {
                    FieldValue* field = destination == nullptr
                        ? nullptr
                        : &(destination->*Field::Member);
                    const auto fieldStatus = DecodeJsonValue<TPopulate, TStrictness, TParserLimits>(
                        cursor,
                        field,
                        depth,
                        skipState,
                        diagnostic
                    );
                    if (fieldStatus != JsonDecodingStatus::Succeeded) {
                        if (!diagnostic.Type.has_value()) { diagnostic.Type = TValue::Identifier; }
                        if (!diagnostic.Field.has_value()) { diagnostic.Field = Field::Identifier; }
                    }
                    return fieldStatus;
                }
            }
        }
    }

    /// Validates required-Field presence and resets omitted Optional Fields during population.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TValue Schema owner Type.
    template<bool TPopulate, class TValue>
    JsonDecodingStatus FinaliseJsonSchemaPresence(
        const std::array<std::uint8_t, 32U>& seen,
        TValue* destination,
        Diagnostic& diagnostic
    ) noexcept {
        JsonDecodingStatus status = JsonDecodingStatus::Succeeded;
        System::ForEachField<TValue>([&]<class TField>() constexpr {
            if (status != JsonDecodingStatus::Succeeded) { return; }
            using FieldValue = std::remove_cv_t<System::FieldValueOf<TField>>;
            const bool present = IsFieldSeen(seen, TField::Identifier);

            if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                if constexpr (TPopulate) {
                    if (!present) { (destination->*TField::Member).reset(); }
                }
            } else if (!present) {
                diagnostic.Type = TValue::Identifier;
                diagnostic.Field = TField::Identifier;
                status = JsonDecodingStatus::MissingRequiredField;
            }
        });
        return status;
    }

    /// Decodes one numeric-Field schema object in arbitrary input Field order.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Schema owner Type.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    JsonDecodingStatus DecodeJsonSchema(
        JsonInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        const auto start = cursor.Position();
        auto status = EnterJsonContainer<TParserLimits>(depth, diagnostic, start);
        if (status != JsonDecodingStatus::Succeeded) { return status; }
        if (cursor.IsAtEnd() || cursor.Current() != static_cast<std::uint8_t>('{')) {
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::TypeMismatch;
        }
        cursor.Advance();
        SkipJsonWhitespace(cursor);

        std::array<std::uint8_t, 32U> seen{};
        if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>('}')) {
            cursor.Advance();
            return FinaliseJsonSchemaPresence<TPopulate>(seen, destination, diagnostic);
        }

        while (true) {
            const auto keyStart = cursor.Position();
            System::FieldIdentifier identifier{0U};
            status = ParseJsonNumericFieldKey(cursor, identifier, diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            if (IsFieldSeen(seen, identifier)) {
                diagnostic.ByteOffset = keyStart;
                diagnostic.Type = TValue::Identifier;
                diagnostic.Field = identifier;
                return JsonDecodingStatus::DuplicateField;
            }
            MarkFieldSeen(seen, identifier);

            SkipJsonWhitespace(cursor);
            status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(':'), diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);

            bool knownField = false;
            status = DecodeJsonSchemaField<
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
            if (status != JsonDecodingStatus::Succeeded) { return status; }

            if (!knownField) {
                diagnostic.Type = TValue::Identifier;
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

            SkipJsonWhitespace(cursor);
            if (!cursor.IsAtEnd() && cursor.Current() == static_cast<std::uint8_t>('}')) {
                cursor.Advance();
                return FinaliseJsonSchemaPresence<TPopulate>(seen, destination, diagnostic);
            }
            status = ConsumeJsonByte(cursor, static_cast<std::uint8_t>(','), diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            SkipJsonWhitespace(cursor);
        }
    }

    /// Decodes one value from the currently implemented JSON Numeric Known-Type profile.
    ///
    /// @tparam TPopulate false for validation-only traversal; true for destination population.
    /// @tparam TStrictness Compile-time unknown-Field policy.
    /// @tparam TParserLimits Compile-time parser resource policy.
    /// @tparam TValue Serialisable target value Type.
    template<
        bool TPopulate,
        StrictnessPolicy TStrictness,
        class TParserLimits,
        class TValue
    >
    JsonDecodingStatus DecodeJsonValue(
        JsonInputCursor& cursor,
        TValue* destination,
        std::size_t depth,
        JsonSkipState& skipState,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(IsSerialisableType<Value>);
        SkipJsonWhitespace(cursor);
        if (cursor.IsAtEnd()) {
            diagnostic.ByteOffset = cursor.Position();
            return JsonDecodingStatus::MalformedRepresentation;
        }

        if constexpr (std::is_same_v<Value, bool>) {
            const auto start = cursor.Position();
            if (cursor.Current() == static_cast<std::uint8_t>('t')) {
                const auto status = ConsumeJsonLiteral(cursor, "true", 4U, diagnostic);
                if (status == JsonDecodingStatus::Succeeded && TPopulate) { *destination = true; }
                return status;
            }
            if (cursor.Current() == static_cast<std::uint8_t>('f')) {
                const auto status = ConsumeJsonLiteral(cursor, "false", 5U, diagnostic);
                if (status == JsonDecodingStatus::Succeeded && TPopulate) { *destination = false; }
                return status;
            }
            diagnostic.ByteOffset = start;
            return JsonDecodingStatus::TypeMismatch;
        } else if constexpr (IsFixedWidthInteger<Value>) {
            if (
                cursor.Current() != static_cast<std::uint8_t>('-') &&
                (cursor.Current() < static_cast<std::uint8_t>('0') || cursor.Current() > static_cast<std::uint8_t>('9'))
            ) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::TypeMismatch;
            }
            JsonNumberToken token{};
            auto status = ParseJsonNumberToken(cursor, token, diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            Value converted{};
            status = ConvertJsonIntegerToken(cursor, token, converted, diagnostic);
            if (status == JsonDecodingStatus::Succeeded && TPopulate) { *destination = converted; }
            return status;
        } else if constexpr (IsSupportedFloatingPoint<Value>) {
            if (
                cursor.Current() != static_cast<std::uint8_t>('-') &&
                (cursor.Current() < static_cast<std::uint8_t>('0') || cursor.Current() > static_cast<std::uint8_t>('9'))
            ) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::TypeMismatch;
            }
            JsonNumberToken token{};
            auto status = ParseJsonNumberToken(cursor, token, diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            Value converted{};
            status = ConvertJsonFloatingToken(cursor, token, converted, diagnostic);
            if (status == JsonDecodingStatus::Succeeded && TPopulate) { *destination = converted; }
            return status;
        } else if constexpr (std::is_enum_v<Value>) {
            using Underlying = typename EnumSerialisationTraits<Value>::UnderlyingType;
            Underlying converted{};
            if (
                cursor.Current() != static_cast<std::uint8_t>('-') &&
                (cursor.Current() < static_cast<std::uint8_t>('0') || cursor.Current() > static_cast<std::uint8_t>('9'))
            ) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::TypeMismatch;
            }
            JsonNumberToken token{};
            auto status = ParseJsonNumberToken(cursor, token, diagnostic);
            if (status != JsonDecodingStatus::Succeeded) { return status; }
            status = ConvertJsonIntegerToken(cursor, token, converted, diagnostic);
            if (status == JsonDecodingStatus::Succeeded && TPopulate) {
                *destination = static_cast<Value>(converted);
            }
            return status;
        } else if constexpr (OptionalValueTraits<Value>::IsValue) {
            if (cursor.Current() == static_cast<std::uint8_t>('n')) {
                const auto start = cursor.Position();
                const auto status = ConsumeJsonLiteral(cursor, "null", 4U, diagnostic);
                if (status != JsonDecodingStatus::Succeeded) {
                    diagnostic.ByteOffset = start;
                    return status;
                }
                if constexpr (TPopulate) { destination->reset(); }
                return JsonDecodingStatus::Succeeded;
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
            return DecodeJsonValue<TPopulate, TStrictness, TParserLimits>(
                cursor,
                element,
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (StandardArrayTraits<Value>::IsValue) {
            return DecodeJsonFixedArray<TPopulate, TStrictness, TParserLimits, Value>(
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
            return DecodeJsonFixedArray<TPopulate, TStrictness, TParserLimits, Value>(
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
            if (cursor.Current() != static_cast<std::uint8_t>('"')) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::TypeMismatch;
            }
            return DecodeJsonBoundedString<TPopulate>(cursor, destination, diagnostic);
        } else if constexpr (BoundedBytesTraits<Value>::IsValue) {
            if (cursor.Current() != static_cast<std::uint8_t>('"')) {
                diagnostic.ByteOffset = cursor.Position();
                return JsonDecodingStatus::TypeMismatch;
            }
            return DecodeJsonBoundedBytes<TPopulate>(cursor, destination, diagnostic);
        } else if constexpr (BoundedVectorTraits<Value>::IsValue) {
            return DecodeJsonVector<TPopulate, TStrictness, TParserLimits>(
                cursor,
                destination,
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (PotentialSchemaType<Value>) {
            return DecodeJsonSchema<TPopulate, TStrictness, TParserLimits>(
                cursor,
                destination,
                depth,
                skipState,
                diagnostic
            );
        } else if constexpr (HasCanonicalRepresentation<Value>) {
            using Representation = std::remove_cv_t<typename CanonicalRepresentation<Value>::Type>;
            Representation representation{};
            auto status = DecodeJsonValue<true, TStrictness, TParserLimits>(
                cursor,
                &representation,
                depth,
                skipState,
                diagnostic
            );
            if (status != JsonDecodingStatus::Succeeded) { return status; }

            using Adapter = Bounded::TypeConversionAdapter<Representation, Value>;
            if constexpr (TPopulate) {
                const auto result = Adapter::Convert(representation, *destination);
                if (!Bounded::IsTypeConversionSuccessful<Representation, Value>(result)) {
                    return JsonDecodingStatus::AdaptationFailed;
                }
            } else if constexpr (std::is_nothrow_default_constructible_v<Value>) {
                Value validationTarget{};
                const auto result = Adapter::Convert(representation, validationTarget);
                if (!Bounded::IsTypeConversionSuccessful<Representation, Value>(result)) {
                    return JsonDecodingStatus::AdaptationFailed;
                }
            } else {
                if (destination == nullptr) { return JsonDecodingStatus::AdaptationFailed; }
                Value validationTarget{*destination};
                const auto result = Adapter::Convert(representation, validationTarget);
                if (!Bounded::IsTypeConversionSuccessful<Representation, Value>(result)) {
                    return JsonDecodingStatus::AdaptationFailed;
                }
            }
            return JsonDecodingStatus::Succeeded;
        } else {
            static_assert(IsSerialisableType<Value>);
            return JsonDecodingStatus::TypeMismatch;
        }
    }

} // ESPressio::Serialisation::Detail
