# Resources, Lifecycle and Concurrency

## Memory/resource behaviour

The current implementation performs no dynamic allocation and owns no retained runtime buffers. `Measure` uses a counter only. `Serialise` writes directly to caller-owned contiguous bytes after preflight measurement. Encoder temporaries are fixed-size stack values such as numeric formatting buffers and canonical strong-Type surrogates. Byte-range output uses the selected EDP-Memory ByteOperations provider, defaulting to the stateless portable provider; no provider object is retained. Result structures retain only fixed-size scalar state plus bounded `std::optional` wrappers around System identifier value Types.

`ParserLimits<>` is compile-time policy metadata; it retains no object state. Later codec slices must consume these bounds without introducing hidden allocation.

## Lifecycle

There is no initialization/shutdown lifecycle, singleton, registry, or retained provider object. Encoding operations are ordinary reentrant calls over caller-owned values/buffers.

## Concurrency and ISR context

The compile-time traits are intrinsically reentrant. The constexpr System identifier conversions mutate only the caller-supplied target reference and retain no shared state, so there is no library synchronization requirement in this slice.

No ISR-safety guarantee is claimed for JSON encoding because floating `std::to_chars` and generic user adapters are not specified as ISR-safe platform services. The encoder performs no waits or internal allocation and retains no shared mutable state.

## Flash footprint

On the current Xtensa GCC/libstdc++ toolchain, first instantiation of floating `std::to_chars` pulls substantial Ryu lookup tables. AI-AGENT-02 measured roughly 122-126 KiB additional flash in the small JSON encoding demo while RAM remained effectively unchanged. This is a correctness-preserving baseline tracked by `EDP-Serialisation#2`, not retained runtime memory.
