#pragma once

#include <cstddef>

#include <ESPressio_BoundedTopology.hpp>
#include <ESPressio_System.hpp>

namespace ESPressio::Serialisation::Detail {

    /// Semantic bounded-index space representing every valid one-byte FieldIdentifier value.
    struct SchemaFieldIndexSpace final {};

    /// Compact one-bit-per-FieldIdentifier presence state reused by JSON and CBOR schema decoders.
    using SchemaFieldPresenceSet = BoundedTopology::BoundedIndexSet<SchemaFieldIndexSpace, 256U>;

    /// Converts one canonical FieldIdentifier to the corresponding bounded presence-set index.
    constexpr SchemaFieldPresenceSet::Index ToSchemaFieldIndex(
        System::FieldIdentifier identifier
    ) noexcept {
        return SchemaFieldPresenceSet::Index::FromUnchecked(
            static_cast<std::size_t>(
                identifier.Value()
            )
        );
    }

    /// Reports whether one schema FieldIdentifier has already been observed.
    constexpr bool IsSchemaFieldSeen(
        const SchemaFieldPresenceSet& seen,
        System::FieldIdentifier identifier
    ) noexcept {
        return seen.IsSet(
            ToSchemaFieldIndex(
                identifier
            )
        );
    }

    /// Marks one schema FieldIdentifier as observed.
    constexpr void MarkSchemaFieldSeen(
        SchemaFieldPresenceSet& seen,
        System::FieldIdentifier identifier
    ) noexcept {
        const auto result = seen.Set(
            ToSchemaFieldIndex(
                identifier
            )
        );

        // Every FieldIdentifier is exactly one byte, so capacity 256 makes this index unconditionally valid.
        (void)result;
    }

} // ESPressio::Serialisation::Detail
