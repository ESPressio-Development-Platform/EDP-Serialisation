#pragma once

#include <array>
#include <cstdint>

#include <ESPressio_BoundedTypes.hpp>
#include <ESPressio_System.hpp>

#include "CanonicalRepresentation.hpp"

namespace ESPressio::Serialisation {

    /// Result family for lossless canonical System identifier conversions.
    enum class SystemIdentifierConversionResult : std::uint8_t {
        /// Conversion completed losslessly.
        Succeeded = 0U
    };

    /// Selects the canonical exact eight-byte representation of System::TypeIdentifier.
    template<>
    struct CanonicalRepresentation<System::TypeIdentifier> final {

        // Representation metadata.

        /// Exact storage Type preserving canonical TypeIdentifier byte order.
        using Type = System::TypeIdentifier::Storage;

    };

    /// Selects the canonical one-byte representation of System::FieldIdentifier.
    template<>
    struct CanonicalRepresentation<System::FieldIdentifier> final {

        // Representation metadata.

        /// Exact storage Type preserving the complete FieldIdentifier value domain.
        using Type = System::FieldIdentifier::Storage;

    };

    /// Selects the canonical exact three-byte representation of System::TypeAuthorityIdentifier.
    template<>
    struct CanonicalRepresentation<System::TypeAuthorityIdentifier> final {

        // Representation metadata.

        /// Exact storage Type preserving canonical Authority byte order.
        using Type = System::TypeAuthorityIdentifier::Storage;

    };

    /// Selects the canonical exact five-byte representation of System::TypeLocalIdentifier.
    template<>
    struct CanonicalRepresentation<System::TypeLocalIdentifier> final {

        // Representation metadata.

        /// Exact storage Type preserving canonical local-identifier byte order.
        using Type = System::TypeLocalIdentifier::Storage;

    };

} // ESPressio::Serialisation

namespace ESPressio::Bounded {

