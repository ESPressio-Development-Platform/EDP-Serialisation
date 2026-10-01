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

The current implementation provides the compile-time contract plus allocation-free JSON encoding and transactional replayable-input JSON decoding for Known-Type Body + Numeric Fields. Typed Envelope execution, LocalisedText Fields, and CBOR remain pending.

## Schema reuse

`SerialisableType<T>` never creates a second schema description. For a `System::SchemaType`, it traverses `System::ForEachField<T>()` and recursively qualifies each canonical `FieldBinding::Value`. Field declaration order is therefore schema source metadata only. The implemented Numeric JSON encoder emits schema Fields in ascending `FieldIdentifier` order by compile-time binding lookup and retains no runtime sorting registry.

## Strong semantic adaptation

A strong Type opts in by specializing `CanonicalRepresentation<T>`. That specialization answers only *what* direct representation carries the value. Existing BoundedTypes `TypeConversionAdapter` specializations answer *how* conversion occurs. Both directions must be available, success-aware and `noexcept`. Forward encoding requires the direct canonical surrogate to be nothrow default-constructible because the existing conversion contract populates caller-created target storage. Transactional reverse validation additionally requires the semantic strong Type to be nothrow default-constructible or nothrow copy-constructible so validation has temporary semantic storage without mutating the destination.

A canonical surrogate may not itself be another top-level canonically adapted strong Type, preventing adaptation chains. Supported containers/schemas inside the surrogate may still contain independently valid adapted values.

## Enum qualification

Enum serialisability is explicit through `EnumSerialisationTraits<TEnum>::UnderlyingType`. The trait proves stable numeric-domain intent but does not create reflection over named enumerators. Generic Serialisation therefore accepts any value representable by the certified underlying fixed-width domain.

## State and allocation

The implementation retains no global or per-codec state, schema registry, DOM, allocator, provider state, cache or mutable singleton. `Measure` uses a zero-storage counting sink; `Serialise` writes directly into caller-owned contiguous bytes only after successful preflight validation/measurement. Public result values remain fixed-size value objects. `Deserialise` retains only parser cursors, fixed token/skip state and a 32-byte schema presence bitmap while validating/replaying caller-owned input.

The two-pass encoding contract assumes the source and canonical forward adapters are observationally stable for the duration of one call. Deserialisation validates the complete immutable input before replaying it to populate; reverse adapters must be deterministic for the same surrogate so validation and population cannot diverge.

## Dependency evolution

Direct dependencies are introduced only when source consumes them. JSON caller-buffer emission now consumes EDP-Memory ByteOperations with the EDP-Platform-Portable stateless provider as its default, so both are direct alongside System and BoundedTypes. Localisation remains deferred until LocalisedText execution.
