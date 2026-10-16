#pragma once

#include <cstddef>
#include <cstdint>

#include <ESPressio_BoundedTypes.hpp>
#include <ESPressio_System.hpp>

#include "CanonicalRepresentation.hpp"

namespace ESPressio::Serialisation {

    /// Carries exact System Identity values without inventing their provenance.
    /// A DeviceIdentifier is a 16-byte CBOR byte-string / Base64 JSON string.
    template<>
    struct CanonicalRepresentation<System::Identity::DeviceIdentifier> final {
        using Type = Bounded::Bytes<System::Identity::DeviceIdentifier::Size>;
    };

    /// A durable RuntimeIncarnationId is canonically a uint32 (zero remains
    /// the owner-defined invalid sentinel; the codec does not mint an ID).
    template<>
    struct CanonicalRepresentation<System::Identity::RuntimeIncarnationId> final {
        using Type = std::uint32_t;
    };

    enum class SystemIdentityAdaptationResult : std::uint8_t {
        Succeeded = 0U,
        IncorrectByteLength = 1U,
        ByteOperationsFailure = 2U
    };

} // ESPressio::Serialisation

namespace ESPressio::Bounded {

    template<>
    struct TypeConversionAdapter<
        System::Identity::DeviceIdentifier,
        Bytes<System::Identity::DeviceIdentifier::Size>
    > final {
        using ResultType = Serialisation::SystemIdentityAdaptationResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;
        static constexpr bool IsSuccessful(ResultType status) noexcept {
            return status == ResultType::Succeeded;
        }

        static ResultType Convert(
            const System::Identity::DeviceIdentifier& source,
            Bytes<System::Identity::DeviceIdentifier::Size>& target
        ) noexcept {
            // Exactly one whole-value bounded assignment; never truncate.
            return target.Assign(source.Bytes()) == BytesAssignmentResult::Succeeded
                ? ResultType::Succeeded : ResultType::ByteOperationsFailure;
        }
    };

    template<>
    struct TypeConversionAdapter<
        Bytes<System::Identity::DeviceIdentifier::Size>,
        System::Identity::DeviceIdentifier
    > final {
        using ResultType = Serialisation::SystemIdentityAdaptationResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;
        static constexpr bool IsSuccessful(ResultType status) noexcept {
            return status == ResultType::Succeeded;
        }

        static ResultType Convert(
            const Bytes<System::Identity::DeviceIdentifier::Size>& source,
            System::Identity::DeviceIdentifier& target
        ) noexcept {
            if (source.Size() != System::Identity::DeviceIdentifier::Size) {
                return ResultType::IncorrectByteLength;
            }
            System::Identity::DeviceIdentifier::Storage bytes{};
            for (std::size_t i=0U; i<bytes.size(); ++i) {
                bytes[i] = source[i];
            }
            target = System::Identity::DeviceIdentifier{bytes};
            return ResultType::Succeeded;
        }
    };

    template<>
    struct TypeConversionAdapter<
        System::Identity::RuntimeIncarnationId,
        std::uint32_t
    > final {
        using ResultType = Serialisation::SystemIdentityAdaptationResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;
        static constexpr bool IsSuccessful(ResultType status) noexcept {
            return status == ResultType::Succeeded;
        }
        static constexpr ResultType Convert(
            const System::Identity::RuntimeIncarnationId& source,
            std::uint32_t& target
        ) noexcept {
            target = source.Value();
            return ResultType::Succeeded;
        }
    };

    template<>
    struct TypeConversionAdapter<
        std::uint32_t,
        System::Identity::RuntimeIncarnationId
    > final {
        using ResultType = Serialisation::SystemIdentityAdaptationResult;
        static constexpr bool IsAvailable = true;
        static constexpr bool IsNoexcept = true;
        static constexpr bool IsSuccessful(ResultType status) noexcept {
            return status == ResultType::Succeeded;
        }
        static constexpr ResultType Convert(
            const std::uint32_t& source,
            System::Identity::RuntimeIncarnationId& target
        ) noexcept {
            target = System::Identity::RuntimeIncarnationId{source};
            return ResultType::Succeeded;
        }
    };

} // ESPressio::Bounded