    /// Converts System::TypeIdentifier to its exact canonical storage representation.
    template<>
    struct TypeConversionAdapter<System::TypeIdentifier, System::TypeIdentifier::Storage> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Copies canonical TypeIdentifier bytes into the caller-owned storage target.
        ///
        /// @param source Semantic TypeIdentifier value being represented.
        /// @param target Caller-owned canonical storage populated on success.
        /// @return Succeeded because every TypeIdentifier has an exact Storage representation.
        static constexpr ResultType Convert(
            const System::TypeIdentifier& source,
            System::TypeIdentifier::Storage& target
        ) noexcept {
            target = source.Bytes();
            return ResultType::Succeeded;
        }

    };

    /// Converts exact TypeIdentifier storage into its semantic System value Type.
    template<>
    struct TypeConversionAdapter<System::TypeIdentifier::Storage, System::TypeIdentifier> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Reconstructs a semantic TypeIdentifier from exact canonical storage.
        ///
        /// @param source Canonical storage being interpreted.
        /// @param target Caller-owned semantic TypeIdentifier populated on success.
        /// @return Succeeded because every Storage value maps exactly to TypeIdentifier.
        static constexpr ResultType Convert(
            const System::TypeIdentifier::Storage& source,
            System::TypeIdentifier& target
        ) noexcept {
            target = System::TypeIdentifier{source};
            return ResultType::Succeeded;
        }

    };

    /// Converts System::FieldIdentifier to its exact one-byte storage representation.
    template<>
    struct TypeConversionAdapter<System::FieldIdentifier, System::FieldIdentifier::Storage> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Copies the complete FieldIdentifier value into caller-owned storage.
        ///
        /// @param source Semantic FieldIdentifier value being represented.
        /// @param target Caller-owned one-byte storage populated on success.
        /// @return Succeeded because every FieldIdentifier value is representable.
        static constexpr ResultType Convert(
            const System::FieldIdentifier& source,
            System::FieldIdentifier::Storage& target
        ) noexcept {
            target = source.Value();
            return ResultType::Succeeded;
        }

    };

    /// Converts exact one-byte FieldIdentifier storage into its semantic System value Type.
    template<>
    struct TypeConversionAdapter<System::FieldIdentifier::Storage, System::FieldIdentifier> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Reconstructs a semantic FieldIdentifier from exact one-byte storage.
        ///
        /// @param source Canonical storage being interpreted.
        /// @param target Caller-owned semantic FieldIdentifier populated on success.
        /// @return Succeeded because every storage byte is a valid FieldIdentifier value.
        static constexpr ResultType Convert(
            const System::FieldIdentifier::Storage& source,
            System::FieldIdentifier& target
        ) noexcept {
            target = System::FieldIdentifier{source};
            return ResultType::Succeeded;
        }

    };

    /// Converts System::TypeAuthorityIdentifier to its exact canonical three-byte storage.
    template<>
    struct TypeConversionAdapter<System::TypeAuthorityIdentifier, System::TypeAuthorityIdentifier::Storage> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Copies canonical Authority bytes into caller-owned storage.
        ///
        /// @param source Semantic Authority identifier being represented.
        /// @param target Caller-owned canonical storage populated on success.
        /// @return Succeeded because every Authority identifier has exact Storage.
        static constexpr ResultType Convert(
            const System::TypeAuthorityIdentifier& source,
            System::TypeAuthorityIdentifier::Storage& target
        ) noexcept {
            target = source.Bytes();
            return ResultType::Succeeded;
        }

    };

    /// Converts exact Authority storage into its semantic System value Type.
    template<>
    struct TypeConversionAdapter<System::TypeAuthorityIdentifier::Storage, System::TypeAuthorityIdentifier> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Reconstructs a semantic Authority identifier from canonical storage.
        ///
        /// @param source Canonical three-byte storage being interpreted.
        /// @param target Caller-owned semantic Authority identifier populated on success.
        /// @return Succeeded because every Storage value maps exactly to the semantic Type.
        static constexpr ResultType Convert(
            const System::TypeAuthorityIdentifier::Storage& source,
            System::TypeAuthorityIdentifier& target
        ) noexcept {
            target = System::TypeAuthorityIdentifier{source};
            return ResultType::Succeeded;
        }

    };

    /// Converts System::TypeLocalIdentifier to its exact canonical five-byte storage.
    template<>
    struct TypeConversionAdapter<System::TypeLocalIdentifier, System::TypeLocalIdentifier::Storage> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Copies canonical local-identifier bytes into caller-owned storage.
        ///
        /// @param source Semantic local identifier being represented.
        /// @param target Caller-owned canonical storage populated on success.
        /// @return Succeeded because every local identifier has exact Storage.
        static constexpr ResultType Convert(
            const System::TypeLocalIdentifier& source,
            System::TypeLocalIdentifier::Storage& target
        ) noexcept {
            target = source.Bytes();
            return ResultType::Succeeded;
        }

    };

    /// Converts exact local-identifier storage into its semantic System value Type.
    template<>
    struct TypeConversionAdapter<System::TypeLocalIdentifier::Storage, System::TypeLocalIdentifier> final {

        // Adapter metadata.

        /// Operation-specific result Type for this lossless identifier conversion.
        using ResultType = Serialisation::SystemIdentifierConversionResult;

        /// Indicates that this exact conversion is supplied by EDP-Serialisation.
        static constexpr bool IsAvailable = true;

        /// Indicates that this conversion performs no throwing operation.
        static constexpr bool IsNoexcept = true;

        // Result interpretation.

        /// Interprets the adapter-specific conversion result for generic consumers.
        ///
        /// @param result Conversion result being inspected.
        /// @return true only for the lossless Succeeded outcome.
        static constexpr bool IsSuccessful(
            ResultType result
        ) noexcept {
            return result == ResultType::Succeeded;
        }

        // Conversion operations.

        /// Reconstructs a semantic local identifier from canonical storage.
        ///
        /// @param source Canonical five-byte storage being interpreted.
        /// @param target Caller-owned semantic local identifier populated on success.
        /// @return Succeeded because every Storage value maps exactly to the semantic Type.
        static constexpr ResultType Convert(
            const System::TypeLocalIdentifier::Storage& source,
            System::TypeLocalIdentifier& target
        ) noexcept {
            target = System::TypeLocalIdentifier{source};
            return ResultType::Succeeded;
        }

    };

} // ESPressio::Bounded
