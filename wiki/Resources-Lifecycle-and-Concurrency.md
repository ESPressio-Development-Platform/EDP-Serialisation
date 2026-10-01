# Resources, Lifecycle and Concurrency

## Memory/resource behaviour

The current foundation performs no dynamic allocation and owns no runtime buffers. Public traits/concepts are compile-time only. Result structures retain only fixed-size scalar state plus bounded `std::optional` wrappers around System identifier value Types. System identifier adapters copy fixed-size storage values supplied by their callers.

`ParserLimits<>` is compile-time policy metadata; it retains no object state. Later codec slices must consume these bounds without introducing hidden allocation.

## Lifecycle

There is no initialization/shutdown lifecycle, singleton, registry, or retained provider object in the foundation. Every public Type is either compile-time vocabulary, a value result/diagnostic, or a stateless adapter specialization.

## Concurrency and ISR context

The compile-time traits are intrinsically reentrant. The constexpr System identifier conversions mutate only the caller-supplied target reference and retain no shared state, so there is no library synchronization requirement in this slice.

No general ISR guarantee is claimed for future codec operations because those operations do not yet exist. The fixed identifier adapters themselves perform only bounded value copies and no waits/allocations.
