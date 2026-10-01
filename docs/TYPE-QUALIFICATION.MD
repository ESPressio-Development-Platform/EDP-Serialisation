# V1 Type Qualification

`SerialisableType<T>` is the canonical recursive compile-time qualification for values accepted by EDP-Serialisation V1.

## Direct values

Direct scalar values are `bool`, the fixed-width signed/unsigned integer Types from 8 through 64 bits, IEEE-754 binary32 `float`, IEEE-754 binary64 `double`, and explicitly certified enums. The header statically establishes eight-bit bytes and the expected IEEE binary32/binary64 characteristics.

C++20 cannot distinguish typedef spelling. If `std::int32_t` aliases native `int`, or a fixed-width unsigned Type is also the implementation's `std::size_t`, those spellings are one C++ Type and a concept cannot accept one while rejecting the other. Qualification therefore follows actual fixed-width Type identity. Developers must use fixed-width spellings for portable schema intent because source alias spelling is erased before template qualification; wire representation itself remains value-based and independent of host object layout.

## Enum certification

A V1 enum opts in through:

```cpp
template<>
struct ESPressio::Serialisation::EnumSerialisationTraits<MyEnum> {
    using UnderlyingType = std::uint8_t;
};
```

The declared Type must exactly equal `std::underlying_type_t<MyEnum>` and be a supported fixed-width integer Type. Serialisation does not validate membership in the currently named enumerator set.

## Bounded values and sequences

Supported recursive forms are `Bounded::String<N>`, `Bounded::Bytes<N>`, `Bounded::Vector<T,N>`, `std::array<T,N>`, native `T[N]`, and `std::optional<T>`. Directly nested Optional is rejected because omission/null/value cannot preserve its three logical levels consistently across object fields and positional roots/sequences. Transactional decode must be able to create an engaged Optional value or activate a new bounded Vector slot, so Optional elements and Bounded Vector elements must also be nothrow default-constructible.

## Schema recursion

A `System::SchemaType` qualifies only when every Field value recursively qualifies. Serialisation reuses `System::ForEachField`; it does not copy schema metadata into a second registry.

## Strong semantic adaptation

`CanonicalRepresentation<T>` names one direct surrogate. Both `TypeConversionAdapter<T,Representation>` and the reverse adapter must exist, be `noexcept`, and expose the generic `IsSuccessful(ResultType)` capability detected by BoundedTypes. The representation itself must be directly serialisable without another canonical adaptation, preventing adaptation chains.

The library supplies this integration for System identifier value Types using their canonical byte/storage domains.

## Canonical surrogate construction

Forward encoding uses the existing pairwise `TypeConversionAdapter<Strong, Representation>` contract, which populates an already-existing target object. Generic Serialisation therefore also requires the direct canonical surrogate to be nothrow default-constructible so it can create bounded temporary conversion storage without introducing a second construction/conversion extension point. Transactional reverse-adapter validation additionally requires the strong semantic Type itself to be either nothrow default-constructible or nothrow copy-constructible; a non-default semantic Type such as `System::FieldIdentifier` can seed validation from the caller's existing destination value without mutating it.
