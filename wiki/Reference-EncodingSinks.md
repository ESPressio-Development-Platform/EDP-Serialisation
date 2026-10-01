# Reference — Encoding Sinks

**Source:** `src/serialisation/EncodingSinks.hpp`
**Classification:** PRIVATE IMPLEMENTATION

This header owns codec-neutral output sinks shared by JSON and CBOR. Consumers never instantiate these Types directly; public encoding is exposed through `Measure` and `Serialise`.

## `EncodingSinkWriteResult`

Internal sink outcome: `Succeeded` or `CapacityExceeded`.

## `EncodingCountingSink`

Zero-output exact measurement sink. `_size` is the only retained member. `WriteByte` and `WriteBytes` perform overflow-safe size accounting without dereferencing source ranges; `Size` returns the exact accumulated count.

## `EncodingBufferSink<TByteOperationsProvider>`

Caller-buffer sink. `TByteOperationsProvider` must satisfy the EDP-Memory ByteOperations provider contract, be stateless, and be nothrow default-constructible. `_output`, `_capacity`, and `_size` retain only caller-owned output state for one traversal. `WriteByte` performs one bounded assignment; `WriteBytes` delegates complete range copies to the selected provider; `Size` reports committed bytes.

The default provider is `EDP-Platform-Portable::Memory::ByteOperationsProvider`. No provider object, allocator, registry, or dynamic buffer is retained.
