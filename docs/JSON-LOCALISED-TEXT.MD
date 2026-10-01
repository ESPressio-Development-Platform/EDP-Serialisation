# JSON LocalisedText Field Profile

The JSON `FieldProfile::LocalisedText` execution path is implemented for both `RootProfile::KnownTypeBody` and `RootProfile::TypedEnvelope`. It reuses the same scalar/container/schema value codec as the Numeric profile; only schema-object Field identity is represented through EDP-Localisation names.

## Encoding contract

Every schema object emits the reserved metadata member first:

```json
{"RFC5646":"de","Temperature":27}
```

`RFC5646` contains the caller-requested canonical language from the supplied `LocalisationContext`. It remains the requested language even when `ResolveFieldName` obtains an individual Field name from a fallback language pack. Field values are emitted in ascending numeric `FieldIdentifier` order after the metadata member.

Each Field name is resolved through the caller-owned EDP-Localisation Resolver. Two independent caller-owned `WritableTextView` scratch buffers are supplied: one materialises the current Field name and one replays earlier emitted Field names for exact collision detection. Scratch exhaustion returns `ResourceLimitExceeded`; no allocation or truncation fallback exists.

Resolved names must not equal the reserved exact case-sensitive key `RFC5646`. Two emitted schema Fields must not resolve to the same textual key. Either condition returns `FieldNameCollision` before caller output is written because `Serialise` performs the same exact `Measure` preflight first.

## Decoding language priority

For each schema object, decoding discovers optional `RFC5646` metadata independently of JSON member order before resolving ordinary Field names.

Resolution priority is:

1. embedded `RFC5646`, when present;
2. caller-requested language, when supplied and metadata is absent;
3. complete cross-language reverse resolution when neither payload nor caller supplies a requested language.

When payload metadata and a caller-requested language are both present they must match exactly; otherwise decoding returns `InvalidLanguageMetadata`. Embedded/caller languages are combined with the caller-supplied terminal language and EDP-Localisation owns the requested-to-terminal fallback chain.

The no-caller-language overload takes only the terminal language. If payload metadata is absent it calls `ResolveFieldIdentifierAcrossLanguages`; success requires complete proof across the generated supported-language set. `Ambiguous` maps to `FieldNameAmbiguous`; provider/pack/dataset failures map to `LocalisationFailure` and are never treated as an unknown Field.

## Unknown and duplicate textual Fields

A conclusive reverse-resolution `NotFound` maps to `FieldNameNotFound`. Under `StrictnessPolicy::Exact` this fails. Under `IgnoreUnknownFields` the structurally valid unknown value is skipped within `ParserLimits`.

Duplicate textual JSON keys fail even when one spelling uses JSON escapes. Distinct textual keys which resolve to the same canonical `FieldIdentifier` also fail with `DuplicateField`. Duplicate `RFC5646` metadata fails identically.

## Transactional behavior

LocalisedText decoding uses the same two-pass replay contract as Numeric JSON. Pass one validates metadata, Localisation resolution, schema presence, capacities and every value without mutating the destination. Pass two replays the same immutable input and populates only after validation succeeds.

Resolver/pack data must remain observationally stable for the complete operation. The implementation retains no Localisation registry, pack copy, DOM or unbounded key table.

## Memory/resource behavior

Field-name materialisation uses caller-owned scratch. Embedded RFC5646 discovery uses a temporary bounded by the public `LanguageIdentifierView` length domain (`std::uint8_t`, maximum 255 bytes). The temporary is scoped to metadata discovery/resolution and does not accumulate with schema nesting. Duplicate-key checks replay earlier object members rather than retaining their text.

## Dependencies

This profile introduces `EDP-Localisation` as a mandatory direct dependency because production headers consume `LocalisationContext`, `LanguageIdentifierView`, `WritableTextView`, forward Field-name resolution and reverse Field-identifier resolution. EDP-Localisation remains the sole owner of language-pack lookup, fallback policy and ambiguity proof.
