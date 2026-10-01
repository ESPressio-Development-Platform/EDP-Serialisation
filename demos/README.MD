# EDP-Serialisation Demos

Each logical demo is supplied in Arduino IDE, PlatformIO Arduino, and PlatformIO ESP-IDF forms.

- `type-qualification` — demonstrates schema-driven `SerialisableType`, enum certification, bounded values, System identifier adaptation and compile-time profile/limit vocabulary.

The foundation demo intentionally does not call JSON/CBOR operations because those operations are not yet implemented in this checkpoint.

## json-numeric-encoding

Builds the implemented JSON + Known-Type Body + Numeric Field encoding path. The schema deliberately declares Fields out of numeric order, omits a disengaged Optional Field, and encodes `Bounded::Bytes` as canonical padded Base64. Expected JSON is `{"2":"sensor-A","7":21.5,"9":"3q2+7w=="}`.
