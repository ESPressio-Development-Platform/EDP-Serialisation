# Reference — CBOR Encoding

**Source:** `src/serialisation/CborEncoding.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header implements deterministic V1 Numeric CBOR encoding over the same `SerialisableType` universe used by JSON.

## Outcome and primitive helpers

`CborEncodingStatus` distinguishes success, resource/capacity failure, non-finite floating values, invalid UTF-8, and strong-adaptation failure. `MapCborSinkWriteResult`, `WriteCborByte`, and `WriteCborBytes` adapt the shared codec-neutral sink contract.

`EncodeCborHead` emits one major-type head with the shortest legal additional-information width. `EncodeCborInteger` maps signed/unsigned fixed-width values to major types 0/1. `EncodeCborFloating` emits exact binary32 or binary64 bit patterns only. `EncodeCborTextString` validates UTF-8 and emits major type 3; `EncodeCborByteString` emits major type 2.

## Sequence and schema traversal

`EncodeCborArray` emits a definite-length ordered array. `CountCborSchemaFields` counts represented schema Fields after Optional omission. `EncodeCborSchemaFields<TIdentifier>` probes the finite 0..255 FieldIdentifier domain and emits represented Fields in ascending semantic identifier order. `EncodeCborSchema` emits the definite-length map head followed by those entries.

## `EncodeCborValue`

Central recursive dispatcher for bool, fixed-width integers, binary32/binary64, certified enums, Optional, fixed arrays, `Bounded::String`, `Bounded::Bytes`, `Bounded::Vector`, System schema Types, and one-hop canonical strong Types. Disengaged root/sequence Optional emits null. Strong adaptation reuses the success-aware `EDP-BoundedTypes::TypeConversionAdapter` contract.

No DOM, schema registry, heap allocation, host object-layout encoding, or declaration-order wire dependence exists.
