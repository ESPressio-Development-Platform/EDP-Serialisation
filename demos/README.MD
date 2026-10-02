# EDP-Serialisation Demos

Each logical demo is supplied in Arduino IDE, PlatformIO Arduino, and PlatformIO ESP-IDF forms. ESPressio recommends PIOArduino Core for both PlatformIO-format variants; the checked-in project files select the maintained PIOArduino platform package and C++20 baseline.

- `type-qualification` — demonstrates schema-driven `SerialisableType`, enum certification, bounded values, System identifier adaptation and compile-time profile/limit vocabulary.

## json-numeric-encoding

Builds the implemented JSON + Known-Type Body + Numeric Field encoding path. The schema deliberately declares Fields out of numeric order, omits a disengaged Optional Field, and encodes `Bounded::Bytes` as canonical padded Base64. Expected JSON is `{"2":"sensor-A","7":21.5,"9":"3q2+7w=="}`.

## json-numeric-decoding

Builds the implemented transactional JSON + Known-Type Body + Numeric Field decoder. The input uses arbitrary schema Field order, decodes String/float/Base64 Bytes, and omits an Optional Field so the destination becomes disengaged only after complete validation succeeds.

## json-typed-envelope

Builds the implemented JSON Typed Envelope profile over Numeric Fields. It emits the canonical `$edp` metadata object with envelope version `1` and the root Type's 16-character lowercase `TypeIdentifier`, then transactionally decodes the same caller-owned bytes back into the identified schema. The demo intentionally avoids floating-point so its footprint isolates envelope/root-metadata behavior.


## json-localised-text

Builds the JSON LocalisedText Field profile using a compact resolver-compatible provider over the real EDP-Localisation public vocabulary. It emits `{"RFC5646":"en-GB","Temperature":23}` into caller-owned bytes, then transactionally reverse-resolves `Temperature` back to FieldIdentifier `0`. The permanent host suite separately exercises the same Serialisation profile against EDP-Localisation's real generated binary-pack Resolver/fallback implementation.


## cbor-numeric

Builds deterministic CBOR Numeric execution for an identified schema under the explicit Typed Envelope profile. The schema deliberately declares Fields out of numeric order, encodes `Bounded::String` as CBOR text and `Bounded::Bytes` as CBOR bytes, then transactionally decodes the caller-owned representation back into the same compile-time Type.
