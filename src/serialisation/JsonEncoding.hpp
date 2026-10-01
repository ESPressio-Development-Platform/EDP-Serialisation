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
#include <ESPressio_Memory.hpp>
#include <ESPressio_Platform_Portable_ByteOperations.hpp>
#include <ESPressio_System.hpp>

#include "CanonicalRepresentation.hpp"
#include "Results.hpp"
#include "SerialisableType.hpp"

namespace ESPressio::Serialisation::Detail {

    /// Internal outcomes produced while appending bytes to one JSON sink.
    enum class JsonSinkWriteResult : std::uint8_t {
        /// The requested bytes were accounted for or retained successfully.
        Succeeded = 0U,
        /// The sink cannot represent or retain the requested additional bytes.
        CapacityExceeded = 1U
    };

    /// Internal outcomes produced by the allocation-free JSON encoder traversal.
    enum class JsonEncodingStatus : std::uint8_t {
        /// The complete represented value was validated and emitted successfully.
        Succeeded = 0U,
        /// Size accounting or the selected output sink could not retain the complete representation.
        ResourceLimitExceeded = 1U,
        /// A floating value is NaN or infinite and therefore outside the V1 domain.
        NonFiniteNumber = 2U,
        /// A bounded String contains invalid V1 UTF-8 text.
        InvalidUtf8 = 3U,
        /// A strong semantic Type could not convert to its canonical representation.
        AdaptationFailed = 4U
    };

    /// Internal UTF-8 validation outcomes for one bounded text payload.
    enum class Utf8ValidationStatus : std::uint8_t {
        /// The complete payload is valid V1 UTF-8 text.
        Succeeded = 0U,
        /// The payload contains malformed UTF-8 or the forbidden U+0000 scalar value.
        InvalidUtf8 = 1U
    };

    /// Carries the result and source offset of one complete UTF-8 validation pass.
    struct Utf8ValidationResult final {

        // Validation outcome.

        /// Strongly typed validation outcome.
        Utf8ValidationStatus Status = Utf8ValidationStatus::Succeeded;

        // Source location.

        /// Source byte offset associated with InvalidUtf8, or zero on success.
        std::size_t ByteOffset = 0U;

        // Outcome predicates.

        /// Reports whether the complete UTF-8 payload is valid.
        ///
        /// @return true only when Status is Utf8ValidationStatus::Succeeded.
        [[nodiscard]] constexpr bool IsSuccessful() const noexcept {
            return Status == Utf8ValidationStatus::Succeeded;
        }

    };

    /// Zero-storage sink which computes the exact number of encoded JSON bytes.
    class JsonCountingSink final {
    private:

        // Size state.

        /// Exact number of JSON bytes accounted for so far.
        std::size_t _size = 0U;

    public:

        // Byte accounting.

        /// Accounts for one encoded byte while protecting size_t from overflow.
        ///
        /// @param value Encoded byte whose value is irrelevant to counting.
        /// @return Succeeded when one byte was accounted for; CapacityExceeded on size_t overflow.
        JsonSinkWriteResult WriteByte(
            std::uint8_t value
        ) noexcept {
            static_cast<void>(value);

            if (_size == std::numeric_limits<std::size_t>::max()) {
                return JsonSinkWriteResult::CapacityExceeded;
            }

            ++_size;
            return JsonSinkWriteResult::Succeeded;
        }

        /// Accounts for an encoded byte range while protecting size_t from overflow.
        ///
        /// @param source Source bytes whose values are irrelevant to counting.
        /// @param length Number of encoded bytes represented by the range.
        /// @return Succeeded when the complete range was accounted for; CapacityExceeded on size_t overflow.
        JsonSinkWriteResult WriteBytes(
            const char* source,
            std::size_t length
        ) noexcept {
            static_cast<void>(source);

            if (length > std::numeric_limits<std::size_t>::max() - _size) {
                return JsonSinkWriteResult::CapacityExceeded;
            }

            _size += length;
            return JsonSinkWriteResult::Succeeded;
        }

        // Sink state inspection.

        /// Returns the exact number of encoded bytes accounted for so far.
        [[nodiscard]] constexpr std::size_t Size() const noexcept {
            return _size;
        }

    };

    /// Caller-buffer sink used only after successful exact pre-measurement.
    ///
    /// @tparam TByteOperationsProvider Stateless EDP-Memory ByteOperations provider selected at compile time.
    template<
        class TByteOperationsProvider = ESPressio::Platform::Portable::Memory::ByteOperationsProvider
    >
    class JsonBufferSink final {
    private:

