# Reference — SerialisableType

**Source:** `src/serialisation/SerialisableType.hpp`
**Primary public classification:** PUBLIC CONCEPT API
**Detail namespace classification:** PRIVATE IMPLEMENTATION

This header implements the complete recursive Boolean V1 Type universe. It owns no runtime schema copy or registry; schema Fields are traversed directly through EDP-System.

## Public declarations

### `IsSerialisableType<TValue>`

PUBLIC variable template. `TValue` is the value Type being classified. It removes top-level cv-qualification and exposes the final `Detail::SerialisableTypeTrait` Boolean. It has no runtime storage.

### `SerialisableType<TValue>`

PUBLIC concept equal to `IsSerialisableType<TValue>`. Higher ESPressio layers use this as the canonical compile-time predicate for “this semantic value can participate in V1 Serialisation.”

## Platform invariants

Namespace-scope `static_assert`s require:

- 8-bit bytes;
- exact 8/16/32/64-bit fixed-width integer Types;
- IEEE-754/IEC559 binary32 `float` with radix 2, 24 significant binary digits and expected exponent domain;
- IEEE-754/IEC559 binary64 `double` with radix 2, 53 significant binary digits and expected exponent domain.

These checks prevent the implementation from silently changing numeric wire semantics on an incompatible C++ target. C++ does erase typedef spelling: if native `int`, `unsigned long`, `size_t`, etc. are literally the same C++ Type as one of the fixed-width typedefs, the concept cannot know which spelling appeared in source. Developers must use fixed-width spellings for portable schema intent; no wire implementation may therefore fall back to native object layout.

## `Detail::NormalizedType<TValue>`

PRIVATE IMPLEMENTATION alias removing top-level cv qualification before classification. `TValue` is the incoming value Type. References are intentionally not removed and consequently do not qualify as ordinary value domains.

## `Detail::IsFixedWidthInteger<TValue>`

PRIVATE IMPLEMENTATION variable template. True for actual C++ Types equal to the eight `std::intN_t`/`std::uintN_t` domains admitted by V1. It does not inspect object bytes or signed representation.

## `Detail::IsSupportedFloatingPoint<TValue>`

PRIVATE IMPLEMENTATION variable template. True only for `float` and `double`; `long double` is deliberately excluded.

## `Detail::IsOptional<TValue>`

PRIVATE IMPLEMENTATION trait family:

- primary template derives `std::false_type`;
- `IsOptional<std::optional<T>>` derives `std::true_type`.

It exists solely to reject direct `Optional<Optional<T>>` while preserving ordinary recursive Optional qualification.

## `Detail::HasCanonicalRepresentation<TValue>`

PRIVATE IMPLEMENTATION detection concept. True when a `CanonicalRepresentation<TValue>` specialization exposes nested `Type`. It detects opt-in only; validity of the representation/adapters is checked separately.

## `Detail::HasEnumSerialisationTraits<TValue>`

PRIVATE IMPLEMENTATION detection concept. True when `EnumSerialisationTraits<TValue>` exposes `UnderlyingType`.

## `Detail::PotentialSchemaType<TValue>`

PRIVATE IMPLEMENTATION safety concept which requires:

- a static `Identifier` declaration;
- nested `Fields`;
- `Identifier` to have exact `System::TypeIdentifier` Type;
- `Identifier.IsValid()` to be a constant expression suitable for System's predicate.

It exists because current `EDP-System/main` focused `SchemaType` machinery can hard-error when probed with unrelated malformed Types. `EDP-System#10` owns the substitution-safe correction already implemented/validated on `serialisation_prerequisites`; this local pre-gate preserves Boolean Serialisation qualification against current main without taking ownership of System's predicate.

## `Detail::SerialisableTypeTrait<TValue>`

PRIVATE IMPLEMENTATION recursive classifier. The primary template delegates to scalar/schema/adaptation classification. Specializations implement the explicitly admitted recursive container forms:

- `std::array<T,N>` — value equals recursive element qualification; exact extent is retained by the Type itself.
- native `T[N]` — same recursive element rule.
- `std::optional<T>` — rejects nested Optional and requires a nothrow default-constructible element before recursion so an absent destination can be engaged transactionally.
- `Bounded::String<N,Provider>` — true as an admitted bounded-text family; UTF-8 validity is runtime codec validation, not Type qualification.
- `Bounded::Bytes<N,Provider>` — true as an admitted bounded-octet family.
- `Bounded::Vector<T,N>` — requires nothrow default construction plus recursive element qualification because decode activates new inline slots; capacity is compile-time metadata owned by BoundedTypes.

Each specialization exposes member `Value`, the authoritative Boolean result for that exact family. The Optional specialization additionally exposes nested alias `Element`, the cv-normalized contained Type used by the direct-nesting rule.

No arbitrary iterable/container fallback exists; unlisted containers remain false.

## `Detail::AreSchemaFieldsSerialisable<TValue>()`

PRIVATE IMPLEMENTATION consteval predicate. It invokes `System::ForEachField<TValue>` over the canonical FieldSet and examines each `System::FieldValueOf<TField>`. Result begins true and becomes false if any Field's recursive `SerialisableTypeTrait` is false. No Field metadata or registry is retained.

## `Detail::IsCertifiedEnum<TValue>()`

PRIVATE IMPLEMENTATION consteval predicate. It requires `TValue` to be an enum, requires the enum trait specialization, compares declared `UnderlyingType` exactly with `std::underlying_type_t<TValue>`, then requires that Type to satisfy the fixed-width integer classifier. It deliberately does not inspect named enumerators.

## `Detail::IsCanonicalAdaptation<TValue>()`

PRIVATE IMPLEMENTATION consteval predicate implementing S1:

1. `CanonicalRepresentation<TValue>::Type` must exist.
2. Representation must not be a reference or the same normalized Type as `TValue`.
3. Representation itself must not expose another `CanonicalRepresentation`, preventing a top-level adaptation chain.
4. Representation must be nothrow default-constructible so generic conversion has bounded target storage.
5. The semantic `TValue` must be nothrow default-constructible or nothrow copy-constructible so reverse validation can use temporary semantic storage without mutating caller state.
6. Representation must recursively satisfy `SerialisableTypeTrait`; valid nested Fields/elements may independently use their own adaptations.
7. Both BoundedTypes conversion directions must be available.
8. Both directions must expose `HasTypeConversionSuccessPredicate`.
9. Both adapter Types must advertise `IsNoexcept`.

The function performs compile-time qualification only; it does not execute conversion.

## `Detail::IsSerialisableScalarSchemaOrAdaptation<TValue>()`

PRIVATE IMPLEMENTATION consteval classifier used by the primary trait. Evaluation order is:

1. `bool`;
2. fixed-width integer domain;
3. supported floating domain;
4. certified enum;
5. safe System schema — requires `System::SchemaType` and recursively serialisable Fields;
6. otherwise canonical strong-Type adaptation.

This ordered dispatch avoids probing System schema machinery for unrelated scalar/container Types.

## Validation relationships

Host coverage proves positive scalar, enum, bounded container, fixed-array, Optional, nested schema, adapted strong Type, schema-with-adapted-Field, and System identifier cases. Negative assertions cover dynamic STL owners, pointers, `long double`, unsupported schema Fields and nested Optional. Compile-fail fixtures separately prove uncertified enums, incomplete adaptations, nested Optional, and non-default-constructible canonical surrogates are rejected.
