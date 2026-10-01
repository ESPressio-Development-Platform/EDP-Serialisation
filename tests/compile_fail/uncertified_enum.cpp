#include <cstdint>

#include <ESPressio_Serialisation.hpp>

/// Enum intentionally missing EnumSerialisationTraits certification.
enum class Uncertified : std::uint8_t {
    /// Example source-level enumerator.
    Value = 1U
};

static_assert(
    ESPressio::Serialisation::SerialisableType<Uncertified>,
    "Enums must require explicit EnumSerialisationTraits certification"
);

/// Compile-fail harness entry point; the preceding assertion must reject the fixture first.
int main() {
    return 0;
}
