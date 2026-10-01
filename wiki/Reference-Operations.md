# Reference — Operations

**Source:** `src/serialisation/Operations.hpp`
**Public classification:** PUBLIC OPERATION API
**Detail classification:** PRIVATE IMPLEMENTATION

This header exposes JSON Numeric and LocalisedText `Measure`, `Serialise`, and transactional `Deserialise` for Known-Type Body and Typed Envelope roots, and maps private codec outcomes into the public operation-specific result families.

## `Measure<TCodec, TRootProfile, TFieldProfile, TValue>(value)`

PUBLIC API. `TCodec` selects the compile-time codec; `TRootProfile` defaults to `KnownTypeBody`; `TFieldProfile` defaults to `Numeric`; `TValue` must satisfy `SerialisableType`.

The implemented combinations are `Json + Numeric` with either `KnownTypeBody` or `TypedEnvelope`. Typed Envelope additionally requires `System::IdentifiedType<T>` and measures canonical root metadata plus the same body traversal. The parameter-only overload remains Numeric; dedicated overloads implement LocalisedText. CBOR selections remain compile-time rejected. The function validates all represented values/adaptations and returns exact `RequiredBytes` only on success.

## `Serialise<TCodec, TRootProfile, TFieldProfile, TValue>(value, output, capacity)`

PUBLIC API. Codec/root/Field parameters have the same meaning and current profile restriction as `Measure`. `TByteOperationsProvider` defaults to the stateless portable EDP-Memory ByteOperations provider and may be replaced by another conforming stateless provider at compile time; `TValue` is deduced and must satisfy `SerialisableType`.

Parameters:

- `value` — caller-owned source value;
- `output` — first byte of caller-owned contiguous output storage;
- `capacity` — writable bytes available from `output`.

The operation first invokes exact `Measure`. Invalid source/adaptation therefore causes no write. Null output yields `InvalidArgument`; insufficient capacity yields `OutputBufferTooSmall` and preserves exact `RequiredBytes`; both report `BytesWritten == 0`. Only a fully prevalidated value with sufficient capacity enters the direct write traversal.

The source and forward conversion adapters must remain observationally stable for the duration of the two-pass call. The operation performs no hidden allocation or staging-copy of the encoded representation.

## `Detail::ToMeasurementStatus(JsonEncodingStatus)`

PRIVATE IMPLEMENTATION constexpr mapper. Converts each internal encoder outcome into the equivalent `MeasurementStatus`: success, resource limit, non-finite number, invalid UTF-8, or adaptation failure. It has no retained state.

## `Detail::ToSerialisationStatus(JsonEncodingStatus)`

PRIVATE IMPLEMENTATION constexpr mapper from encoder outcomes to the corresponding `SerialisationStatus` family.

## `Detail::ToSerialisationStatus(MeasurementStatus)`

PRIVATE IMPLEMENTATION constexpr mapper used when Serialise preflight fails. It preserves the coherent public meaning of every Measurement failure in the Serialisation result family.

## `Detail::ValidateImplementedEncodingProfile<TCodec, TRootProfile, TFieldProfile>()`

PRIVATE IMPLEMENTATION consteval profile gate. `TCodec`, `TRootProfile`, and `TFieldProfile` are compile-time selection parameters. It currently requires exact `Json`, `Numeric`, and either `KnownTypeBody` or `TypedEnvelope`; pending CBOR combinations fail with focused `static_assert` diagnostics rather than runtime error/fallback state.

## Validation relationships

Host coverage checks exact measurement/written-size agreement, scalar/container/schema/adapter output, canonical Field ordering, Optional omission, UTF-8/Base64 behavior, invalid-source preflight, null output, insufficient-capacity preservation, and failed adaptation. PlatformIO Arduino and ESP-IDF demos instantiate the public operations on ESP32.


## `Deserialise<TCodec,TRootProfile,TFieldProfile,TStrictness,TParserLimits,TValue>(input,length,destination)`

PUBLIC API. `TCodec` is currently required to be `Json`; the parameter-only overload defaults to `KnownTypeBody` / `Numeric`; `TStrictness` defaults to `Exact`; `TParserLimits` defaults to `DefaultParserLimits`; `TValue` must satisfy `SerialisableType`.

The operation rejects null input, validates the complete immutable input with `DecodeJsonValue<false>`, allows only trailing JSON whitespace, then replays the same bytes through `DecodeJsonValue<true>` to populate. Failures report `BytesConsumed == 0`; success reports the complete supplied length. Pass one may read existing destination state solely to seed a copy-constructible non-default strong semantic validation temporary; it does not mutate caller state.

## `Detail::ToDeserialisationStatus(JsonDecodingStatus)`

PRIVATE IMPLEMENTATION constexpr mapper preserving decoder failure distinctions including malformed structure, resource limits, unknown/duplicate/missing Fields, numeric failures, invalid UTF-8/Base64, bounded capacity and adaptation failure.

## `Detail::ValidateImplementedDecodingProfile<TCodec,TRootProfile,TFieldProfile>()`

PRIVATE IMPLEMENTATION consteval profile gate. Numeric and LocalisedText each have focused compile-time gates; unsupported codec/profile execution remains a compile-time error rather than runtime fallback.

## Typed Envelope dispatch

For `RootProfile::TypedEnvelope`, all three public operations statically require `System::IdentifiedType<TValue>`. Measure/Serialise delegate to `EncodeJsonTypedEnvelope`; Deserialise delegates each replay pass to `DecodeJsonTypedEnvelope`. Known-Type Body continues to delegate directly to the body encoder/decoder.


## LocalisedText overloads

PUBLIC API overloads select `FieldProfile::LocalisedText` and require an EDP-Localisation Resolver plus caller-owned scratch.

### Encoding `Measure` / `Serialise`

Both accept `resolver`, a complete `LocalisationContext`, `fieldNameScratch`, and `comparisonScratch`. The scratch ranges must be valid and non-overlapping. The context and Resolver fallback chain are validated before traversal. `Measure` resolves/collision-checks each emitted Field and includes exact RFC5646/name bytes. `Serialise` calls that same measurement first, preserving the no-partial-write contract.

### Decoding with explicit caller context

`Deserialise(..., resolver, context, fieldNameScratch)` supplies both caller requested and terminal languages. Embedded RFC5646 metadata, when present, has priority but must exactly equal `context.RequestedLanguage`. Missing metadata uses the caller context.

### Decoding without caller requested language

`Deserialise(..., resolver, terminalLanguage, fieldNameScratch)` permits payload-only RFC5646 metadata. If metadata is absent, textual keys use `ResolveFieldIdentifierAcrossLanguages`, preserving Localisation `NotFound` versus `Ambiguous` semantics.

Both overload families delegate transactional passes to `Detail::DeserialiseJsonWithFieldPolicy`; the Resolver/pack source and caller scratch must remain stable for the complete operation.

## `Detail::DeserialiseJsonWithFieldPolicy`

PRIVATE IMPLEMENTATION helper shared by Numeric-policy composition and LocalisedText overloads. It performs validate-only traversal, trailing-data check, replayed population, and Typed Envelope dispatch while preserving one Field policy instance across each pass.

## Localised status mappings

`Detail::ToMeasurementStatus`, `ToSerialisationStatus`, and `ToDeserialisationStatus` preserve `InvalidLanguageMetadata`, `FieldNameCollision`, `FieldNameNotFound`, `FieldNameAmbiguous`, and `LocalisationFailure` as operation-appropriate public outcomes rather than collapsing them into generic malformed/unknown statuses.
