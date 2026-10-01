#include <cstdint>

#include <ESPressio_Serialisation.hpp>

int main() {
    const std::uint32_t value = 7U;
    const auto result = ESPressio::Serialisation::Measure<
        ESPressio::Serialisation::Json,
        ESPressio::Serialisation::RootProfile::TypedEnvelope
    >(value);
    static_cast<void>(result);
    return 0;
}
