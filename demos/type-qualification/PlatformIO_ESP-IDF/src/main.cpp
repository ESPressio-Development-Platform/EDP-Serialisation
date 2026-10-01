#include <cstdint>
#include <cstdio>

#include <ESPressio_Serialisation.hpp>

namespace Demo {

    /// Demonstrates an explicitly certified fixed-width enum.
    enum class Mode : std::uint8_t {
        /// Demo idle state.
        Idle = 0U,
        /// Demo active state.
        Active = 1U
    };

    /// Demonstrates recursive schema qualification with fixed-width and bounded-text Fields.
    struct Reading final {

        // Schema payload.

        /// Example fixed-width voltage Field.
        std::uint16_t Millivolts = 0U;

        /// Example bounded text label Field.
        ESPressio::Bounded::String<16U> Label{};

        // Schema metadata.

        /// Stable semantic Type identity owned by the demo schema.
        inline static constexpr ESPressio::System::TypeIdentifier Identifier{
            ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U}
        };

        /// Canonical FieldSet for the demonstration payload.
        using Fields = ESPressio::System::FieldSet<
            ESPressio::System::FieldBinding<&Reading::Millivolts, 1U>,
            ESPressio::System::FieldBinding<&Reading::Label, 2U>
        >;

    };

} // Demo

namespace ESPressio::Serialisation {

    /// Certifies the demo enum's stable uint8_t numeric representation.
    template<>
    struct EnumSerialisationTraits<Demo::Mode> final {

        // Certification metadata.

        /// Exact fixed-width underlying Type used by the demo enum.
        using UnderlyingType = std::uint8_t;

    };

} // ESPressio::Serialisation

static_assert(ESPressio::Serialisation::SerialisableType<Demo::Mode>);
static_assert(ESPressio::Serialisation::SerialisableType<Demo::Reading>);
static_assert(ESPressio::Serialisation::SerialisableType<ESPressio::System::TypeIdentifier>);
static_assert(ESPressio::Serialisation::DefaultParserLimits::MaximumNestingDepth == 32U);


/// Reports successful compile-time qualification under ESP-IDF.
extern "C" void app_main() {
    std::printf("EDP-Serialisation type qualification demo compiled successfully.\n");
}
