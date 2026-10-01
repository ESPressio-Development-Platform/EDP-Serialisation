# Reference — Operations

**Source:** `src/serialisation/Operations.hpp`
**Public classification:** PUBLIC OPERATION API
**Detail classification:** PRIVATE IMPLEMENTATION

This header exposes the first executable Serialisation operations and maps the private JSON encoder outcomes into the public operation-specific result families.

## `Measure<TCodec, TRootProfile, TFieldProfile, TValue>(value)`

PUBLIC API. `TCodec` selects the compile-time codec; `TRootProfile` defaults to `KnownTypeBody`; `TFieldProfile` defaults to `Numeric`; `TValue` must satisfy `SerialisableType`.

The implemented combination is currently `Json + KnownTypeBody + Numeric`. Other profile selections are rejected at compile time. The function traverses the complete source through `JsonCountingSink`, validates all represented values/adaptations, and returns exact `RequiredBytes` only on success. It allocates no output buffer and mutates no source state itself.

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

PRIVATE IMPLEMENTATION consteval profile gate. `TCodec`, `TRootProfile`, and `TFieldProfile` are compile-time selection parameters. It currently requires exact `Json`, `KnownTypeBody`, and `Numeric`; pending combinations fail with focused `static_assert` diagnostics rather than runtime error/fallback state.

## Validation relationships

Host coverage checks exact measurement/written-size agreement, scalar/container/schema/adapter output, canonical Field ordering, Optional omission, UTF-8/Base64 behavior, invalid-source preflight, null output, insufficient-capacity preservation, and failed adaptation. PlatformIO Arduino and ESP-IDF demos instantiate the public operations on ESP32.
