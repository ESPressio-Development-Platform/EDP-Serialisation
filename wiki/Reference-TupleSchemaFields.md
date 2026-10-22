# Tuple Schema Fields (C++20)

**Public:** `SerialisableType<std::tuple<T...>>` with finite,
statically known element Types and all elements already owner-certified.

This additive opt-in capability allows an ordered heterogeneous
positional Field inside an EDP-System `SchemaType` without inventing
numeric element tags. Canonical CBOR is a definite array; JSON is a
fixed ordered array. Empty tuples use CBOR `80` and JSON `[]`.
Incorrect element counts, invalid representations or unsupported
element Types fail closed using the ordinary transactional decoder.
No dynamic allocation, RTTI or runtime type registry is introduced.

See [Tuple Schema Fields](../docs/TUPLE-SCHEMA-FIELDS.MD) for the
wire/ownership contract, status and qualification. Native owner tests
are `tests/host/tuple_schema_fields.cpp`, run under GCC and Clang.
The feature does not certify a generated Localisation diagnostic
message by itself; that check belongs to its consuming Logging adapter.
