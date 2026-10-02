# Reference — JSON LocalisedText

**Source:** `src/serialisation/JsonLocalisedText.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header adapts the shared JSON value encoder/decoder to `FieldProfile::LocalisedText`. It owns no language pack and exposes no public operation independently of `Operations.hpp`.

## Constants and policy state

- `JsonRfc5646MetadataKey` — exact reserved metadata key `RFC5646`.
- `JsonLocalisedFieldEncodingPolicy<TResolver>` — non-owning pointers/views binding Resolver, `LocalisationContext`, current-name scratch and comparison scratch for one encoding traversal.
- `JsonLocalisedFieldDecodingPolicy<TResolver>` — non-owning Resolver pointer, terminal language, optional caller-requested language and one Field-name scratch view.
- `JsonLocalisedReplayLimits<TParserLimits>` — internal replay policy used while scanning LocalisedText metadata/duplicate keys without charging the caller's unknown-value skip budget twice.
- `JsonLocalisedLanguageSource` — internal selection of `Embedded`, `Caller`, or `AcrossLanguages` reverse-resolution policy.

All policy structures are per-call stack values and retain no ownership.

## Scratch and text predicates

- `IsLocalisedScratchValid` — validates pointer/capacity invariants for one `WritableTextView`.
- `AreLocalisedScratchViewsIndependent` — requires the two encoding scratch ranges not to overlap.
- `IsLocalisedTextEqual` / `AreLocalisedTextsEqual` — exact byte comparisons; no Unicode normalization, case folding or fuzzy matching.

## Encoding helpers

- `ResolveJsonFieldName` — calls `TResolver::ResolveFieldName` into caller scratch and maps failed lookup to `LocalisationFailure`, incomplete materialisation to `ResourceLimitExceeded`.
- `CheckEarlierLocalisedFieldNameCollision` — recursively re-resolves earlier emitted schema Fields into comparison scratch and rejects equal textual keys.
- `EncodeJsonSchemaPrefix` — LocalisedText overload emitting canonical `"RFC5646":<requested-language>` as the first member of every schema object.
- `EncodeJsonSchemaFieldKey` — resolves one Field key, rejects the reserved metadata spelling and earlier collisions, then emits the JSON String key and colon.

These overloads are selected by the generic policy-aware functions in `JsonEncoding.hpp`; Numeric JSON continues using `JsonNumericFieldEncodingPolicy`.

## Decoding helpers

- `ParseJsonStringToScratch` — decodes one JSON String into caller-owned Field-name scratch with UTF-8 validation inherited from the shared parser.
- `DiscoverJsonLocalisedLanguage` — replays one schema object to find/validate duplicate `RFC5646` metadata without consuming the caller cursor.
- `ValidateJsonLocalisedLanguageSource` — applies embedded > caller > all-language priority, exact payload/caller mismatch rejection, and Resolver context validation.
- `ResolveJsonLocalisedFieldIdentifier` — calls context-specific or across-language reverse resolution according to the validated language source. Embedded-language resolution replays metadata rather than retaining a nested language stack.
- `MapLocalisedFieldResolutionStatus` — preserves `NotFound` and `Ambiguous` as distinct decoder outcomes; all provider/format/dataset/read failures become `LocalisationFailure`.
- `DecodeJsonSchemaWithFieldPolicy` — LocalisedText schema-object decoder. It performs exact textual duplicate replay, reserved metadata handling, reverse Field resolution, canonical Field duplicate tracking, strict/ignore-unknown behavior and required/Optional presence finalization before returning to the shared value decoder.

## Resource invariants

The header allocates no dynamic memory. Field-name storage is caller-owned. Metadata discovery uses one local language array bounded by the public `LanguageIdentifierView` byte domain and destroys it before recursive value decode. Schema duplicate identity continues to use the shared 32-byte `SchemaFieldPresenceSet` backed by `EDP-BoundedTopology::BoundedIndexSet` plus replay for textual duplicate proof.
