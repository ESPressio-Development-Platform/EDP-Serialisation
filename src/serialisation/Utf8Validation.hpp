#pragma once

#include <cstddef>
#include <cstdint>

namespace ESPressio::Serialisation {

    /// Strict V1 UTF-8 validation outcomes for all canonical text codecs.
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

    /// Validates one complete bounded String payload against the V1 UTF-8 domain.
    ///
    /// Rejects malformed continuation structure, overlong sequences, UTF-16 surrogate values,
    /// code points above U+10FFFF, incomplete sequences, and U+0000.
    ///
    /// @param source First payload byte.
    /// @param length Number of payload bytes.
    /// @return Strong validation result carrying the first invalid source offset.
    [[nodiscard]] inline constexpr Utf8ValidationResult ValidateUtf8(
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

}