        static_assert(
            sizeof(ESPressio::Memory::Detail::ByteOperationsProviderTraits<TByteOperationsProvider>) > 0U,
            "JSON output requires a provider satisfying the EDP-Memory ByteOperations contract"
        );

        static_assert(
            std::is_empty_v<TByteOperationsProvider>,
            "JSON output requires a stateless ByteOperations provider because no provider state is retained"
        );

        static_assert(
            std::is_nothrow_default_constructible_v<TByteOperationsProvider>,
            "JSON output requires a nothrow default-constructible ByteOperations provider"
        );

        // Caller-owned storage.

        /// First byte of caller-owned output storage.
        std::uint8_t* _output = nullptr;

        /// Total writable capacity of caller-owned output storage.
        std::size_t _capacity = 0U;

        /// Number of bytes committed to caller-owned storage so far.
        std::size_t _size = 0U;

    public:

        // Construction.

        /// Binds the sink to caller-owned storage for one complete serialisation pass.
        ///
        /// @param output First byte of caller-owned output storage.
        /// @param capacity Total writable output capacity in bytes.
        JsonBufferSink(
            std::uint8_t* output,
            std::size_t capacity
        ) noexcept :
            _output(output),
            _capacity(capacity) {
        }

        // Byte emission.

        /// Writes one encoded byte when caller-owned capacity remains available.
        ///
        /// @param value Encoded byte to retain.
        /// @return Succeeded when the byte was retained; CapacityExceeded otherwise.
        JsonSinkWriteResult WriteByte(
            std::uint8_t value
        ) noexcept {
            if (_size >= _capacity) { return JsonSinkWriteResult::CapacityExceeded; }

            _output[_size] = value;
            ++_size;
            return JsonSinkWriteResult::Succeeded;
        }

        /// Writes a complete encoded byte range through the selected EDP-Memory ByteOperations provider.
        ///
        /// @param source First source byte to copy.
        /// @param length Number of bytes to copy.
        /// @return Succeeded when the complete range was retained; CapacityExceeded otherwise.
        JsonSinkWriteResult WriteBytes(
            const char* source,
            std::size_t length
        ) noexcept {
            if (_size > _capacity || length > _capacity - _size) {
                return JsonSinkWriteResult::CapacityExceeded;
            }

            TByteOperationsProvider{}.CopyBytes(
                _output + _size,
                source,
                length
            );

            _size += length;
            return JsonSinkWriteResult::Succeeded;
        }

        // Sink state inspection.

        /// Returns the number of bytes committed to caller-owned storage so far.
        [[nodiscard]] constexpr std::size_t Size() const noexcept {
            return _size;
        }

    };

    /// Describes one standard fixed-size array specialization.
    ///
    /// @tparam TValue Candidate Type being inspected.
    template<class TValue>
    struct StandardArrayTraits final {

        /// Indicates that an unrelated Type is not a std::array specialization.
        static constexpr bool IsValue = false;

    };

    /// Describes one concrete standard fixed-size array specialization.
    ///
    /// @tparam TElement Array element Type.
    /// @tparam TLength Exact array extent.
    template<class TElement, std::size_t TLength>
    struct StandardArrayTraits<std::array<TElement, TLength>> final {

        /// Indicates that the inspected Type is a std::array specialization.
        static constexpr bool IsValue = true;

        /// Element Type retained by the array.
        using Element = TElement;

        /// Exact number of array elements.
        static constexpr std::size_t Count = TLength;

    };

    /// Describes one standard Optional specialization.
    ///
    /// @tparam TValue Candidate Type being inspected.
    template<class TValue>
    struct OptionalValueTraits final {

        /// Indicates that an unrelated Type is not a std::optional specialization.
        static constexpr bool IsValue = false;

    };

    /// Describes one concrete standard Optional specialization.
    ///
    /// @tparam TElement Optional element Type.
    template<class TElement>
    struct OptionalValueTraits<std::optional<TElement>> final {

        /// Indicates that the inspected Type is a std::optional specialization.
        static constexpr bool IsValue = true;

        /// Element Type retained when the Optional is engaged.
        using Element = TElement;

    };

    /// Describes one bounded String specialization.
    ///
    /// @tparam TValue Candidate Type being inspected.
    template<class TValue>
    struct BoundedStringTraits final {

