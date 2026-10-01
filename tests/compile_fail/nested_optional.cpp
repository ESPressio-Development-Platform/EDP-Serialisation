#include <cstdint>
#include <optional>

#include <ESPressio_Serialisation.hpp>

static_assert(
    ESPressio::Serialisation::SerialisableType<std::optional<std::optional<std::uint32_t>>>,
    "Nested Optional must remain outside the V1 serialisable universe"
);

/// Compile-fail harness entry point; the preceding assertion must reject the fixture first.
int main() {
    return 0;
}
