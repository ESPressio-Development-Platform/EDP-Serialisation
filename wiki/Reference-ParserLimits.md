# Reference — ParserLimits

**Source:** `src/serialisation/ParserLimits.hpp`
**Classification:** PUBLIC POLICY API

## `ParserLimits<TMaximumNestingDepth,TMaximumSkippedContainerItems>`

Zero-state compile-time policy describing parser resources which later deserialisation operations must enforce without mutable global configuration.

**Template parameters**

- `TMaximumNestingDepth` — maximum syntactic object/array/container nesting accepted by the codec. Default: `32`.
- `TMaximumSkippedContainerItems` — maximum container items traversed solely while structurally skipping unknown data. Default: `1024`.

**Members**

- `MaximumNestingDepth` — exact compile-time copy of `TMaximumNestingDepth`; affects parser scratch/resource planning but adds no object storage.
- `MaximumSkippedContainerItems` — exact compile-time copy of the unknown-skip bound.

## `DefaultParserLimits`

Alias of `ParserLimits<>`, therefore selecting depth `32` and skipped-item bound `1024`.

These limits do not replace Type-owned capacities such as `Bounded::String<N>`, `Bytes<N>`, `Vector<T,N>`, or fixed-array extents. The current foundation only publishes the policy vocabulary; codec enforcement arrives with deserialisation implementation.
