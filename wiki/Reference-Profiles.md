# Reference — Profiles

**Source:** `src/serialisation/Profiles.hpp`
**Classification:** PUBLIC PROFILE API

This header contains compile-time representation selectors. The tags/enums retain no runtime state.

## `RootProfile`

- `KnownTypeBody = 0` — caller already knows the semantic root Type; only the value body is represented.
- `TypedEnvelope = 1` — explicit root envelope carries envelope version, semantic `TypeIdentifier`, and body.

## `FieldProfile`

- `Numeric = 0` — schema objects use canonical numeric `FieldIdentifier` keys.
- `LocalisedText = 1` — schema objects use Localisation-resolved text keys plus reserved RFC5646 context metadata. Localisation execution is not implemented in this foundation slice.

## `StrictnessPolicy`

- `Exact = 0` — any unknown Field is a deserialisation failure.
- `IgnoreUnknownFields = 1` — a conclusively unknown Field may be structurally validated/skipped; ambiguous or failed identity resolution remains an error.

## Codec tags

- `Json` — empty compile-time tag selecting JSON when codec operations are introduced.
- `Cbor` — empty compile-time tag selecting CBOR when codec operations are introduced.

Neither tag contains a codec object, registry entry, vtable, or runtime state.

## `TypedEnvelopeVersion`

Constant `std::uint8_t` value `1`. It versions only the EDP-Serialisation typed-envelope representation, not the library release, application schema, codec, or semantic Type.
