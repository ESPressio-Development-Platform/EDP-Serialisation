# Reference — Schema Field Presence

**Source:** `src/serialisation/SchemaFieldPresence.hpp`  
**Classification:** PRIVATE IMPLEMENTATION

This header adapts the complete one-byte `System::FieldIdentifier` domain onto the reusable `EDP-BoundedTopology::BoundedIndexSet` facility. It is shared by JSON Numeric, JSON LocalisedText, and CBOR Numeric schema decoding so Serialisation does not retain a parallel hand-written finite-membership bitmap.

## `SchemaFieldIndexSpace`

Private semantic tag distinguishing Serialisation's schema-Field index space from unrelated bounded index spaces.

## `SchemaFieldPresenceSet`

Alias of `BoundedTopology::BoundedIndexSet<SchemaFieldIndexSpace, 256U>`. The capacity exactly matches the complete `FieldIdentifier` value domain `0..255`, retaining one bit per possible Field identity and therefore exactly 32 bytes of membership state.

## `ToSchemaFieldIndex(identifier)`

Converts the canonical one-byte `System::FieldIdentifier` into the corresponding strong bounded index. Because every possible FieldIdentifier value is valid and the set capacity is exactly 256, this conversion is total and requires no runtime rejection path.

## `IsSchemaFieldSeen(seen, identifier)`

Predicate delegating membership inspection to `BoundedIndexSet::IsSet`.

## `MarkSchemaFieldSeen(seen, identifier)`

Delegates membership mutation to `BoundedIndexSet::Set`. The mutation result cannot be `InvalidIndex` for a canonical one-byte FieldIdentifier mapped into capacity 256; the helper intentionally retains no second membership implementation.

The header owns no dynamic memory, synchronization, registry, object lifetime, or codec-specific representation logic.
