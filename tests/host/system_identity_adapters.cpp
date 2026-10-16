#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include <ESPressio_Serialisation.hpp>

namespace S = ESPressio::Serialisation;
namespace I = ESPressio::System::Identity;
namespace B = ESPressio::Bounded;

using DeviceBytes = B::Bytes<I::DeviceIdentifier::Size>;
static_assert(S::SerialisableType<I::DeviceIdentifier>);
static_assert(S::SerialisableType<I::RuntimeIncarnationId>);
static_assert(B::IsTypeConversionAvailable<I::DeviceIdentifier, DeviceBytes>);
static_assert(B::IsTypeConversionAvailable<DeviceBytes, I::DeviceIdentifier>);
static_assert(B::IsTypeConversionAvailable<I::RuntimeIncarnationId,std::uint32_t>);
static_assert(B::IsTypeConversionAvailable<std::uint32_t,I::RuntimeIncarnationId>);

int main() {
    constexpr I::DeviceIdentifier::Storage deviceStorage{
        0U,1U,2U,3U,4U,5U,6U,7U,8U,9U,10U,11U,12U,13U,14U,15U
    };
    const I::DeviceIdentifier device{deviceStorage};
    const I::RuntimeIncarnationId incarnation{0x01020304U};
    assert(device.IsValid() && incarnation.IsValid());

    DeviceBytes raw;
    assert((B::TypeConversionAdapter<I::DeviceIdentifier,DeviceBytes>::Convert(
        device,raw) == S::SystemIdentityAdaptationResult::Succeeded));
    assert(raw.Size() == 16U);
    for(std::size_t i=0;i<16U;++i) assert(raw[i] == deviceStorage[i]);
    I::DeviceIdentifier restored;
    assert((B::TypeConversionAdapter<DeviceBytes,I::DeviceIdentifier>::Convert(
        raw,restored)==S::SystemIdentityAdaptationResult::Succeeded));
    assert(restored==device);

    const auto shortRaw = DeviceBytes{};
    const auto unchanged = restored;
    assert((B::TypeConversionAdapter<DeviceBytes,I::DeviceIdentifier>::Convert(
        shortRaw,restored)==S::SystemIdentityAdaptationResult::IncorrectByteLength));
    assert(restored==unchanged);
    // Exactly 16 bytes in CBOR byte-string, not sixteen JSON/CBOR integers.
    std::array<std::byte,100U> cbor{};
    cbor.fill(std::byte{0xA5U});
    const auto encoded=S::SerialisePositionalCbor<2U>(
        std::span<std::byte>(cbor),device,incarnation);
    assert(encoded.Status==S::SerialisationStatus::Succeeded);
    assert(cbor[0U]==std::byte{0x82U});
    assert(cbor[1U]==std::byte{0x50U});
    for(std::size_t i=0U;i<16U;++i) {
        assert(cbor[2U+i]==std::byte{static_cast<std::uint8_t>(i)});
    }
    assert(cbor[18U]==std::byte{0x1AU});
    assert(cbor[19U]==std::byte{0x01U});
    assert(cbor[20U]==std::byte{0x02U});
    assert(cbor[21U]==std::byte{0x03U});
    assert(cbor[22U]==std::byte{0x04U});
    // correct expected: 1+1+16+5 = 23 bytes.
    assert(encoded.BytesWritten==23U);
    std::array<std::byte,3U> small{
        std::byte{0xEFU},std::byte{0xEFU},std::byte{0xEFU}
    };
    const auto denied=S::SerialisePositionalCbor<2U>(
        std::span<std::byte>(small),device,incarnation
    );
    assert(denied.Status==S::SerialisationStatus::OutputBufferTooSmall);
    assert(denied.BytesWritten==0U);
    for (auto v:small) assert(v==std::byte{0xEFU});

    std::uint32_t scalar{};
    assert((B::TypeConversionAdapter<I::RuntimeIncarnationId,std::uint32_t>::Convert(
        incarnation,scalar)==S::SystemIdentityAdaptationResult::Succeeded));
    assert(scalar==0x01020304U);
    I::RuntimeIncarnationId restoredIncarnation;
    assert((B::TypeConversionAdapter<std::uint32_t,I::RuntimeIncarnationId>::Convert(
        scalar,restoredIncarnation)==S::SystemIdentityAdaptationResult::Succeeded));
    assert(restoredIncarnation==incarnation);

    const I::DeviceIdentifier zeroDevice;
    DeviceBytes zeroBytes;
    assert((B::TypeConversionAdapter<I::DeviceIdentifier,DeviceBytes>::Convert(
        zeroDevice,zeroBytes)==S::SystemIdentityAdaptationResult::Succeeded));
    I::DeviceIdentifier reversedZero;
    assert((B::TypeConversionAdapter<DeviceBytes,I::DeviceIdentifier>::Convert(
        zeroBytes,reversedZero)==S::SystemIdentityAdaptationResult::Succeeded));
    assert(!reversedZero.IsValid());
    // Invalid-zero enforcement belongs to record semantics, not reversible codec.
    const I::RuntimeIncarnationId zeroIncarnation;
    assert((B::TypeConversionAdapter<I::RuntimeIncarnationId,std::uint32_t>::Convert(
        zeroIncarnation,scalar)==S::SystemIdentityAdaptationResult::Succeeded));
    assert(scalar==0U);

    std::array<std::byte,100U> json{};
    const auto jsonEncoded=S::SerialisePositionalJson<2U>(
        std::span<std::byte>(json),device,incarnation);
    assert(jsonEncoded.Status==S::SerialisationStatus::Succeeded);
    const std::string_view output{
        reinterpret_cast<const char*>(json.data()),jsonEncoded.BytesWritten
    };
    // Existing EDP-Serialisation Base64 JSON policy, not bespoke hex.
    assert(output == "[\"AAECAwQFBgcICQoLDA0ODw==\",16909060]");
}
