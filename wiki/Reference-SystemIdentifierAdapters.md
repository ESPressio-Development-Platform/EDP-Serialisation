# Reference — SystemIdentifierAdapters

**Source:** `src/serialisation/SystemIdentifierAdapters.hpp`
**Classification:** PUBLIC INTEGRATION API

This header integrates EDP-System semantic identifier value Types with the Serialisation strong-Type adaptation contract without adding a dependency from System back to Serialisation.

## `SystemIdentifierConversionResult`

Operation-specific result enum for the fixed identifier conversions.

- `Succeeded` — the complete represented value copied losslessly. There is currently no failure value because each source/target pair spans the same represented storage domain.

## CanonicalRepresentation specializations

Each specialization is PUBLIC INTEGRATION API and contains nested alias `Type`:

| Semantic Type | `Type` surrogate | Meaning |
|---|---|---|
| `System::TypeIdentifier` | `System::TypeIdentifier::Storage` | exact canonical eight-byte Type identity |
| `System::FieldIdentifier` | `System::FieldIdentifier::Storage` | exact one-byte Field identity |
| `System::TypeAuthorityIdentifier` | `System::TypeAuthorityIdentifier::Storage` | exact canonical three-byte Authority identity |
| `System::TypeLocalIdentifier` | `System::TypeLocalIdentifier::Storage` | exact canonical five-byte local identity |

These exact fixed-size storage Types avoid exposing private object layout. They are value surrogates for ordinary Fields; the special Typed Envelope representation has its separately locked JSON-hex / CBOR-byte-string envelope rules when codec support is implemented.

## TypeConversionAdapter specializations

Eight PUBLIC INTEGRATION API specializations provide both directions for the four rows above. For every specialization the member contract is identical:

- `ResultType` — alias of `Serialisation::SystemIdentifierConversionResult`.
- `IsAvailable = true` — announces the pairwise conversion to generic BoundedTypes consumers.
- `IsNoexcept = true` — certifies the conversion for Serialisation canonical adaptation.
- `IsSuccessful(ResultType result)` — pure predicate; returns true exactly for `Succeeded`.
- `Convert(const Source&, Target&)` — copies the represented value into caller-owned target and returns `Succeeded`. It performs no allocation, wait, I/O, shared-state mutation, registry access, or hidden lifetime transfer.

### TypeIdentifier → Storage

`Convert` assigns `source.Bytes()` to the eight-byte target. The source remains unchanged.

### Storage → TypeIdentifier

`Convert` constructs `System::TypeIdentifier{source}` and assigns it to caller target. Every eight-byte Storage domain value has a representable TypeIdentifier object; semantic validity requirements for using an identifier as a schema identity remain owned by EDP-System, not this value conversion.

### FieldIdentifier → Storage

`Convert` assigns `source.Value()` to the one-byte target. Every 0..255 FieldIdentifier value is retained exactly, including zero.

### Storage → FieldIdentifier

`Convert` constructs the FieldIdentifier from the storage byte; every byte belongs to the FieldIdentifier represented domain.

### TypeAuthorityIdentifier → Storage / reverse

Forward conversion copies `source.Bytes()`; reverse constructs the Authority identifier from exact three-byte storage. The adapter does not reinterpret the storage as a host-endian integer.

### TypeLocalIdentifier → Storage / reverse

Forward conversion copies `source.Bytes()`; reverse constructs the local identifier from exact five-byte storage. The adapter does not reinterpret the storage as a host-endian integer.

## Ownership and concurrency

All target objects are supplied by the caller. No adapter retains references after return. The operations are constexpr/noexcept fixed-size value copies, require no synchronization, and introduce no per-object Serialisation state.

## Validation

Host tests exercise TypeIdentifier forward conversion plus generic success interpretation and statically prove all four semantic identifier Types qualify through the canonical-adaptation path where applicable.
