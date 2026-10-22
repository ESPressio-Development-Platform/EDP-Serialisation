# Statically heterogeneous Tuple Schema Fields

**Scope:** EDP-Serialisation `feature/logging-v1` C++20 additive
support for `std::tuple<T...>` containing only already supported,
owner-certified `SerialisableType` elements. Package remains 0.1.0.

A compile-time heterogeneous `std::tuple` is now a valid **nested**
EDP-System `FieldBinding` value and a Known-Type Body. Its ordered
elements are encoded by the existing CBOR and JSON value implementations,
not private Logging tags. CBOR uses a definite array and JSON uses an
ordered array. The zero-element tuple encodes as CBOR `80` / JSON
`[]`; incorrect lengths fail closed on decode. The established
`SerialisePositionalCbor/Json` owners and a tuple of the same Types
produce byte-identical positional arrays.

The Type concept statically rejects any unsupported element. This
is **not** support for runtime-sized tuples, dynamically allocated
STL containers, `std::variant`, arbitrary reflection or open-union
wire tags. No new TypeIdentifier or FieldIdentifier is issued.

`Measure` checks the complete value before emission.
`Serialise` maintains the existing all-or-nothing preflight
contract, caller-owned destination and no internal heap.
`Deserialise` enforces exact arity and the existing transactional
owner rules for every element: malformed bytes, unsupported
representation and out-of-range scalar produce a typed failure
without accepting a partial record. Strong semantic values still
require explicit owner `CanonicalRepresentation` adapters and
generated schemas remain the sole authority for parameter types.

## Qualification

`tests/host/tuple_schema_fields.cpp` uses actual EDP-System
`FieldSet` at FieldId 13 inside one already-identified root, both
CBOR and JSON Typed Envelope roundtrips, exact positional-byte
equality, wrong arity rejection and an empty tuple golden vector.
`tests/run_host_tests.sh` (GCC and Clang) and
`tests/CMakeLists.txt` include this test.
No vendor encoder or private Logging field-tag lookup is introduced.

Whole 15-field EDP-Logging G4 physical integration is a **separate**
test. Its first 8 KiB ESP32 HIL app stack was inadequate; 16 KiB
test-only stack showed a measured minimum 5,560 B remaining and
3/3 full native WROOM pass on 2026-10-22. This is an application
stack budget issue, **not** an assertion of a universal platform
stack requirement. More complete product resource qualification
remains open.
