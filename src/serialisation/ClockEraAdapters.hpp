#pragma once

#include <cstdint>

#include <clock/ClockEra.hpp>
#include <ESPressio_BoundedTypes.hpp>

#include "CanonicalRepresentation.hpp"

namespace ESPressio::Serialisation {

    /// Canonical opaque Clock Era carriage. Era allocation, provenance and
    /// continuity are exclusively owned by the upstream Clock coordinator.
    ///
    /// This header is opt-in and introduces the conditional direct dependency
    /// EDP-Serialisation -> EDP-Clock (never the reverse).
    template<>
    struct CanonicalRepresentation<ESPressio::Clock::ClockEra> final {
        using Type = std::uint64_t;
    };

    /// The adapter is reversible for all 64-bit values, including the invalid
    /// zero sentinel. This outcome does NOT certify upstream era authority.
    enum class ClockEraAdaptationResult : std::uint8_t {
        Succeeded = 0U
    };

} // namespace ESPressio::Serialisation

namespace ESPressio::Bounded {

    template<>
    struct TypeConversionAdapter<ESPressio::Clock::ClockEra, std::uint64_t> final {
        using ResultType = ESPressio::Serialisation::ClockEraAdaptationResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;

        static constexpr bool IsSuccessful(ResultType status) noexcept {
            return status == ResultType::Succeeded;
        }

        static constexpr ResultType Convert(
            const ESPressio::Clock::ClockEra& value,
            std::uint64_t& representation
        ) noexcept {
            representation = value.Value();
            return ResultType::Succeeded;
        }
    };

    template<>
    struct TypeConversionAdapter<std::uint64_t, ESPressio::Clock::ClockEra> final {
        using ResultType = ESPressio::Serialisation::ClockEraAdaptationResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;

        static constexpr bool IsSuccessful(ResultType status) noexcept {
            return status == ResultType::Succeeded;
        }

        static constexpr ResultType Convert(
            const std::uint64_t& representation,
            ESPressio::Clock::ClockEra& value
        ) noexcept {
            value = ESPressio::Clock::ClockEra::FromValue(representation);
            return ResultType::Succeeded;
        }
    };

} // namespace ESPressio::Bounded
