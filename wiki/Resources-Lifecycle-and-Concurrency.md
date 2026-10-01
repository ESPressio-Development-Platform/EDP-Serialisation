# Resources, Lifecycle and Concurrency

## Memory/resource behaviour

The current implementation performs no dynamic allocation and owns no retained runtime buffers. `Measure` uses a counter only. `Serialise` writes directly to caller-owned contiguous bytes after preflight measurement. Encoder temporaries are fixed-size stack values such as numeric formatting buffers and canonical strong-Type surrogates. Byte-range output uses the selected EDP-Memory ByteOperations provider, defaulting to the stateless portable provider; no provider object is retained. Result structures retain only fixed-size scalar state plus bounded `std::optional` wrappers around System identifier value Types.

`Deserialise` consumes `ParserLimits<>` directly. Validation/population retain immutable cursors, fixed number/skip state and a 32-byte Field presence bitmap. Unknown-object duplicate detection replays prior members instead of retaining an unbounded text-key table.

## Lifecycle

There is no initialization/shutdown lifecycle, singleton, registry, or retained provider object. Encoding and decoding operations are ordinary reentrant calls over caller-owned values/buffers. Decode performs two passes over immutable replayable input and mutates the destination only during pass two.

## Concurrency and ISR context

The compile-time traits are intrinsically reentrant. The constexpr System identifier conversions mutate only the caller-supplied target reference and retain no shared state, so there is no library synchronization requirement in this slice.

No ISR-safety guarantee is claimed for JSON codec operations because floating `std::to_chars`/`std::from_chars` and generic user adapters are not specified as ISR-safe platform services. The encoder performs no waits or internal allocation and retains no shared mutable state.

## Flash footprint

On the current Xtensa GCC/libstdc++ toolchain, first instantiation of floating `std::to_chars` pulls substantial Ryu lookup tables. AI-AGENT-02 measured roughly 122-126 KiB additional flash in the small JSON encoding demo while RAM remained effectively unchanged. This is a correctness-preserving baseline tracked by `EDP-Serialisation#2`, not retained runtime memory.

## JSON decoder footprint baseline

The JSON Numeric decoder demo on AI-AGENT-02 measured 304,460 bytes flash / 22,260 bytes RAM under PlatformIO Arduino and 220,953 bytes flash / 12,616 bytes RAM under PlatformIO ESP-IDF. These are complete demo-image figures, not retained parser-state sizes. The parser itself retains fixed stack/caller-owned state only; no proportional input buffer or DOM is allocated.
