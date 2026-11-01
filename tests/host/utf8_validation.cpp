#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <serialisation/Utf8Validation.hpp>

namespace S = ESPressio::Serialisation;
constexpr char Nul[] = {'A', '\0', 'B'};
constexpr char ValidUnicode[] = {char(0xF0), char(0x9F), char(0x98), char(0x80)};
constexpr char Overlong[] = {char(0xC0), char(0xAF)};
constexpr char Truncated[] = {char(0xE2), char(0x82)};
constexpr char Surrogate[] = {char(0xED), char(0xA0), char(0x80)};
constexpr char OutOfRange[] = {char(0xF4), char(0x90), char(0x80), char(0x80)};
static_assert(S::ValidateUtf8("hello", 5U).IsSuccessful());
static_assert(S::ValidateUtf8("", 0U).IsSuccessful());
static_assert(S::ValidateUtf8(ValidUnicode, 4U).IsSuccessful());
static_assert(S::ValidateUtf8(Nul, 3U).Status == S::Utf8ValidationStatus::InvalidUtf8);
static_assert(S::ValidateUtf8(Nul, 3U).ByteOffset == 1U);
static_assert(S::ValidateUtf8(Overlong, 2U).ByteOffset == 0U);
static_assert(S::ValidateUtf8(Truncated, 2U).ByteOffset == 0U);
static_assert(S::ValidateUtf8(Surrogate, 3U).ByteOffset == 0U);
static_assert(S::ValidateUtf8(OutOfRange, 4U).ByteOffset == 0U);
int main() {
    char invalidAfterAscii[] = {'O','K',char(0x80)};
    auto rejected = S::ValidateUtf8(invalidAfterAscii,3U);
    assert(!rejected.IsSuccessful() && rejected.ByteOffset == 2U);
    std::array<char,2U> pair{};
    std::size_t accepted = 0U;
    for (std::size_t first = 0U; first < 256U; ++first) {
        for (std::size_t second = 0U; second < 256U; ++second) {
            pair[0] = char(first);
            pair[1] = char(second);
            accepted += S::ValidateUtf8(pair.data(),pair.size()).IsSuccessful() ? 1U : 0U;
        }
    }
    assert(accepted > 0U && accepted < 65536U);
}
