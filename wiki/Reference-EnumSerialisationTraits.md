# Reference — EnumSerialisationTraits

**Source:** `src/serialisation/EnumSerialisationTraits.hpp`
**Classification:** PUBLIC EXTENSION API

## `EnumSerialisationTraits<TEnum>`

Explicitly certifies the stable numeric representation of an enum. The primary template is deliberately incomplete; ordinary C++ enums are not serialisable until their owning integration provides a specialization.

**Template parameter**

- `TEnum` — enum Type being certified.

A specialization exposes nested alias `UnderlyingType`. `SerialisableType` requires it to be exactly `std::underlying_type_t<TEnum>` and one of the admitted fixed-width integer Types. The trait does not enumerate or validate source-level named enumerators; every value representable by the certified underlying domain remains structurally serialisable, preserving forward compatibility and flags/bitmask use cases.

The trait owns no runtime state and does not define enum-name/string representation.

**Validation:** the positive host fixture certifies `Mode : std::uint8_t`; `tests/compile_fail/uncertified_enum.cpp` proves an enum without the specialization is rejected.
