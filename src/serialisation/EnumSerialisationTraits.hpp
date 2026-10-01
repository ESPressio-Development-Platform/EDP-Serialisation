#pragma once

namespace ESPressio::Serialisation {

    /// Explicitly certifies the stable fixed-width numeric representation of one enum Type.
    ///
    /// The primary declaration is intentionally incomplete. A specialization supplies only
    /// `UnderlyingType`; named-enumerator membership remains application-domain validation.
    ///
    /// @tparam TEnum Enum Type being certified.
    template<class TEnum>
    struct EnumSerialisationTraits;

} // ESPressio::Serialisation