        /// Indicates that an unrelated Type is not a Bounded::String specialization.
        static constexpr bool IsValue = false;

    };

    /// Describes one concrete bounded String specialization.
    ///
    /// @tparam TCapacity Maximum retained UTF-8 payload bytes.
    /// @tparam TProvider Selected BoundedTypes byte-operations provider.
    template<std::size_t TCapacity, class TProvider>
    struct BoundedStringTraits<Bounded::String<TCapacity, TProvider>> final {

        /// Indicates that the inspected Type is a Bounded::String specialization.
        static constexpr bool IsValue = true;

        /// Maximum decoded UTF-8 payload bytes retained by the String.
        static constexpr std::size_t Capacity = TCapacity;

    };

    /// Describes one bounded Bytes specialization.
    ///
    /// @tparam TValue Candidate Type being inspected.
    template<class TValue>
    struct BoundedBytesTraits final {

        /// Indicates that an unrelated Type is not a Bounded::Bytes specialization.
        static constexpr bool IsValue = false;

    };

    /// Describes one concrete bounded Bytes specialization.
    ///
    /// @tparam TCapacity Maximum retained octets.
    /// @tparam TProvider Selected BoundedTypes byte-operations provider.
    template<std::size_t TCapacity, class TProvider>
    struct BoundedBytesTraits<Bounded::Bytes<TCapacity, TProvider>> final {

        /// Indicates that the inspected Type is a Bounded::Bytes specialization.
        static constexpr bool IsValue = true;

        /// Maximum decoded octets retained by the Bytes value.
        static constexpr std::size_t Capacity = TCapacity;

    };

    /// Describes one bounded Vector specialization.
    ///
    /// @tparam TValue Candidate Type being inspected.
    template<class TValue>
    struct BoundedVectorTraits final {

        /// Indicates that an unrelated Type is not a Bounded::Vector specialization.
        static constexpr bool IsValue = false;

    };

    /// Describes one concrete bounded Vector specialization.
    ///
    /// @tparam TElement Vector element Type.
    /// @tparam TCapacity Maximum retained element count.
    template<class TElement, std::size_t TCapacity>
    struct BoundedVectorTraits<Bounded::Vector<TElement, TCapacity>> final {

        /// Indicates that the inspected Type is a Bounded::Vector specialization.
        static constexpr bool IsValue = true;

        /// Element Type retained by the Vector.
        using Element = TElement;

        /// Maximum logical element count retained by the Vector.
        static constexpr std::size_t Capacity = TCapacity;

    };

    /// Finds the FieldBinding with one compile-time FieldIdentifier inside a FieldSet.
    ///
    /// @tparam TFieldSet Canonical System::FieldSet being searched.
    /// @tparam TIdentifier Numeric FieldIdentifier being selected.
    template<class TFieldSet, System::FieldIdentifier::Storage TIdentifier>
    struct FieldBindingForIdentifier;

    /// Terminates FieldBinding lookup when the FieldSet contains no matching binding.
    ///
    /// @tparam TIdentifier Numeric FieldIdentifier being selected.
    template<System::FieldIdentifier::Storage TIdentifier>
    struct FieldBindingForIdentifier<System::FieldSet<>, TIdentifier> final {

        /// Missing Field marker.
        using Type = void;

    };

    /// Recursively selects one binding by its stable numeric FieldIdentifier.
    ///
    /// @tparam TFirst First FieldBinding in the remaining FieldSet.
    /// @tparam TRest Remaining FieldBindings.
    /// @tparam TIdentifier Numeric FieldIdentifier being selected.
    template<class TFirst, class... TRest, System::FieldIdentifier::Storage TIdentifier>
    struct FieldBindingForIdentifier<System::FieldSet<TFirst, TRest...>, TIdentifier> final {

        /// Matching FieldBinding, or void when no remaining binding has TIdentifier.
        using Type = std::conditional_t<
            TFirst::Identifier.Value() == TIdentifier,
            TFirst,
            typename FieldBindingForIdentifier<
                System::FieldSet<TRest...>,
                TIdentifier
            >::Type
        >;

    };

