# Reference — Optional

**Source:** `src/serialisation/Optional.hpp`
**Classification:** PUBLIC TYPE ALIAS

## `Optional<TValue>`

Exact alias of `std::optional<TValue>`. It provides ESPressio vocabulary without introducing another container Type, ABI, state representation, or allocation policy.

**Template parameter**

- `TValue` — contained value Type; it must recursively satisfy V1 serialisability when the Optional itself is qualified.

A directly nested `Optional<Optional<T>>` is rejected by `SerialisableType` because omission/null/value cannot preserve all three nested logical levels consistently across object Fields and positional roots/sequences. Non-nested Optional remains supported.

The later codec contract uses omission for disengaged object Fields and codec null for root/sequence positions; those runtime operations are not implemented in this foundation checkpoint.
