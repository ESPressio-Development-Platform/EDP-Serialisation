#include <cstdint>
#include <ESPressio_Serialisation.hpp>
int main() {
    const auto outcome = ESPressio::Serialisation::MeasurePositionalCbor<1U>(
        std::uint8_t{1U}, std::uint8_t{2U}
    );
    return outcome.IsSuccessful() ? 0 : 1;
}
