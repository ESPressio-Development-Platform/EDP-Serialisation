# Reference — JSON Encoding

**Source:** `src/serialisation/JsonEncoding.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header is the allocation-free recursive JSON encoder used by the public Measure/Serialise operations. No declaration in this file is a cross-repository compatibility surface; consumers use `Operations.hpp` through the public umbrella.

## Internal outcome Types

### `JsonSinkWriteResult`

- `Succeeded` — requested bytes were counted or retained completely.
- `CapacityExceeded` — size accounting overflowed or the selected sink lacked capacity.

### `JsonEncodingStatus`

- `Succeeded` — complete value validation/emission succeeded.
- `ResourceLimitExceeded` — sink/size accounting could not represent the output.
- `NonFiniteNumber` — float/double was NaN or infinity.
- `InvalidUtf8` — bounded String failed V1 UTF-8 validation.
- `AdaptationFailed` — forward canonical Type conversion reported non-success.

### `Utf8ValidationStatus`

- `Succeeded` — complete payload is valid V1 UTF-8.
- `InvalidUtf8` — malformed UTF-8 or forbidden U+0000 was encountered.

### `Utf8ValidationResult`

Members:

- `Status` — authoritative validation outcome;
- `ByteOffset` — first invalid source byte offset, zero on success.

`IsSuccessful()` is the Boolean predicate testing `Status == Succeeded`.

## Sink Types

### `JsonCountingSink`

Zero-output measurement sink. Private member `_size` is the authoritative exact byte count. `WriteByte` increments with `size_t` overflow protection; `WriteBytes` accounts for a complete range with overflow protection; `Size` reports the count. The source pointer supplied to `WriteBytes` is deliberately not dereferenced because measurement retains no encoded bytes.

### `JsonBufferSink<TByteOperationsProvider>`

`TByteOperationsProvider` must satisfy the EDP-Memory ByteOperations contract, be stateless, and be nothrow default-constructible; it defaults to `Platform::Portable::Memory::ByteOperationsProvider`. Caller-buffer emission sink. Private members `_output`, `_capacity`, and `_size` retain only the caller pointer/capacity and current committed position for one traversal. The constructor binds that storage. `WriteByte` performs one bounded direct byte assignment. `WriteBytes` delegates the complete non-overlapping range to the compile-time-selected EDP-Memory `CopyBytes` provider. `Size` reports committed bytes. The sink owns no storage, retains no provider object, and performs no allocation.

## Family-detection traits

`StandardArrayTraits`, `OptionalValueTraits`, `BoundedStringTraits`, `BoundedBytesTraits`, and `BoundedVectorTraits` each have a false primary template and an exact specialization for the supported family. Their `IsValue` member drives compile-time dispatch. Array/Optional/Vector specializations additionally expose element metadata (`Element`, and array `Count`) needed by recursive traversal.

## `FieldBindingForIdentifier<TFieldSet, TIdentifier>`

Compile-time FieldSet search used to impose canonical wire order without retaining runtime sorting state. `TFieldSet` is the canonical System schema and `TIdentifier` is one candidate numeric identity. The empty-set specialization exposes `Type = void`; the recursive specialization exposes the matching FieldBinding or recurses. JSON schema emission probes the bounded one-byte identifier domain from 0 through 255.

## Common sink helpers

- `MapSinkWriteResult` translates sink failure into `ResourceLimitExceeded` and records the current sink byte position.
- `WriteJsonByte` appends one byte through a selected sink.
- `WriteJsonBytes` appends one character range through a selected sink.

All are private stateless templates parameterized by sink Type.

## `ValidateUtf8(source, length)`

Validates exactly `length` bytes without null scanning. It accepts legal UTF-8 scalar encodings and rejects embedded U+0000, bad continuation bytes, overlong forms, UTF-16 surrogate encodings, values above U+10FFFF, and incomplete sequences. It returns the first invalid source offset and performs no normalization.

## String and byte encoders

`EncodeJsonString` validates UTF-8 first, then emits surrounding quotes, native valid non-ASCII bytes, deterministic JSON syntax/short escapes, and lowercase `\\u00xx` for remaining ASCII controls.

`EncodeJsonBase64` maps arbitrary octets to standard RFC 4648 Base64 using the canonical alphabet and mandatory `=` padding. It emits an ordinary JSON String, including `""` for empty bytes. No prefix/wrapper is introduced.

## Numeric encoders

`EncodeJsonInteger<TSink,TValue>` uses fixed stack text storage plus C++20 `std::to_chars` after promotion to exact 64-bit signed/unsigned domains. Output is canonical base-10.

`EncodeJsonFloating<TSink,TValue>` rejects non-finite values, emits explicit `0`/`-0` spellings, and otherwise uses C++20 `std::to_chars` shortest round-tripping decimal. The current Xtensa libstdc++ implementation has a material one-time flash cost because of its Ryu lookup tables; this is tracked by `EDP-Serialisation#2`.

`EncodeJsonFieldKey` emits one numeric System FieldIdentifier as quoted canonical decimal followed by `:`.

## Recursive traversal

`EncodeJsonArray` emits a finite indexed source as `[...]`, placing commas deterministically and recursively invoking `EncodeJsonValue` for each element.

`EncodeJsonSchemaFields<TIdentifier>` probes Field identifiers in ascending numeric order. For a present binding it omits disengaged Optional object Fields; otherwise it emits comma/key/value and records Type/Field diagnostic identity on failure. It recursively advances through the finite 0..255 identity domain.

`EncodeJsonSchema` writes braces and delegates Field emission to the ascending identifier traversal.

`EncodeJsonValue` is the central recursive dispatcher over the complete currently-qualified V1 universe:

- bool;
- fixed-width integers;
- float/double;
- certified enums;
- Optional;
- `std::array` and native fixed arrays;
- `Bounded::String`;
- `Bounded::Bytes`;
- `Bounded::Vector`;
- System schema Types;
- one-hop canonical strong Types.

Strong Types allocate no heap state: the function nothrow-default-constructs the canonical surrogate on the stack, invokes the existing success-aware BoundedTypes adapter, then recursively encodes the surrogate. Classification guarantees that representation/adapter contract before this path can instantiate.

## Invariants

- no DOM, runtime registry, dynamic allocation, hidden ownership, or mutable global state;
- no declaration-order wire dependence for schema Fields;
- String and Bytes remain semantically distinct despite both using JSON String tokens;
- measurement and output use the same traversal implementation through different sinks;
- caller-owned output is entered only after complete public preflight measurement.
