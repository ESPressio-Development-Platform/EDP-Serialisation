# JSON Typed Envelope

This document defines the implemented V1 JSON `RootProfile::TypedEnvelope` execution over `FieldProfile::Numeric`. It is an explicit root wrapper around the same deterministic body codec used by Known-Type Body.

## Canonical representation

For an identified root Type `T`, canonical output is:

```json
{"$edp":{"v":1,"type":"0102030405060708"},"value":{}}
```

`$edp` is emitted first, with metadata members `v` then `type`, followed by `value`. `v` is `TypedEnvelopeVersion` (`1`). `type` is exactly 16 lowercase hexadecimal characters representing the eight canonical `System::TypeIdentifier::Bytes()` in order. There is no `0x`, separator, codec tag, schema description, nested TypeIdentifier repetition, or runtime Type name.

The wrapped `value` uses the existing JSON Numeric body rules unchanged. Nested schemas therefore remain identified by compile-time parent schema, not by repeated envelope metadata.

## Root qualification

Typed Envelope is available only when the compile-time root both satisfies `SerialisableType<T>` and `System::IdentifiedType<T>`. Scalars, arrays, bounded containers, or other serialisable values without a semantic root `TypeIdentifier` continue to use `KnownTypeBody`. The compile-fail suite locks this requirement.

## Measurement and serialisation

`Measure<Json, RootProfile::TypedEnvelope>(value)` performs the same complete source validation as Known-Type encoding and returns the exact envelope byte count.

`Serialise<Json, RootProfile::TypedEnvelope>(...)` measures first and therefore preserves the existing all-or-nothing caller-buffer guarantee. Insufficient capacity or source validation failure writes no bytes.

## Transactional deserialisation

`Deserialise<Json, RootProfile::TypedEnvelope>(...)` uses the same two-pass replay model as Known-Type decoding. Pass one validates the complete envelope, metadata, Type identity, and body without mutating the destination. Only after that succeeds does pass two populate the destination.

The decoder accepts semantically equivalent outer/metadata member order, but requires exactly one `$edp`, `value`, `v`, and `type` member. Duplicate envelope members return `DuplicateField`; missing/unknown envelope members are malformed representation.

An unsupported numeric envelope version returns `UnsupportedEnvelopeVersion`. A syntactically canonical 16-character lowercase hexadecimal TypeIdentifier which differs from the compile-time target returns `TypeIdentifierMismatch` before destination population. Malformed TypeIdentifier text is `MalformedRepresentation`.

## Parser limits and memory

The envelope consumes ordinary JSON object nesting under the selected `ParserLimits`: the outer envelope is one container level and `$edp`/the wrapped body enter nested levels normally. No envelope DOM or metadata registry is retained. Runtime state is fixed local flags, the existing input cursor, and the existing body parser state.

## Deliberate non-features

Typed Envelope does not dynamically construct an object from the embedded identifier. The caller always supplies the compile-time destination Type. It also does not auto-detect JSON/CBOR, carry library/package versions, or provide application schema migration. LocalisedText Fields are implemented inside the same Typed Envelope body dispatch; CBOR execution remains a later V1 checkpoint.
