#include <cstdint>
#include <optional>

#include <ESPressio_Serialisation.hpp>

/// Schema value which is serialisable in-place but cannot be newly default-constructed.
struct NonDefaultOptionalElement final {

    // Schema payload.

    /// Fixed-width payload retained by the fixture.
    std::uint16_t Value;

    // Schema metadata.

    /// Stable semantic Type identity owned by the negative fixture.
    inline static constexpr ESPressio::System::TypeIdentifier Identifier{
        ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U}
    };

    /// Canonical schema binding for the fixed-width payload.
    using Fields = ESPressio::System::FieldSet<
        ESPressio::System::FieldBinding<&NonDefaultOptionalElement::Value, 1U>
    >;

    // Construction.

    /// Prevents transactional decode from engaging an absent Optional without an application constructor.
    NonDefaultOptionalElement() = delete;

    /// Creates an explicit existing semantic value.
    constexpr explicit NonDefaultOptionalElement(
        std::uint16_t value
    ) noexcept :
        Value(value) {
    }

};

static_assert(
    ESPressio::Serialisation::SerialisableType<
        std::optional<NonDefaultOptionalElement>
    >,
    "Optional elements which cannot be nothrow default-constructed must remain outside V1"
);

/// Compile-fail harness entry point; the preceding assertion must reject the fixture first.
int main() {
    return 0;
}
