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

The current implementation provides the compile-time contract plus allocation-free JSON and CBOR Numeric encoding/transactional replayable-input decoding for both Known-Type Body and explicit Typed Envelope roots. JSON also implements LocalisedText Fields.

## Schema reuse

`SerialisableType<T>` never creates a second schema description. For a `System::SchemaType`, it traverses `System::ForEachField<T>()` and recursively qualifies each canonical `FieldBinding::Value`. Field declaration order is therefore schema source metadata only. The implemented Numeric JSON and CBOR encoders emit schema Fields in ascending `FieldIdentifier` order by compile-time binding lookup and retain no runtime sorting registry.

## Strong semantic adaptation

A strong Type opts in by specializing `CanonicalRepresentation<T>`. That specialization answers only *what* direct representation carries the value. Existing BoundedTypes `TypeConversionAdapter` specializations answer *how* conversion occurs. Both directions must be available, success-aware and `noexcept`. Forward encoding requires the direct canonical surrogate to be nothrow default-constructible because the existing conversion contract populates caller-created target storage. Transactional reverse validation additionally requires the semantic strong Type to be nothrow default-constructible or nothrow copy-constructible so validation has temporary semantic storage without mutating the destination.

A canonical surrogate may not itself be another top-level canonically adapted strong Type, preventing adaptation chains. Supported containers/schemas inside the surrogate may still contain independently valid adapted values.

## Enum qualification

Enum serialisability is explicit through `EnumSerialisationTraits<TEnum>::UnderlyingType`. The trait proves stable numeric-domain intent but does not create reflection over named enumerators. Generic Serialisation therefore accepts any value representable by the certified underlying fixed-width domain.

## State and allocation

The implementation retains no global or per-codec state, schema registry, DOM, allocator, provider state, cache or mutable singleton. `Measure` uses one shared zero-storage codec-neutral counting sink; JSON and CBOR `Serialise` write directly into caller-owned contiguous bytes only after successful preflight validation/measurement. Public result values remain fixed-size value objects. `Deserialise` retains only parser cursors, fixed token/skip state and a 32-byte `EDP-BoundedTopology::BoundedIndexSet` schema presence set while validating/replaying caller-owned input.

The two-pass encoding contract assumes the source and canonical forward adapters are observationally stable for the duration of one call. Deserialisation validates the complete immutable input before replaying it to populate; reverse adapters must be deterministic for the same surrogate so validation and population cannot diverge.

## Dependency evolution

Direct dependencies are introduced only when source consumes them. Schema duplicate/presence tracking directly consumes `EDP-BoundedTopology::BoundedIndexSet`. JSON and CBOR caller-buffer emission consume the shared EDP-Memory ByteOperations contract with the EDP-Platform-Portable stateless provider as the default, so both are direct alongside System and BoundedTypes. LocalisedText execution now consumes EDP-Localisation directly; Serialisation delegates language validation/fallback and forward/reverse presentation identity to that repository and retains no language-pack representation.

## Typed Envelope root layer

`RootProfile::TypedEnvelope` is deliberately a thin root-only layer around the selected body codec. Canonical JSON emits `$edp` metadata (`v`, canonical lowercase root `TypeIdentifier`) before `value`; canonical CBOR emits definite `[1, h'<8 identifier bytes>', <body>]`. Decode validates the complete selected envelope in pass one before any body population occurs in pass two. The embedded identity is checked against the compile-time target and is never used to look up or dynamically construct a Type. Nested schema values therefore remain ordinary body values with no repeated identity metadata.


## LocalisedText Field layer

The shared JSON scalar/container/schema codec is Field-policy aware. Numeric Fields use a zero-state policy. LocalisedText binds a caller-owned Resolver, language context and bounded text scratch. Schema encoding emits reserved RFC5646 metadata first and resolves each Field key through EDP-Localisation; schema decoding discovers metadata by replay, applies embedded > caller > all-language priority, then reverse-resolves names to canonical FieldIdentifiers before dispatching through the same value decoder. This avoids a parallel codec, Localisation registry, DOM or retained dictionary.


## CBOR Numeric layer

CBOR Numeric reuses the same schema/adaptation universe and the shared codec-neutral output sinks. It emits definite-length arrays/maps, unsigned numeric FieldIdentifier keys in ascending order, native major type 0/1 integers with shortest legal heads, exact binary32/binary64 widths, text/byte strings with no cross-category coercion, and CBOR null for disengaged root/sequence Optional values. The decoder rejects indefinite containers and non-canonical heads, applies the same shared 256-entry `EDP-BoundedTopology::BoundedIndexSet` schema presence set and ParserLimits, and uses the same two-pass transactional destination rule as JSON.
