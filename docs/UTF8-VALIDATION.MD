# Strict UTF-8 validation — consuming and expanding EDP-Serialisation

**Version:** 0.1.0, current working branch `feature/logging-v1`.

## Consuming developers

The opt-in header `<serialisation/Utf8Validation.hpp>` provides `ESPressio::Serialisation::ValidateUtf8(const char*, std::size_t)`. Its dependency surface consists only of the standard `<cstdint>` and `<cstddef>` headers. It neither creates a runtime, nor needs Memory, Localisation, a codec, a platform provider or an SDK.

The `constexpr noexcept` operation returns `Utf8ValidationResult`, containing `Utf8ValidationStatus` and the zero-based byte offset of the first rejected byte, plus the `IsSuccessful()` predicate. For a non-empty input, the caller must supply a readable byte range. No heap allocation occurs.

**The existing strict V1 text rule is unchanged.** Valid Unicode scalar sequences succeed; embedded U+0000, invalid lead/continuation bytes, overlong sequences, truncation, surrogate code points and values above U+10FFFF fail. No Unicode normalisation or domain-specific language qualification is performed. NUL-containing binary diagnostic arguments must use an explicitly schema-qualified bytes representation, not canonical text.

Existing JSON/CBOR encoding and decoding still use the exact same underlying validation algorithm. The new header is an extraction, not a permissive validation mode. The default Logging umbrella does not import it; only Logging's opt-in owned-argument module requires this Serialisation owner.

## Library expansion

The existing internal Detail-qualified type/function names remain visible as using-declarations in JsonEncoding.hpp, forwarding to the same single implementation with no duplicated logic or changed result.

The pre-existing validation algorithm formerly in `JsonEncoding.hpp` is now the single owner implementation in `Utf8Validation.hpp`. Its typed result and byte-offset semantics are unchanged, and making the function `constexpr` enables compile-time checking without a second validator. Do not widen its default text domain, change the returned error offsets, or introduce duplicate source-side validation; perform design review and cross-library negative/golden tests before any change of canonical text policy.

Regressions must cover standalone header compilation, malformed and valid UTF-8, NUL at non-zero offsets, complete JSON/CBOR round trips and negative wire inputs, and generated consumers in Logging, Mesh, Command, Event and State. The new public reusable component must be discoverable in Policy V2's Component Reuse catalogue, with its feature-branch status explicitly distinguished from `main`.
