# CBOR Numeric Profiles

EDP-Serialisation implements deterministic V1 CBOR for `FieldProfile::Numeric` with both `RootProfile::KnownTypeBody` and `RootProfile::TypedEnvelope`.

## Representation

- Schema objects are definite-length CBOR maps with unsigned integer `FieldIdentifier` keys emitted in ascending semantic identifier order.
- `std::array`, native fixed arrays, and `Bounded::Vector` are definite-length arrays. Indefinite-length arrays/maps are rejected.
- Signed/unsigned fixed-width integers use CBOR major types 0/1 and the shortest legal argument width.
- `bool` uses CBOR `false`/`true` and never coerces to or from integer.
- `float` is always IEEE-754 binary32 (`0xFA`); `double` is always binary64 (`0xFB`). Half precision and cross-width float coercion are rejected. NaN and infinities are outside V1; finite subnormals and signed zero are preserved.
- `Bounded::String<N>` maps only to a definite CBOR text string and is validated as V1 UTF-8.
- `Bounded::Bytes<N>` maps only to a definite CBOR byte string.
- Disengaged Optional object Fields are omitted. A root/sequence Optional, or an explicitly null Optional object Field, uses CBOR null (`0xF6`).

## Typed Envelope

The CBOR Typed Envelope is exactly the definite three-element array:

```text
[1, h'<8 canonical TypeIdentifier bytes>', <body>]
```

The identifier is an exact eight-byte CBOR byte string in `TypeIdentifier::Bytes()` order. The envelope contains no codec tag, schema, Type name, runtime registry metadata, or nested identity repetition.

## Decoding and transactionality

`Deserialise<Cbor,...>` accepts one caller-owned `std::span<const std::byte>`, validates the complete contiguous input before replaying it for population. Any validation failure reports `BytesConsumed == 0` and leaves the destination unchanged. Trailing bytes fail.

`StrictnessPolicy::Exact` rejects unknown numeric Fields. `IgnoreUnknownFields` structurally validates and skips unknown values subject to `ParserLimits`; duplicate numeric Field identities always fail.

The decoder rejects non-canonical integer/length heads, indefinite-length containers, wrong major-type categories, integer overflow, wrong floating width, non-finite floats, invalid UTF-8, capacity overflow, and failed reverse strong-Type adaptation before population.

## Output ownership

`Measure<Cbor>` uses the shared zero-storage counting sink. `Serialise<Cbor>` accepts caller-owned `std::span<std::byte>`, performs exact preflight measurement, and writes directly into that span only when capacity is sufficient. Range writes reuse the same EDP-Memory `ByteOperations` provider contract used by JSON; no CBOR-specific allocator or output owner exists.

## Demo

`demos/cbor-numeric` is maintained in Arduino IDE, PlatformIO Arduino, and PlatformIO ESP-IDF forms. The PlatformIO-format projects are built with PIOArduino. Each form round-trips the same identified schema through both Known-Type Body and Typed Envelope roots while demonstrating integer, text-string, byte-string, canonical Field ordering, and transactional decoding.
