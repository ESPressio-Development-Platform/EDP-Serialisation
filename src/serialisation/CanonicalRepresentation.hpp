#pragma once

namespace ESPressio::Serialisation {

    /// Declares the one direct serialisable representation of an adapted strong semantic Type.
    ///
    /// The primary declaration is intentionally incomplete. Integrations opt in by supplying
    /// an exact specialization with a nested `Type`; conversion behavior remains owned by
    /// `EDP-BoundedTypes::TypeConversionAdapter` in both directions.
    ///
    /// @tparam TValue Strong semantic Type being adapted.
    template<class TValue>
    struct CanonicalRepresentation;

} // ESPressio::Serialisation
