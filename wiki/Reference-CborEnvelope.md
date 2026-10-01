# Reference — CBOR Typed Envelope

**Source:** `src/serialisation/CborEnvelope.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header implements the explicit CBOR `RootProfile::TypedEnvelope` layer. It is deliberately a thin wrapper over the Numeric CBOR body codec.

## `EncodeCborTypedEnvelope<TSink,TValue>`

Requires an identified root and emits exactly a definite three-element array: version `1`, an exact 8-byte `TypeIdentifier` byte string, then the existing CBOR body. No nested TypeIdentifier, runtime Type registry, schema, or codec tag is introduced.

## `DecodeCborTypedEnvelope<TPopulate,TStrictness,TParserLimits,TValue>`

Requires the exact three-element shape, validates version `1`, requires an eight-byte identifier byte string, compares those bytes with the compile-time target identity, then delegates body validation/population to `DecodeCborValue`. Unsupported version and identity mismatch remain distinct outcomes.

Because the public operation runs this routine in validation mode before population mode, envelope metadata failures cannot mutate the destination.
