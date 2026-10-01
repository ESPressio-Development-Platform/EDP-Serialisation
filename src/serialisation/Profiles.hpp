#pragma once

#include <cstdint>

namespace ESPressio::Serialisation {

    /// Selects the root representation profile.
    enum class RootProfile : std::uint8_t {
        /// Encodes only the value body because the caller already knows the semantic root Type.
        KnownTypeBody = 0U,
        /// Encodes the explicit V1 envelope version, root TypeIdentifier, and value body.
        TypedEnvelope = 1U
    };

    /// Selects the Field-key representation profile.
    enum class FieldProfile : std::uint8_t {
        /// Encodes canonical numeric FieldIdentifier keys.
        Numeric = 0U,
        /// Encodes Localisation-resolved textual Field keys plus RFC5646 metadata.
        LocalisedText = 1U
    };

    /// Selects unknown-Field handling during deserialisation.
    enum class StrictnessPolicy : std::uint8_t {
        /// Rejects every Field not represented by the target schema.
        Exact = 0U,
        /// Structurally validates and skips conclusively unknown Fields.
        IgnoreUnknownFields = 1U
    };

    /// Selects the JSON codec at compile time.
    struct Json final {
    };

    /// Selects the CBOR codec at compile time.
    struct Cbor final {
    };

    /// Initial typed-envelope wire-format version.
    inline constexpr std::uint8_t TypedEnvelopeVersion = 1U;

} // ESPressio::Serialisation
