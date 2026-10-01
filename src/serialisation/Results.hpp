#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include <ESPressio_System.hpp>

namespace ESPressio::Serialisation {

    /// Outcomes specific to exact encoded-size measurement.
    enum class MeasurementStatus : std::uint8_t {
        /// Measurement and source validation completed successfully.
        Succeeded = 0U,
        /// Caller-supplied operation arguments do not satisfy the selected profile contract.
        InvalidArgument = 1U,
        /// The selected compile-time resource policy cannot accommodate the represented structure.
        ResourceLimitExceeded = 2U,
        /// Localised Field representation metadata is missing or invalid.
        InvalidLanguageMetadata = 3U,
        /// Two schema Fields would produce the same textual representation key.
        FieldNameCollision = 4U,
        /// Required Localisation resolution could not be completed conclusively.
        LocalisationFailure = 5U,
        /// A floating-point value is NaN or infinite and therefore outside the V1 domain.
        NonFiniteNumber = 6U,
        /// A bounded String contains bytes which are not valid V1 UTF-8 text.
        InvalidUtf8 = 7U,
        /// Canonical strong-Type conversion could not produce the representation value.
        AdaptationFailed = 8U
    };

    /// Outcomes specific to writing one encoded representation.
    enum class SerialisationStatus : std::uint8_t {
        /// Source validation and output emission completed successfully.
        Succeeded = 0U,
        /// Caller-supplied operation arguments do not satisfy the selected profile contract.
        InvalidArgument = 1U,
        /// Caller-owned output storage is smaller than the exact measured representation size.
        OutputBufferTooSmall = 2U,
        /// The selected compile-time resource policy cannot accommodate the represented structure.
        ResourceLimitExceeded = 3U,
        /// Localised Field representation metadata is missing or invalid.
        InvalidLanguageMetadata = 4U,
        /// Two schema Fields would produce the same textual representation key.
        FieldNameCollision = 5U,
        /// Required Localisation resolution could not be completed conclusively.
        LocalisationFailure = 6U,
        /// A floating-point value is NaN or infinite and therefore outside the V1 domain.
        NonFiniteNumber = 7U,
        /// A bounded String contains bytes which are not valid V1 UTF-8 text.
        InvalidUtf8 = 8U,
        /// Canonical strong-Type conversion could not produce the representation value.
        AdaptationFailed = 9U
    };

    /// Outcomes specific to validating and populating one decoded value.
    enum class DeserialisationStatus : std::uint8_t {
        /// Complete input validation and destination population completed successfully.
        Succeeded = 0U,
        /// Caller-supplied operation arguments do not satisfy the selected profile contract.
        InvalidArgument = 1U,
        /// Input bytes do not form a structurally valid selected-codec representation.
        MalformedRepresentation = 2U,
        /// A complete root value was followed by disallowed trailing input.
        TrailingData = 3U,
        /// The selected compile-time resource policy was exceeded while validating input.
        ResourceLimitExceeded = 4U,
        /// Typed-envelope metadata uses a version not understood by V1.
        UnsupportedEnvelopeVersion = 5U,
        /// Typed-envelope semantic identity does not equal the compile-time target Type.
        TypeIdentifierMismatch = 6U,
        /// Exact strictness encountered a structurally valid but unknown Field.
        UnknownField = 7U,
        /// The representation contains the same logical Field/key more than once.
        DuplicateField = 8U,
        /// A required non-Optional schema Field is absent.
        MissingRequiredField = 9U,
        /// Embedded or caller-provided RFC5646 metadata is missing, malformed, or inconsistent.
        InvalidLanguageMetadata = 10U,
        /// Textual Field identity was conclusively absent from the applicable Localisation domain.
        FieldNameNotFound = 11U,
        /// Textual Field identity resolves to more than one distinct FieldIdentifier.
        FieldNameAmbiguous = 12U,
        /// Required Localisation resolution could not be completed conclusively.
        LocalisationFailure = 13U,
        /// Encoded value category does not match the target schema value category.
        TypeMismatch = 14U,
        /// Numeric input lies outside the target fixed-width numeric domain.
        NumericOutOfRange = 15U,
        /// A non-zero textual floating value underflows completely to zero.
        NumericUnderflow = 16U,
        /// Floating input is NaN or infinite and therefore outside the V1 domain.
        NonFiniteNumber = 17U,
        /// Decoded text is not valid V1 UTF-8 or cannot be retained by Bounded::String.
        InvalidUtf8 = 18U,
        /// JSON byte text is not strict canonical padded RFC 4648 Base64.
        InvalidBase64 = 19U,
        /// Decoded bounded String, Bytes, or Vector content exceeds target capacity.
        CapacityExceeded = 20U,
        /// Canonical representation conversion into the semantic target Type failed.
        AdaptationFailed = 21U
    };

    /// Bounded common diagnostic facts which may accompany a failed operation.
    struct Diagnostic final {

        // Immediate failure location.

        /// Input/output byte offset most directly associated with the failure.
        std::size_t ByteOffset = 0U;

        /// Immediate semantic Type identity when the failure can identify one.
        std::optional<System::TypeIdentifier> Type{};

        /// Immediate Type-local Field identity when the failure can identify one.
        std::optional<System::FieldIdentifier> Field{};

    };

    /// Exact result of one Measure operation.
    struct MeasurementResult final {

        // Operation outcome.

        /// Strongly typed measurement outcome.
        MeasurementStatus Status = MeasurementStatus::Succeeded;

        // Size accounting.

        /// Exact encoded bytes required when measurement succeeds.
        std::size_t RequiredBytes = 0U;

        // Failure context.

        /// Compact diagnostic facts associated with the result.
        Diagnostic Detail{};

        // Outcome predicates.

        /// Reports whether exact measurement completed successfully.
        ///
        /// @return true only when Status is MeasurementStatus::Succeeded.
        [[nodiscard]] constexpr bool IsSuccessful() const noexcept {
            return Status == MeasurementStatus::Succeeded;
        }

    };

    /// Exact result of one Serialise operation.
    struct SerialisationResult final {

        // Operation outcome.

        /// Strongly typed serialisation outcome.
        SerialisationStatus Status = SerialisationStatus::Succeeded;

        // Size accounting.

        /// Exact bytes required for the complete representation.
        std::size_t RequiredBytes = 0U;

        /// Bytes committed to caller output; remains zero on every non-success result.
        std::size_t BytesWritten = 0U;

        // Failure context.

        /// Compact diagnostic facts associated with the result.
        Diagnostic Detail{};

        // Outcome predicates.

        /// Reports whether output emission completed successfully.
        ///
        /// @return true only when Status is SerialisationStatus::Succeeded.
        [[nodiscard]] constexpr bool IsSuccessful() const noexcept {
            return Status == SerialisationStatus::Succeeded;
        }

    };

    /// Exact result of one Deserialise operation.
    struct DeserialisationResult final {

        // Operation outcome.

        /// Strongly typed deserialisation outcome.
        DeserialisationStatus Status = DeserialisationStatus::Succeeded;

        // Input accounting.

        /// Bytes consumed by the complete validated root representation.
        std::size_t BytesConsumed = 0U;

        // Failure context.

        /// Compact diagnostic facts associated with the result.
        Diagnostic Detail{};

        // Outcome predicates.

        /// Reports whether validation and destination population completed successfully.
        ///
        /// @return true only when Status is DeserialisationStatus::Succeeded.
        [[nodiscard]] constexpr bool IsSuccessful() const noexcept {
            return Status == DeserialisationStatus::Succeeded;
        }

    };

} // ESPressio::Serialisation
