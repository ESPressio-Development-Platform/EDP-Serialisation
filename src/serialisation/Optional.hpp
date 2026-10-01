#pragma once

#include <optional>

namespace ESPressio::Serialisation {

    /// Canonical ESPressio spelling for an optional serialisable value.
    ///
    /// This is a zero-overhead alias so EDP-BoundedTypes and existing C++ code continue to
    /// recognize the standard optional Type directly.
    ///
    /// @tparam TValue Optional contained value Type.
    template<class TValue>
    using Optional = std::optional<TValue>;

} // ESPressio::Serialisation
