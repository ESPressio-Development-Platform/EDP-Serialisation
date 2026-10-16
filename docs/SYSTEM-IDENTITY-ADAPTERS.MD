# G3 System Identity adapters

EDP-Serialisation 0.1.0 feature/logging-v1 now provides reversible canonical adapters for the existing EDP-System `Identity::DeviceIdentifier` (exact 16-byte Bounded::Bytes with CBOR byte-string / JSON Base64) and `Identity::RuntimeIncarnationId` (uint32 CBOR unsigned / JSON integer). These use owner-defined storage and its exact conversion contract, with no new library dependency or independent wire-format encoder.

A reverse DeviceIdentifier adapter rejects a bounded byte value unless its logical length is exactly 16, without modifying the target. Both identity-zero values remain representable but INVALID at the System identity level; codec round-trip is never evidence that an identity has been durably assigned. No ClockEra, field-family-width Localisation StringIdentifier or governed root LogRecord TypeIdentifier is fabricated.

The host suite tests both directions, exact canonical 23-byte CBOR and Base64 JSON, and preflight transactional rejection, under GCC and Clang including existing negative compilation cases. The package remains version 0.1.0. Physical HIL and complete G3/G4 are separate qualification gates.
