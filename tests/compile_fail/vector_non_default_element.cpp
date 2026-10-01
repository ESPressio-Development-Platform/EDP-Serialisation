#include <cstdint>

#include <ESPressio_Serialisation.hpp>

/// Memory-bounded schema value which deliberately cannot be default-constructed.
struct NonDefaultVectorElement final {

    // Schema payload.

    /// Fixed-width payload retained by the fixture.
    std::uint16_t Value;

    // Schema metadata.

    /// Stable semantic Type identity owned by the negative fixture.
    inline static constexpr ESPressio::System::TypeIdentifier Identifier{
        ESPressio::System::TypeIdentifier::Storage{0x00U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U}
    };

    /// Canonical schema binding for the fixed-width payload.
    using Fields = ESPressio::System::FieldSet<
        ESPressio::System::FieldBinding<&NonDefaultVectorElement::Value, 1U>
    >;

    // Construction.

    /// Prevents transactional decode from activating a new bounded Vector slot generically.
    NonDefaultVectorElement() = delete;

    /// Creates an explicit existing semantic value.
    constexpr explicit NonDefaultVectorElement(
        std::uint16_t value
    ) noexcept :
        Value(value) {
    }

};

namespace ESPressio::Bounded {

    /// Certifies the negative fixture's complete owned state as memory-bounded.
    template<>
    struct MemoryBoundedTraits<NonDefaultVectorElement> final : MemoryBoundedValueDeclaration<
        false,
        std::uint16_t
    > {
    };

} // ESPressio::Bounded

static_assert(
    ESPressio::Serialisation::SerialisableType<
        ESPressio::Bounded::Vector<NonDefaultVectorElement, 2U>
    >,
    "Vector elements which cannot be nothrow default-constructed must remain outside V1"
);

/// Compile-fail harness entry point; the preceding assertion must reject the fixture first.
int main() {
    return 0;
}
