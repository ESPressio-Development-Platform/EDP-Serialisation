# Reference — CanonicalRepresentation

**Source:** `src/serialisation/CanonicalRepresentation.hpp`
**Classification:** PUBLIC EXTENSION API

## `CanonicalRepresentation<TValue>`

`CanonicalRepresentation` names the single direct serialisable surrogate of a strong semantic Type without owning conversion behavior. The primary template is deliberately incomplete, so a Type does not acquire adaptation accidentally.

**Template parameter**

- `TValue` — strong semantic value Type for which Serialisation needs a representation. A specialization must represent the complete semantic value without depending on private object layout.

A valid specialization exposes nested alias `Type`. That alias is consumed by `SerialisableType<TValue>` and must not itself have another `CanonicalRepresentation` specialization; this enforces the locked no-adaptation-chain rule at the represented-Type boundary. Containers/schemas inside the selected representation may still contain independently valid adapted values.

Conversion is not implemented here. Both `Bounded::TypeConversionAdapter<TValue,Type>` and the reverse adapter must be available, `noexcept`, and expose generic success interpretation. This preserves pairwise conversion ownership in EDP-BoundedTypes rather than recreating a Serialisation adapter hierarchy.

The trait owns no runtime state, storage, allocation, lifecycle, synchronization, or registry entry.

**Validation:** host tests cover a valid strong `uint32_t` surrogate and an incomplete one-direction adapter which must fail qualification.
