#pragma once

#include <cstddef>

namespace ESPressio::Serialisation {

    /// Compile-time parser resource limits for one deserialisation invocation.
    ///
    /// @tparam TMaximumNestingDepth Maximum syntactic container nesting accepted by a codec.
    /// @tparam TMaximumSkippedContainerItems Maximum total container items traversed only to
    ///         skip unknown values under IgnoreUnknownFields.
    template<
        std::size_t TMaximumNestingDepth = 32U,
        std::size_t TMaximumSkippedContainerItems = 1024U
    >
    struct ParserLimits final {

        // Resource bounds.

        /// Maximum syntactic nesting depth accepted by the selected codec.
        static constexpr std::size_t MaximumNestingDepth = TMaximumNestingDepth;

        /// Maximum skipped/unknown container items accepted in one operation.
        static constexpr std::size_t MaximumSkippedContainerItems = TMaximumSkippedContainerItems;

    };

    /// Conservative default parser resource policy used unless callers select another policy.
    using DefaultParserLimits = ParserLimits<>;

} // ESPressio::Serialisation
