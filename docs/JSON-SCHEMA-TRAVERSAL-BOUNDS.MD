# JSON schema traversal bounded by declared maximum FieldIdentifier

**Scope:** `EDP-Serialisation/feature/logging-v1` implementation of
[owner issue #10](https://github.com/ESPressio-Development-Platform/EDP-Serialisation/issues/10),
with regression dependency on the existing schema/tuple owner contract (#9).

## Implementation and correctness boundary

The owner JSON writer previously recursively instantiated
`EmitJsonSchemaFieldsAscending` through numeric identifier **255**
irrespective of the actual EDP-System `FieldSet` declarations.
For the fifteen-field LogRecord Typed Envelope (root FieldIds 0–14),
that creates a long unnecessary recursive call chain and stack demand.

`Detail::MaximumDeclaredJsonFieldIdentifier<TValue>` now computes the
actual maximum FieldIdentifier at **compile time** by visiting
`System::FieldsOf<TValue>::ForEach`. The ascending writer stops after
that high-water ID, not 255. This preserves:
- source-defined FieldIdentifiers, all 0–255 values, sparse declarations,
  declaration order independence and canonical ascending numeric output;
- unchanged JSON representations, parsing, result status and validation;
- existing `EDP-System` schema metadata and no Logging dependency;
- no heap, additional runtime allocation, mutable shared state or
  altered API/version.

No compression, omission of *declared* fields, new type registration,
runtime field indexing, wire versioning or change of private C++ ABI
was introduced. For a schema whose max ID is 255, the previous complete
traversal range remains available.

## Regression tests

The full existing EDP-Serialisation owner tests were re-run under GCC
and Clang, including all negative compile-fail expectations and the
preexisting negative tests for unregistered and incorrectly
underlying-width-declared enum types. The small new static assertions
check high-water bounds **9**, **7**, **255**, **255**, respectively, for
Nested, Payload, BoundaryFields and SparseOptionalFields; the last two
guard sparse/unsorted schemas and the maximum legal identifier.

RPI400 detached
`20261022T123157-67f0fdca`: **PASS exit 0** (GCC+Clang),
including complete C++20 code generation, positional encoding,
schema-field JSON/CBOR, identity adapters and compile-fail tests.
Earlier `20261022T120527-937880f1` also passed both compilers;
the **final** run restores the enum negative tests omitted from an
intermediate source fixture and therefore supersedes it.

## Fixed 14 KiB ESP32-WROOM physical comparison

Both images used 14,336 bytes of app_main test-task stack and the
same physical WROOM-32 / PioArduino ESP-IDF full 15-field generated
LogRecord Typed Envelope fixture, with its existing full CBOR/JSON
roundtrips, 18 canonical HIL markers, EDLG binary/CRC, G11 resource
and native/NVS-restart checks.

| Metric | Before owner bound | After owner bound |
|---|---:|---:|
| Flash used by whole fixture | 388,513 B | 378,789 B |
| Static RAM used by whole fixture | 21,480 B | 21,480 B |
| Stack remaining after source projection | 11,208 B | 11,208 B |
| Stack remaining after CBOR encode/decode | 10,536 B | 10,536 B |
| Stack remaining after JSON encode/decode | **3,512 B** | **10,472 B** |
| Whole-firmware factory SHA256 | `acb6aeee53f64852f2c467abc95b4fc19d05f93785369a8908089efceb8c3f56` | `6ab5745e6ffc587c1c5ca6480e27a11447a6ac0bccc72e6baa6c483f3c3ba572` |

The new image was built in detached run
`20261022T120813-3c70bd12` (**PASS exit0**).
After a chat interruption it was recovered and verified by unchanged
SHA256 before actual independent physical flash and on-chip
`verify-flash`. Detached
`20261022T122616-a4d5f0a3` **PASS exit0** recorded all
three new independent reset/NVS-restart UART cycles, each
**18/18** and all five ordered stack-phase probes, with minimum
remaining 10,472 B. The private captures are retained at
`/srv/espressio/scratch/gpt6-logging-20261022-0919-v4/g4-whole-record-physical-14k-jsonbounded-recovery-20261022`;
SHA256 of the capture-hash manifest is
`cb39d128a142951efdb8a25954b2acac95c408dab20a2c10ed7f82aac396727b`.
No raw captures or firmware are added to source control.

These are whole-test-firmware resource differences on **one** ESP32
toolchain, not general library-only flash/stack figures, and stack
phase measurements are cumulative FreeRTOS high-water marks rather
than isolated function-frame sizes. The improvement therefore
demonstrates a concrete reduction in this configuration, not a
universal production stack budget or deterministic deadline proof.

## Remaining boundaries

This optimization supports the full canonical G4 Typed Envelope but
does **not** prove complete Logging G1–G15 application integration:
power-interruption durability, parallel persistent sink handoff,
all native contention/wear limits, end-to-end historical issuer
authenticity, full application product budgets and cross-platform
flash measurements remain separate open release gates. No library
version mutation or main-branch merge was performed.