    /// Maps one sink-capacity failure into encoder resource failure and records output position.
    ///
    /// @tparam TSink JSON sink Type.
    /// @param sink Sink whose current position is diagnostic context.
    /// @param writeResult Result returned by the sink operation.
    /// @param diagnostic Diagnostic payload to populate on failure.
    /// @return Succeeded or ResourceLimitExceeded.
    template<class TSink>
    JsonEncodingStatus MapSinkWriteResult(
        const TSink& sink,
        JsonSinkWriteResult writeResult,
        Diagnostic& diagnostic
    ) noexcept {
        if (writeResult == JsonSinkWriteResult::Succeeded) {
            return JsonEncodingStatus::Succeeded;
        }

        diagnostic.ByteOffset = sink.Size();
        return JsonEncodingStatus::ResourceLimitExceeded;
    }

    /// Appends one byte to a JSON sink with common diagnostic handling.
    ///
    /// @tparam TSink JSON sink Type.
    /// @param sink Destination sink.
    /// @param value Encoded byte to append.
    /// @param diagnostic Diagnostic payload to populate on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink>
    JsonEncodingStatus WriteJsonByte(
        TSink& sink,
        std::uint8_t value,
        Diagnostic& diagnostic
    ) noexcept {
        return MapSinkWriteResult(
            sink,
            sink.WriteByte(value),
            diagnostic
        );
    }

    /// Appends one character range to a JSON sink with common diagnostic handling.
    ///
    /// @tparam TSink JSON sink Type.
    /// @param sink Destination sink.
    /// @param source First encoded character.
    /// @param length Number of encoded characters.
    /// @param diagnostic Diagnostic payload to populate on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink>
    JsonEncodingStatus WriteJsonBytes(
        TSink& sink,
        const char* source,
        std::size_t length,
        Diagnostic& diagnostic
    ) noexcept {
        return MapSinkWriteResult(
            sink,
            sink.WriteBytes(
                source,
                length
            ),
            diagnostic
        );
    }

    /// Validates one complete bounded String payload against the V1 UTF-8 domain.
    ///
    /// Rejects malformed continuation structure, overlong sequences, UTF-16 surrogate values,
    /// code points above U+10FFFF, incomplete sequences, and U+0000.
    ///
    /// @param source First payload byte.
    /// @param length Number of payload bytes.
    /// @return Strong validation result carrying the first invalid source offset.
    inline Utf8ValidationResult ValidateUtf8(
        const char* source,
        std::size_t length
    ) noexcept {
        std::size_t index = 0U;

        while (index < length) {
            const auto first = static_cast<std::uint8_t>(source[index]);

            if (first == 0U) {
                return {
                    Utf8ValidationStatus::InvalidUtf8,
                    index
                };
            }

            if (first <= 0x7FU) {
                ++index;
                continue;
            }

            if (first >= 0xC2U && first <= 0xDFU) {
                if (index + 1U >= length) {
                    return {Utf8ValidationStatus::InvalidUtf8, index};
                }

                const auto second = static_cast<std::uint8_t>(source[index + 1U]);
                if (second < 0x80U || second > 0xBFU) {
                    return {Utf8ValidationStatus::InvalidUtf8, index};
                }

                index += 2U;
                continue;
            }

            if (first >= 0xE0U && first <= 0xEFU) {
                if (index + 2U >= length) {
                    return {Utf8ValidationStatus::InvalidUtf8, index};
                }

                const auto second = static_cast<std::uint8_t>(source[index + 1U]);
                const auto third = static_cast<std::uint8_t>(source[index + 2U]);
                const bool secondValid =
                    first == 0xE0U
                        ? second >= 0xA0U && second <= 0xBFU
                        : first == 0xEDU
                            ? second >= 0x80U && second <= 0x9FU
                            : second >= 0x80U && second <= 0xBFU;

                if (!secondValid || third < 0x80U || third > 0xBFU) {
                    return {Utf8ValidationStatus::InvalidUtf8, index};
                }

                index += 3U;
                continue;
            }

            if (first >= 0xF0U && first <= 0xF4U) {
                if (index + 3U >= length) {
                    return {Utf8ValidationStatus::InvalidUtf8, index};
                }

                const auto second = static_cast<std::uint8_t>(source[index + 1U]);
                const auto third = static_cast<std::uint8_t>(source[index + 2U]);
                const auto fourth = static_cast<std::uint8_t>(source[index + 3U]);
                const bool secondValid =
                    first == 0xF0U
                        ? second >= 0x90U && second <= 0xBFU
                        : first == 0xF4U
                            ? second >= 0x80U && second <= 0x8FU
                            : second >= 0x80U && second <= 0xBFU;

                if (
                    !secondValid ||
                    third < 0x80U || third > 0xBFU ||
                    fourth < 0x80U || fourth > 0xBFU
                ) {
                    return {Utf8ValidationStatus::InvalidUtf8, index};
                }

                index += 4U;
                continue;
            }

            return {
                Utf8ValidationStatus::InvalidUtf8,
                index
            };
        }

        return {};
    }

