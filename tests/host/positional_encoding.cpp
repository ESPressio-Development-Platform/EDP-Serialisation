#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>

#include <ESPressio_Serialisation.hpp>

namespace Serialisation = ESPressio::Serialisation;

int main() {
    constexpr std::array<std::uint8_t, 26U> expected{
        0x85U, 0xF5U, 0x21U,
        0x1BU, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
        0xFFU, 0xFFU, 0xFFU, 0xFFU,
        0xFAU, 0x3FU, 0xC0U, 0x00U, 0x00U,
        0xFBU, 0x40U, 0x0AU, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U
    };

    const auto measured = Serialisation::MeasurePositionalCbor<5U>(
        true, static_cast<std::int8_t>(-2),
        std::numeric_limits<std::uint64_t>::max(),
        1.5f, 3.25
    );
    assert(measured.Status == Serialisation::MeasurementStatus::Succeeded);
    assert(measured.RequiredBytes == expected.size());

    std::array<std::byte, 26U> output{};
    output.fill(std::byte{0xBD});
    const auto result = Serialisation::SerialisePositionalCbor<5U>(
        std::span<std::byte>(output),
        true, static_cast<std::int8_t>(-2),
        std::numeric_limits<std::uint64_t>::max(),
        1.5f, 3.25
    );
    assert(result.Status == Serialisation::SerialisationStatus::Succeeded);
    assert(result.RequiredBytes == expected.size());
    assert(result.BytesWritten == expected.size());
    for (std::size_t i = 0U; i < output.size(); ++i) {
        assert(std::to_integer<std::uint8_t>(output[i]) == expected[i]);
    }

    // Preflight failure is transactional: never mutates caller storage.
    std::array<std::byte, 2U> small{};
    small.fill(std::byte{0xBB});
    const auto tooSmall = Serialisation::SerialisePositionalCbor<5U>(
        std::span<std::byte>(small),
        true, static_cast<std::int8_t>(-2),
        std::numeric_limits<std::uint64_t>::max(),
        1.5f, 3.25
    );
    assert(tooSmall.Status == Serialisation::SerialisationStatus::OutputBufferTooSmall);
    assert(tooSmall.RequiredBytes == expected.size());
    assert(tooSmall.BytesWritten == 0U);
    assert(small[0] == std::byte{0xBB} && small[1] == std::byte{0xBB});

    auto outputOnNonfinite = output;
    outputOnNonfinite.fill(std::byte{0xAA});
    const auto nonfinite = Serialisation::SerialisePositionalCbor<5U>(
        std::span<std::byte>(outputOnNonfinite),
        std::numeric_limits<double>::infinity()
    );
    assert(nonfinite.Status == Serialisation::SerialisationStatus::NonFiniteNumber);
    assert(nonfinite.BytesWritten == 0U);
    for (const auto byte : outputOnNonfinite) assert(byte == std::byte{0xAA});

    std::array<std::byte, 1U> empty{};
    const auto emptyResult = Serialisation::SerialisePositionalCbor<0U>(
        std::span<std::byte>(empty)
    );
    assert(emptyResult.Status == Serialisation::SerialisationStatus::Succeeded);
    assert(emptyResult.BytesWritten == 1U);
    assert(empty[0] == std::byte{0x80});

    constexpr const char* jsonExpected =
        "[true,-2,18446744073709551615,1.5,3.25]";
    std::array<std::byte, 128U> jsonBuffer{};
    const auto jsonMeasurement = Serialisation::MeasurePositionalJson<5U>(
        true, static_cast<std::int8_t>(-2),
        std::numeric_limits<std::uint64_t>::max(),
        1.5f, 3.25
    );
    assert(jsonMeasurement.Status == Serialisation::MeasurementStatus::Succeeded);
    assert(jsonMeasurement.RequiredBytes == std::char_traits<char>::length(jsonExpected));

    const auto jsonResult = Serialisation::SerialisePositionalJson<5U>(
        std::span<std::byte>(jsonBuffer),
        true, static_cast<std::int8_t>(-2),
        std::numeric_limits<std::uint64_t>::max(),
        1.5f, 3.25
    );
    assert(jsonResult.Status == Serialisation::SerialisationStatus::Succeeded);
    assert(jsonResult.BytesWritten == jsonMeasurement.RequiredBytes);
    const std::string_view jsonView(
        reinterpret_cast<const char*>(jsonBuffer.data()), jsonResult.BytesWritten
    );
    assert(jsonView == jsonExpected);

    std::array<std::byte, 1U> jsonTooSmall{std::byte{0xD1}};
    const auto jsonTooSmallResult = Serialisation::SerialisePositionalJson<5U>(
        std::span<std::byte>(jsonTooSmall),
        true, static_cast<std::int8_t>(-2),
        std::numeric_limits<std::uint64_t>::max(),
        1.5f, 3.25
    );
    assert(jsonTooSmallResult.Status == Serialisation::SerialisationStatus::OutputBufferTooSmall);
    assert(jsonTooSmallResult.BytesWritten == 0U);
    assert(jsonTooSmall[0] == std::byte{0xD1});

    const auto invalidJson = Serialisation::SerialisePositionalJson<5U>(
        std::span<std::byte>(jsonBuffer),
        std::numeric_limits<double>::infinity()
    );
    assert(invalidJson.Status == Serialisation::SerialisationStatus::NonFiniteNumber);
    assert(invalidJson.BytesWritten == 0U);

    std::array<std::byte, 2U> emptyJson{};
    const auto emptyJsonResult = Serialisation::SerialisePositionalJson<0U>(
        std::span<std::byte>(emptyJson)
    );
    assert(emptyJsonResult.Status == Serialisation::SerialisationStatus::Succeeded);
    assert(emptyJsonResult.BytesWritten == 2U);
    assert(emptyJson[0] == std::byte{0x5B} && emptyJson[1] == std::byte{0x5D});

}
