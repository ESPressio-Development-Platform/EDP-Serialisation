# Reference — JSON Typed Envelope

**Source:** `src/serialisation/JsonEnvelope.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header implements the JSON `RootProfile::TypedEnvelope` wrapper. Public consumers select the root profile through `Measure`, `Serialise`, and `Deserialise`; declarations in this header are not cross-repository compatibility surfaces.

## Member classifiers

- `JsonEnvelopeRootMember` — fixed internal identity for `$edp`, `value`, or unknown root members.
- `JsonEnvelopeMetadataMember` — fixed internal identity for `v`, `type`, or unknown metadata members.
- `ParseJsonEnvelopeRootMember` / `ParseJsonEnvelopeMetadataMember` — stream decoded JSON key bytes and compare against the small fixed known-name set without allocating or retaining arbitrary key strings. Escaped-equivalent spellings resolve to the same logical member.

## Encoding helpers

- `EncodeJsonTypeIdentifier` — emits exactly eight `TypeIdentifier::Bytes()` as 16 lowercase hexadecimal characters inside a JSON String.
- `EncodeJsonTypedEnvelope<TSink,TValue>` — statically requires `System::IdentifiedType<TValue>` and emits canonical `{"$edp":{"v":1,"type":"..."},"value":<body>}`. The body is delegated to `EncodeJsonValue`, so no second schema/value encoder exists.

## Decoding helpers

- `DecodeJsonEnvelopeVersion` — parses one JSON number and accepts only the current `TypedEnvelopeVersion`; any other valid numeric version returns the internal `UnsupportedEnvelopeVersion`.
- `DecodeJsonEnvelopeTypeIdentifier<TValue>` — parses exactly 16 logical lowercase hexadecimal characters and compares them directly with the compile-time target Type's canonical bytes. Malformed text and semantic mismatch remain distinct outcomes.
- `DecodeJsonEnvelopeMetadata<TParserLimits,TValue>` — validates exactly one `v` and one `type` member, permits either order, rejects duplicate/unknown/missing metadata, and accounts the metadata object against nesting limits.
- `DecodeJsonTypedEnvelope<TPopulate,TStrictness,TParserLimits,TValue>` — validates exactly one `$edp` and one `value` member in either order, delegates body processing to `DecodeJsonValue`, and reuses the public operation's validate-then-populate replay model.

## Transactionality

Pass one parses and validates the entire envelope including version and TypeIdentifier before `Operations.hpp` begins pass two. Therefore a version or identity failure cannot mutate destination state. Pass two reparses the same immutable bytes; if `value` precedes `$edp` in source order, the metadata has nevertheless already been proven by pass one.

## Memory and ownership

Envelope parsing retains only fixed booleans/counters plus the existing caller-owned input cursor and body parser state. It adds no registry, DOM, dynamic allocation, Type factory, background state, or nested TypeIdentifier storage.