    /// Emits one canonical JSON String from already bounded source text.
    ///
    /// Printable/native UTF-8 is emitted directly. JSON syntax bytes and ASCII controls use
    /// deterministic short escapes where defined, otherwise lowercase `\\u00xx` escapes.
    ///
    /// @tparam TSink JSON sink Type.
    /// @param sink Destination sink.
    /// @param source First UTF-8 payload byte.
    /// @param length Number of UTF-8 payload bytes.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink>
    JsonEncodingStatus EncodeJsonString(
        TSink& sink,
        const char* source,
        std::size_t length,
        Diagnostic& diagnostic
    ) noexcept {
        const auto validation = ValidateUtf8(
            source,
            length
        );

        if (!validation.IsSuccessful()) {
            diagnostic.ByteOffset = validation.ByteOffset;
            return JsonEncodingStatus::InvalidUtf8;
        }

        auto status = WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('"'),
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        constexpr char HexDigits[] = "0123456789abcdef";

        for (std::size_t index = 0U; index < length; ++index) {
            const auto byte = static_cast<std::uint8_t>(source[index]);
            const char* escape = nullptr;

            switch (byte) {
                case static_cast<std::uint8_t>('"'):
                    escape = "\\\"";
                    break;
                case static_cast<std::uint8_t>('\\'):
                    escape = "\\\\";
                    break;
                case 0x08U:
                    escape = "\\b";
                    break;
                case 0x09U:
                    escape = "\\t";
                    break;
                case 0x0AU:
                    escape = "\\n";
                    break;
                case 0x0CU:
                    escape = "\\f";
                    break;
                case 0x0DU:
                    escape = "\\r";
                    break;
                default:
                    break;
            }

            if (escape != nullptr) {
                status = WriteJsonBytes(
                    sink,
                    escape,
                    2U,
                    diagnostic
                );
            } else if (byte < 0x20U) {
                const char unicodeEscape[] = {
                    '\\',
                    'u',
                    '0',
                    '0',
                    HexDigits[(byte >> 4U) & 0x0FU],
                    HexDigits[byte & 0x0FU]
                };
                status = WriteJsonBytes(
                    sink,
                    unicodeEscape,
                    sizeof(unicodeEscape),
                    diagnostic
                );
            } else {
                status = WriteJsonByte(
                    sink,
                    byte,
                    diagnostic
                );
            }

            if (status != JsonEncodingStatus::Succeeded) { return status; }
        }

        return WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('"'),
            diagnostic
        );
    }

    /// Emits one canonical JSON integer token using base-10 text.
    ///
    /// @tparam TSink JSON sink Type.
    /// @tparam TValue Fixed-width integer Type.
    /// @param sink Destination sink.
    /// @param value Integer value to encode.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink, class TValue>
    JsonEncodingStatus EncodeJsonInteger(
        TSink& sink,
        TValue value,
        Diagnostic& diagnostic
    ) noexcept {
        std::array<char, 32U> buffer{};

        if constexpr (std::is_signed_v<TValue>) {
            const auto conversion = std::to_chars(
                buffer.data(),
                buffer.data() + buffer.size(),
                static_cast<std::int64_t>(value)
            );

            if (conversion.ec != std::errc{}) {
                diagnostic.ByteOffset = sink.Size();
                return JsonEncodingStatus::ResourceLimitExceeded;
            }

            return WriteJsonBytes(
                sink,
                buffer.data(),
                static_cast<std::size_t>(conversion.ptr - buffer.data()),
                diagnostic
            );
        } else {
            const auto conversion = std::to_chars(
                buffer.data(),
                buffer.data() + buffer.size(),
                static_cast<std::uint64_t>(value)
            );

            if (conversion.ec != std::errc{}) {
                diagnostic.ByteOffset = sink.Size();
                return JsonEncodingStatus::ResourceLimitExceeded;
            }

            return WriteJsonBytes(
                sink,
                buffer.data(),
                static_cast<std::size_t>(conversion.ptr - buffer.data()),
                diagnostic
            );
        }
    }

    /// Emits one deterministic shortest round-tripping JSON floating token.
    ///
    /// @tparam TSink JSON sink Type.
    /// @tparam TValue Supported binary32 or binary64 Type.
    /// @param sink Destination sink.
    /// @param value Finite floating value to encode.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink, class TValue>
    JsonEncodingStatus EncodeJsonFloating(
        TSink& sink,
        TValue value,
        Diagnostic& diagnostic
    ) noexcept {
        if (!std::isfinite(value)) { return JsonEncodingStatus::NonFiniteNumber; }

        if (value == static_cast<TValue>(0)) {
            const char* spelling = std::signbit(value)
                ? "-0"
                : "0";
            const std::size_t length = std::signbit(value)
                ? 2U
                : 1U;

            return WriteJsonBytes(
                sink,
                spelling,
                length,
                diagnostic
            );
        }

        std::array<char, 64U> buffer{};
        const auto conversion = std::to_chars(
            buffer.data(),
            buffer.data() + buffer.size(),
            value
        );

        if (conversion.ec != std::errc{}) {
            diagnostic.ByteOffset = sink.Size();
            return JsonEncodingStatus::ResourceLimitExceeded;
        }

        return WriteJsonBytes(
            sink,
            buffer.data(),
            static_cast<std::size_t>(conversion.ptr - buffer.data()),
            diagnostic
        );
    }

    /// Emits one canonical padded RFC 4648 Base64 JSON String from arbitrary octets.
    ///
    /// @tparam TSink JSON sink Type.
    /// @param sink Destination sink.
    /// @param source First source octet.
    /// @param length Number of source octets.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink>
    JsonEncodingStatus EncodeJsonBase64(
        TSink& sink,
        const std::uint8_t* source,
        std::size_t length,
        Diagnostic& diagnostic
    ) noexcept {
        constexpr char Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        auto status = WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('"'),
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        std::size_t index = 0U;
        while (index + 3U <= length) {
            const auto first = source[index];
            const auto second = source[index + 1U];
            const auto third = source[index + 2U];
            const char encoded[] = {
                Alphabet[first >> 2U],
                Alphabet[((first & 0x03U) << 4U) | (second >> 4U)],
                Alphabet[((second & 0x0FU) << 2U) | (third >> 6U)],
                Alphabet[third & 0x3FU]
            };

            status = WriteJsonBytes(
                sink,
                encoded,
                sizeof(encoded),
                diagnostic
            );
            if (status != JsonEncodingStatus::Succeeded) { return status; }

            index += 3U;
        }

        const auto remaining = length - index;
        if (remaining == 1U) {
            const auto first = source[index];
            const char encoded[] = {
                Alphabet[first >> 2U],
                Alphabet[(first & 0x03U) << 4U],
                '=',
                '='
            };
            status = WriteJsonBytes(
                sink,
                encoded,
                sizeof(encoded),
                diagnostic
            );
        } else if (remaining == 2U) {
            const auto first = source[index];
            const auto second = source[index + 1U];
            const char encoded[] = {
                Alphabet[first >> 2U],
                Alphabet[((first & 0x03U) << 4U) | (second >> 4U)],
                Alphabet[(second & 0x0FU) << 2U],
                '='
            };
            status = WriteJsonBytes(
                sink,
                encoded,
                sizeof(encoded),
                diagnostic
            );
        }

        if (status != JsonEncodingStatus::Succeeded) { return status; }

        return WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('"'),
            diagnostic
        );
    }

    /// Emits one canonical decimal JSON object key for a numeric FieldIdentifier.
    ///
    /// @tparam TSink JSON sink Type.
    /// @param sink Destination sink.
    /// @param identifier Stable one-byte Field identity.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink>
    JsonEncodingStatus EncodeJsonFieldKey(
        TSink& sink,
        System::FieldIdentifier identifier,
        Diagnostic& diagnostic
    ) noexcept {
        auto status = WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('"'),
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        status = EncodeJsonInteger(
            sink,
            identifier.Value(),
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        constexpr char Suffix[] = "\":";
        return WriteJsonBytes(
            sink,
            Suffix,
            sizeof(Suffix) - 1U,
            diagnostic
        );
    }

    /// Forward declaration for recursive JSON value encoding.
    ///
    /// @tparam TSink JSON sink Type.
    /// @tparam TValue Serialisable value Type.
    /// @param sink Destination sink.
    /// @param value Source value.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink, class TValue>
    JsonEncodingStatus EncodeJsonValue(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept;

    /// Emits one JSON array from a finite indexed source.
    ///
    /// @tparam TSink JSON sink Type.
    /// @tparam TAccessor Callable returning the source value at one valid index.
    /// @param sink Destination sink.
    /// @param count Exact number of source values.
    /// @param accessor Indexed source accessor.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink, class TAccessor>
    JsonEncodingStatus EncodeJsonArray(
        TSink& sink,
        std::size_t count,
        TAccessor&& accessor,
        Diagnostic& diagnostic
    ) noexcept {
        auto status = WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('['),
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        for (std::size_t index = 0U; index < count; ++index) {
            if (index != 0U) {
                status = WriteJsonByte(
                    sink,
                    static_cast<std::uint8_t>(','),
                    diagnostic
                );
                if (status != JsonEncodingStatus::Succeeded) { return status; }
            }

            status = EncodeJsonValue(
                sink,
                accessor(index),
                diagnostic
            );
            if (status != JsonEncodingStatus::Succeeded) { return status; }
        }

        return WriteJsonByte(
            sink,
            static_cast<std::uint8_t>(']'),
            diagnostic
        );
    }

    /// Emits schema Fields in ascending numeric FieldIdentifier order without runtime sorting state.
    ///
    /// @tparam TIdentifier Current numeric FieldIdentifier candidate in the compile-time traversal.
    /// @tparam TSink JSON sink Type.
    /// @tparam TValue Schema owner Type.
    /// @param sink Destination sink.
    /// @param value Source schema object.
    /// @param firstField Tracks whether a comma is required before the next emitted Field.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<std::uint16_t TIdentifier, class TSink, class TValue>
    JsonEncodingStatus EncodeJsonSchemaFields(
        TSink& sink,
        const TValue& value,
        bool& firstField,
        Diagnostic& diagnostic
    ) noexcept {
        if constexpr (TIdentifier > 255U) {
            static_cast<void>(sink);
            static_cast<void>(value);
            static_cast<void>(firstField);
            static_cast<void>(diagnostic);
            return JsonEncodingStatus::Succeeded;
        } else {
            using Field = typename FieldBindingForIdentifier<
                System::FieldsOf<TValue>,
                static_cast<System::FieldIdentifier::Storage>(TIdentifier)
            >::Type;

            if constexpr (!std::is_void_v<Field>) {
                using FieldValue = std::remove_cv_t<System::FieldValueOf<Field>>;
                const auto& fieldValue = value.*Field::Member;
                bool emitField = true;

                if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                    emitField = fieldValue.has_value();
                }

                if (emitField) {
                    auto status = JsonEncodingStatus::Succeeded;

                    if (!firstField) {
                        status = WriteJsonByte(
                            sink,
                            static_cast<std::uint8_t>(','),
                            diagnostic
                        );
                    }

                    if (status == JsonEncodingStatus::Succeeded) {
                        status = EncodeJsonFieldKey(
                            sink,
                            Field::Identifier,
                            diagnostic
                        );
                    }

                    if (status == JsonEncodingStatus::Succeeded) {
                        if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                            status = EncodeJsonValue(
                                sink,
                                fieldValue.value(),
                                diagnostic
                            );
                        } else {
                            status = EncodeJsonValue(
                                sink,
                                fieldValue,
                                diagnostic
                            );
                        }
                    }

                    if (status != JsonEncodingStatus::Succeeded) {
                        if (!diagnostic.Type.has_value()) {
                            diagnostic.Type = TValue::Identifier;
                        }
                        if (!diagnostic.Field.has_value()) {
                            diagnostic.Field = Field::Identifier;
                        }
                        return status;
                    }

                    firstField = false;
                }
            }

            return EncodeJsonSchemaFields<TIdentifier + 1U>(
                sink,
                value,
                firstField,
                diagnostic
            );
        }
    }

    /// Emits one schema object in canonical ascending numeric FieldIdentifier order.
    ///
    /// @tparam TSink JSON sink Type.
    /// @tparam TValue Schema owner Type.
    /// @param sink Destination sink.
    /// @param value Source schema object.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink, class TValue>
    JsonEncodingStatus EncodeJsonSchema(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        auto status = WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('{'),
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        bool firstField = true;
        status = EncodeJsonSchemaFields<0U>(
            sink,
            value,
            firstField,
            diagnostic
        );
        if (status != JsonEncodingStatus::Succeeded) { return status; }

        return WriteJsonByte(
            sink,
            static_cast<std::uint8_t>('}'),
            diagnostic
        );
    }

    /// Emits one value from the complete currently-qualified V1 Type universe as canonical JSON.
    ///
    /// @tparam TSink JSON sink Type.
    /// @tparam TValue Serialisable source Type.
    /// @param sink Destination sink.
    /// @param value Source value.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal encoding outcome.
    template<class TSink, class TValue>
    JsonEncodingStatus EncodeJsonValue(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(
            IsSerialisableType<Value>,
            "JSON encoding requires a SerialisableType source value"
        );

        if constexpr (std::is_same_v<Value, bool>) {
            constexpr char TrueLiteral[] = "true";
            constexpr char FalseLiteral[] = "false";
            return value
                ? WriteJsonBytes(
                    sink,
                    TrueLiteral,
                    sizeof(TrueLiteral) - 1U,
                    diagnostic
                )
                : WriteJsonBytes(
                    sink,
                    FalseLiteral,
                    sizeof(FalseLiteral) - 1U,
                    diagnostic
                );
        } else if constexpr (IsFixedWidthInteger<Value>) {
            return EncodeJsonInteger(
                sink,
                value,
                diagnostic
            );
        } else if constexpr (IsSupportedFloatingPoint<Value>) {
            return EncodeJsonFloating(
                sink,
                value,
                diagnostic
            );
        } else if constexpr (std::is_enum_v<Value>) {
            using Underlying = typename EnumSerialisationTraits<Value>::UnderlyingType;
            return EncodeJsonInteger(
                sink,
                static_cast<Underlying>(value),
                diagnostic
            );
        } else if constexpr (OptionalValueTraits<Value>::IsValue) {
            if (!value.has_value()) {
                constexpr char NullLiteral[] = "null";
                return WriteJsonBytes(
                    sink,
                    NullLiteral,
                    sizeof(NullLiteral) - 1U,
                    diagnostic
                );
            }

            return EncodeJsonValue(
                sink,
                value.value(),
                diagnostic
            );
        } else if constexpr (StandardArrayTraits<Value>::IsValue) {
            return EncodeJsonArray(
                sink,
                StandardArrayTraits<Value>::Count,
                [&](std::size_t index) -> const auto& {
                    return value[index];
                },
                diagnostic
            );
        } else if constexpr (std::is_array_v<Value>) {
            return EncodeJsonArray(
                sink,
                std::extent_v<Value>,
                [&](std::size_t index) -> const auto& {
                    return value[index];
                },
                diagnostic
            );
        } else if constexpr (BoundedStringTraits<Value>::IsValue) {
            return EncodeJsonString(
                sink,
                value.Data(),
                value.Size(),
                diagnostic
            );
        } else if constexpr (BoundedBytesTraits<Value>::IsValue) {
            return EncodeJsonBase64(
                sink,
                value.Data(),
                value.Size(),
                diagnostic
            );
        } else if constexpr (BoundedVectorTraits<Value>::IsValue) {
            return EncodeJsonArray(
                sink,
                value.Size(),
                [&](std::size_t index) -> const auto& {
                    return value[index];
                },
                diagnostic
            );
        } else if constexpr (PotentialSchemaType<Value>) {
            static_assert(
                System::SchemaType<Value>,
                "Serialisable schema values must satisfy System::SchemaType"
            );
            return EncodeJsonSchema(
                sink,
                value,
                diagnostic
            );
        } else if constexpr (HasCanonicalRepresentation<Value>) {
            using Representation = std::remove_cv_t<typename CanonicalRepresentation<Value>::Type>;
            Representation representation{};
            using Adapter = Bounded::TypeConversionAdapter<
                Value,
                Representation
            >;
            const auto conversionResult = Adapter::Convert(
                value,
                representation
            );

            if (!Bounded::IsTypeConversionSuccessful<Value, Representation>(conversionResult)) {
                return JsonEncodingStatus::AdaptationFailed;
            }

            return EncodeJsonValue(
                sink,
                representation,
                diagnostic
            );
        } else {
            static_assert(
                IsSerialisableType<Value>,
                "SerialisableType classification and JSON encoding dispatch are inconsistent"
            );
            return JsonEncodingStatus::ResourceLimitExceeded;
        }
    }

} // ESPressio::Serialisation::Detail
