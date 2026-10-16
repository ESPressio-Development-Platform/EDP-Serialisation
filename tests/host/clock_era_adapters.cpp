#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include <ESPressio_Serialisation.hpp>
#include <serialisation/ClockEraAdapters.hpp>

using Era = ESPressio::Clock::ClockEra;
namespace S = ESPressio::Serialisation;
namespace B = ESPressio::Bounded;

static_assert(S::SerialisableType<Era>);
static_assert(B::IsTypeConversionAvailable<Era,std::uint64_t>);
static_assert(B::IsTypeConversionAvailable<std::uint64_t,Era>);

int main() {
    const Era era = Era::FromValue(0x0102030405060708ULL);
    assert(era.IsValid());
    std::uint64_t representation=0U;
    assert((B::TypeConversionAdapter<Era,std::uint64_t>::Convert(
        era,representation
    ) == S::ClockEraAdaptationResult::Succeeded));
    assert(representation==0x0102030405060708ULL);
    Era roundtrip;
    assert((B::TypeConversionAdapter<std::uint64_t,Era>::Convert(
        representation,roundtrip
    ) == S::ClockEraAdaptationResult::Succeeded));
    assert(roundtrip==era);

    std::array<std::byte,40U> output{};
    const auto cbor=S::SerialisePositionalCbor<1U>(
        std::span<std::byte>(output),era
    );
    assert(cbor.IsSuccessful());
    constexpr std::array<std::uint8_t,10U> expected{
        0x81U,0x1BU,0x01U,0x02U,0x03U,0x04U,0x05U,0x06U,0x07U,0x08U
    };
    assert(cbor.BytesWritten==expected.size());
    for(std::size_t i=0;i<expected.size();++i) {
        assert(output[i]==std::byte{expected[i]});
    }

    output.fill(std::byte{0xA5U});
    const auto failed=S::SerialisePositionalCbor<1U>(
        std::span<std::byte>(output.data(),3U),era
    );
    assert(!failed.IsSuccessful());
    for (const auto v:output) assert(v==std::byte{0xA5U});

    const auto json=S::SerialisePositionalJson<1U>(
        std::span<std::byte>(output),era
    );
    assert(json.IsSuccessful());
    const std::string_view text{
        reinterpret_cast<const char*>(output.data()),json.BytesWritten
    };
    assert(text=="[72623859790382856]");

    const Era unavailable{};
    std::uint64_t sentinel=42U;
    assert((B::TypeConversionAdapter<Era,std::uint64_t>::Convert(
        unavailable,sentinel
    ) == S::ClockEraAdaptationResult::Succeeded));
    assert(sentinel==0U);
    Era invalid=Era::FromValue(1U);
    assert((B::TypeConversionAdapter<std::uint64_t,Era>::Convert(
        sentinel,invalid
    ) == S::ClockEraAdaptationResult::Succeeded));
    assert(!invalid.IsValid());
    // The conversion does NOT derive an Era or qualify authority/provenance.
}
