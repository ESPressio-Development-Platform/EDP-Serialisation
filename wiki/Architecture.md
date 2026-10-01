# Architecture

## Ownership layers

```text
EDP-System
  semantic TypeIdentifier / FieldIdentifier / FieldSet / SchemaType
        |
        v
EDP-Serialisation
  V1 serialisable-Type universe / representation policy / codecs (incrementally implemented)
        ^
        |
EDP-BoundedTypes
  bounded value storage / capacity / pairwise TypeConversionAdapter
```

The current foundation implements the middle layer's **compile-time contract**, not runtime JSON/CBOR codec operations yet.

## Schema reuse

`SerialisableType<T>` never creates a second schema description. For a `System::SchemaType`, it traverses `System::ForEachField<T>()` and recursively qualifies each canonical `FieldBinding::Value`. Field declaration order is therefore schema source metadata only; later codecs must impose the separately locked numeric FieldIdentifier wire ordering.

## Strong semantic adaptation

A strong Type opts in by specializing `CanonicalRepresentation<T>`. That specialization answers only *what* direct representation carries the value. Existing BoundedTypes `TypeConversionAdapter` specializations answer *how* conversion occurs. Both directions must be available, success-aware and `noexcept`.

A canonical surrogate may not itself be another top-level canonically adapted strong Type, preventing adaptation chains. Supported containers/schemas inside the surrogate may still contain independently valid adapted values.

## Enum qualification

Enum serialisability is explicit through `EnumSerialisationTraits<TEnum>::UnderlyingType`. The trait proves stable numeric-domain intent but does not create reflection over named enumerators. Generic Serialisation therefore accepts any value representable by the certified underlying fixed-width domain.

## State and allocation

The foundation retains no global or per-codec state, schema registry, DOM, allocator, provider object, cache or mutable singleton. The public result values are fixed-size value objects. System identifier adapters operate only on caller-owned source/target values.

Later codec implementation must preserve this ownership model: caller-owned contiguous buffers, no hidden dynamic allocation, and transactional deserialisation.

## Dependency evolution

Direct dependencies are introduced only when source consumes them. The foundation needs System and BoundedTypes. Localisation and direct Memory usage remain deferred to the slices which implement textual Field profiles and raw-memory operations respectively.
