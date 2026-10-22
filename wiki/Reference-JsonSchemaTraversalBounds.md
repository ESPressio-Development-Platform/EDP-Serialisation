# JSON Schema Traversal Bounds

The owner JSON Typed Envelope encoder visits EDP-System schema field
bindings in ascending FieldIdentifier order. Its compile-time
`Detail::MaximumDeclaredJsonFieldIdentifier<TValue>` finds the largest
issued ID using `FieldsOf<T>::ForEach` and ends the writer traversal
there; it no longer instantiates empty levels up to 255 for every
schema. Sparse, unordered declarations and max ID 255 remain supported.

No wire format, type identifier, allocation, or mandatory dependencies
changed. Existing GCC/Clang regression and compile-fail tests passed.
With a fixed **14 KiB** ESP32-D0WD test task stack, physical
15-field LogRecord CBOR/JSON HIL testing (3 independent boots) passed
and cumulative minimum stack remaining after JSON rose from
**3,512 to 10,472 bytes**, while full-app flash use fell from
**388,513 to 378,789 bytes** and static RAM stayed at 21,480 bytes.
These are test-application measurements, not universal library cost
figures. See [complete evidence](../docs/JSON-SCHEMA-TRAVERSAL-BOUNDS.MD)
and [issue #10](https://github.com/ESPressio-Development-Platform/EDP-Serialisation/issues/10).
