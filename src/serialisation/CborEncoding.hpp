#pragma once

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "JsonEncoding.hpp"

namespace ESPressio::Serialisation::Detail {

    /// Internal outcomes produced by deterministic CBOR encoding.
    enum class CborEncodingStatus : std::uint8_t {
        /// The complete value was validated and encoded successfully.
        Succeeded = 0U,
        /// Size accounting or output capacity cannot retain the complete representation.
        ResourceLimitExceeded = 1U,
        /// A floating value is NaN or infinite and therefore outside the V1 domain.
        NonFiniteNumber = 2U,
        /// A bounded String contains invalid V1 UTF-8 text.
        InvalidUtf8 = 3U,
        /// Canonical strong-Type adaptation failed.
        AdaptationFailed = 4U
    };

    /// Maps one shared sink result into the CBOR encoder outcome family.
    inline CborEncodingStatus MapCborSinkWriteResult(
        EncodingSinkWriteResult result,
        std::size_t offset,
        Diagnostic& diagnostic
    ) noexcept {
        if (result == EncodingSinkWriteResult::Succeeded) {
            return CborEncodingStatus::Succeeded;
        }
        diagnostic.ByteOffset = offset;
        return CborEncodingStatus::ResourceLimitExceeded;
    }

    /// Writes one CBOR byte through the shared codec-neutral sink.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @param sink Destination sink.
    /// @param value Byte to append.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink>
    CborEncodingStatus WriteCborByte(
        TSink& sink,
        std::uint8_t value,
        Diagnostic& diagnostic
    ) noexcept {
        const auto offset = sink.Size();
        return MapCborSinkWriteResult(
            sink.WriteByte(value),
            offset,
            diagnostic
        );
    }

    /// Writes one CBOR byte range through the shared codec-neutral sink.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @param sink Destination sink.
    /// @param source First source byte.
    /// @param length Source byte count.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink>
    CborEncodingStatus WriteCborBytes(
        TSink& sink,
        const std::uint8_t* source,
        std::size_t length,
        Diagnostic& diagnostic
    ) noexcept {
        const auto offset = sink.Size();
        return MapCborSinkWriteResult(
            sink.WriteBytes(
                reinterpret_cast<const char*>(source),
                length
            ),
            offset,
            diagnostic
        );
    }

    /// Emits one canonical CBOR major-type header using the shortest legal argument width.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @param sink Destination sink.
    /// @param majorType CBOR major type in the range 0..7.
    /// @param argument Unsigned additional-information argument.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink>
    CborEncodingStatus EncodeCborHead(
        TSink& sink,
        std::uint8_t majorType,
        std::uint64_t argument,
        Diagnostic& diagnostic
    ) noexcept {
        std::array<std::uint8_t, 9U> bytes{};
        std::size_t size = 1U;
        std::uint8_t additional = 0U;
        std::size_t argumentBytes = 0U;

        if (argument <= 23U) {
            additional = static_cast<std::uint8_t>(argument);
        } else if (argument <= 0xFFU) {
            additional = 24U;
            argumentBytes = 1U;
        } else if (argument <= 0xFFFFU) {
            additional = 25U;
            argumentBytes = 2U;
        } else if (argument <= 0xFFFFFFFFULL) {
            additional = 26U;
            argumentBytes = 4U;
        } else {
            additional = 27U;
            argumentBytes = 8U;
        }

        bytes[0U] = static_cast<std::uint8_t>(
            static_cast<std::uint8_t>(majorType << 5U) | additional
        );
        for (std::size_t index = 0U; index < argumentBytes; ++index) {
            const auto shift = static_cast<unsigned>(
                (argumentBytes - 1U - index) * 8U
            );
            bytes[1U + index] = static_cast<std::uint8_t>(
                argument >> shift
            );
        }
        size += argumentBytes;
        return WriteCborBytes(
            sink,
            bytes.data(),
            size,
            diagnostic
        );
    }

    /// Encodes one supported integer through CBOR major type 0 or 1.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @tparam TValue Supported fixed-width integer Type.
    /// @param sink Destination sink.
    /// @param value Source integer.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink, class TValue>
    CborEncodingStatus EncodeCborInteger(
        TSink& sink,
        TValue value,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(IsFixedWidthInteger<Value>);

        if constexpr (std::is_signed_v<Value>) {
            const auto signedValue = static_cast<std::int64_t>(value);
            if (signedValue < 0) {
                const auto argument = static_cast<std::uint64_t>(
                    -(signedValue + 1)
                );
                return EncodeCborHead(
                    sink,
                    1U,
                    argument,
                    diagnostic
                );
            }
            return EncodeCborHead(
                sink,
                0U,
                static_cast<std::uint64_t>(signedValue),
                diagnostic
            );
        } else {
            return EncodeCborHead(
                sink,
                0U,
                static_cast<std::uint64_t>(value),
                diagnostic
            );
        }
    }

    /// Encodes one finite floating value using the exact schema-prescribed IEEE width.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @tparam TValue `float` or `double`.
    /// @param sink Destination sink.
    /// @param value Source floating value.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink, class TValue>
    CborEncodingStatus EncodeCborFloating(
        TSink& sink,
        TValue value,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(IsSupportedFloatingPoint<Value>);
        if (!std::isfinite(value)) { return CborEncodingStatus::NonFiniteNumber; }

        if constexpr (std::is_same_v<Value, float>) {
            const auto bits = std::bit_cast<std::uint32_t>(value);
            std::array<std::uint8_t, 5U> bytes{
                0xFAU,
                static_cast<std::uint8_t>(bits >> 24U),
                static_cast<std::uint8_t>(bits >> 16U),
                static_cast<std::uint8_t>(bits >> 8U),
                static_cast<std::uint8_t>(bits)
            };
            return WriteCborBytes(
                sink,
                bytes.data(),
                bytes.size(),
                diagnostic
            );
        } else {
            const auto bits = std::bit_cast<std::uint64_t>(value);
            std::array<std::uint8_t, 9U> bytes{};
            bytes[0U] = 0xFBU;
            for (std::size_t index = 0U; index < 8U; ++index) {
                const auto shift = static_cast<unsigned>((7U - index) * 8U);
                bytes[1U + index] = static_cast<std::uint8_t>(bits >> shift);
            }
            return WriteCborBytes(
                sink,
                bytes.data(),
                bytes.size(),
                diagnostic
            );
        }
    }

    /// Encodes one bounded UTF-8 String as CBOR major type 3.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @param sink Destination sink.
    /// @param data First UTF-8 source byte.
    /// @param size UTF-8 source byte count.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink>
    CborEncodingStatus EncodeCborTextString(
        TSink& sink,
        const char* data,
        std::size_t size,
        Diagnostic& diagnostic
    ) noexcept {
        const auto validation = ValidateUtf8(
            data,
            size
        );
        if (!validation.IsSuccessful()) {
            diagnostic.ByteOffset = validation.ByteOffset;
            return CborEncodingStatus::InvalidUtf8;
        }
        auto status = EncodeCborHead(
            sink,
            3U,
            static_cast<std::uint64_t>(size),
            diagnostic
        );
        if (status != CborEncodingStatus::Succeeded) { return status; }
        return WriteCborBytes(
            sink,
            reinterpret_cast<const std::uint8_t*>(data),
            size,
            diagnostic
        );
    }

    /// Encodes arbitrary octets as CBOR major type 2.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @param sink Destination sink.
    /// @param data First source octet.
    /// @param size Source octet count.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink>
    CborEncodingStatus EncodeCborByteString(
        TSink& sink,
        const std::uint8_t* data,
        std::size_t size,
        Diagnostic& diagnostic
    ) noexcept {
        auto status = EncodeCborHead(
            sink,
            2U,
            static_cast<std::uint64_t>(size),
            diagnostic
        );
        if (status != CborEncodingStatus::Succeeded) { return status; }
        return WriteCborBytes(
            sink,
            data,
            size,
            diagnostic
        );
    }

    /// Forward declaration for recursive CBOR value encoding.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @tparam TValue Serialisable source Type.
    template<class TSink, class TValue>
    CborEncodingStatus EncodeCborValue(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept;

    /// Encodes one definite-length ordered sequence.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @tparam TAccessor Indexed accessor returning each source element.
    /// @param sink Destination sink.
    /// @param count Number of represented elements.
    /// @param accessor Indexed source accessor.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink, class TAccessor>
    CborEncodingStatus EncodeCborArray(
        TSink& sink,
        std::size_t count,
        TAccessor&& accessor,
        Diagnostic& diagnostic
    ) noexcept {
        auto status = EncodeCborHead(
            sink,
            4U,
            static_cast<std::uint64_t>(count),
            diagnostic
        );
        if (status != CborEncodingStatus::Succeeded) { return status; }
        for (std::size_t index = 0U; index < count; ++index) {
            status = EncodeCborValue(
                sink,
                accessor(index),
                diagnostic
            );
            if (status != CborEncodingStatus::Succeeded) { return status; }
        }
        return CborEncodingStatus::Succeeded;
    }

    /// Counts the schema Fields emitted after Optional omission.
    ///
    /// @tparam TValue Serialisable schema Type.
    /// @param value Source schema object.
    /// @return Number of represented map entries.
    template<class TValue>
    std::size_t CountCborSchemaFields(
        const TValue& value
    ) noexcept {
        std::size_t count = 0U;
        System::ForEachField<TValue>([&]<class TField>() constexpr {
            using FieldValue = std::remove_cv_t<System::FieldValueOf<TField>>;
            if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                if ((value.*TField::Member).has_value()) { ++count; }
            } else {
                ++count;
            }
        });
        return count;
    }

    /// Emits only declared Fields in ascending FieldIdentifier order.
    ///
    /// Iterative schema traversal avoids the former 256-deep recursive
    /// instantiation, which can exceed an embedded FreeRTOS worker stack.
    /// FieldSet::ForEach uses a bounded fold over actual declared Fields.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @tparam TValue Serialisable schema Type.
    /// @param sink Destination sink.
    /// @param value Source schema object.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Internal CBOR encoding outcome.
    template<class TSink, class TValue>
    CborEncodingStatus EncodeCborSchemaFields(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        std::uint16_t nextLowerBound = 0U;

        for (
            std::size_t position = 0U;
            position < System::FieldsOf<TValue>::Count;
            ++position
        ) {
            std::uint16_t selected = 256U;
            System::ForEachField<TValue>([&]<class TField>() constexpr {
                const auto identifier =
                    static_cast<std::uint16_t>(TField::Identifier.Value());
                if (
                    identifier >= nextLowerBound &&
                    identifier < selected
                ) {
                    selected = identifier;
                }
            });
            if (selected == 256U) {
                return CborEncodingStatus::ResourceLimitExceeded;
            }

            auto status = CborEncodingStatus::Succeeded;
            System::ForEachField<TValue>([&]<class TField>() constexpr {
                if (
                    status != CborEncodingStatus::Succeeded ||
                    TField::Identifier.Value() != selected
                ) {
                    return;
                }
                using FieldValue =
                    std::remove_cv_t<System::FieldValueOf<TField>>;
                const auto& fieldValue = value.*TField::Member;

                bool emitted = true;
                if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                    emitted = fieldValue.has_value();
                }
                if (!emitted) { return; }

                status = EncodeCborHead(
                    sink, 0U,
                    static_cast<std::uint64_t>(TField::Identifier.Value()),
                    diagnostic
                );
                if (status == CborEncodingStatus::Succeeded) {
                    if constexpr (OptionalValueTraits<FieldValue>::IsValue) {
                        status = EncodeCborValue(
                            sink, fieldValue.value(), diagnostic
                        );
                    } else {
                        status = EncodeCborValue(
                            sink, fieldValue, diagnostic
                        );
                    }
                }
                if (status != CborEncodingStatus::Succeeded) {
                    if (!diagnostic.Type.has_value()) {
                        diagnostic.Type = TValue::Identifier;
                    }
                    if (!diagnostic.Field.has_value()) {
                        diagnostic.Field = TField::Identifier;
                    }
                }
            });
            if (status != CborEncodingStatus::Succeeded) {
                return status;
            }
            nextLowerBound = static_cast<std::uint16_t>(selected + 1U);
        }
        return CborEncodingStatus::Succeeded;
    }

    /// Encodes one schema object as a definite-length numeric-key CBOR map.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @tparam TValue Serialisable schema Type.
    /// @param sink Destination sink.
    /// @param value Source schema object.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink, class TValue>
    CborEncodingStatus EncodeCborSchema(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        const auto count = CountCborSchemaFields(value);
        auto status = EncodeCborHead(
            sink,
            5U,
            static_cast<std::uint64_t>(count),
            diagnostic
        );
        if (status != CborEncodingStatus::Succeeded) { return status; }
        return EncodeCborSchemaFields(
            sink,
            value,
            diagnostic
        );
    }

    /// Encodes one value from the complete currently-qualified V1 universe as deterministic CBOR.
    ///
    /// @tparam TSink Codec-neutral sink Type.
    /// @tparam TValue Serialisable source Type.
    /// @param sink Destination sink.
    /// @param value Source value.
    /// @param diagnostic Diagnostic payload populated on failure.
    /// @return Complete internal CBOR encoding outcome.
    template<class TSink, class TValue>
    CborEncodingStatus EncodeCborValue(
        TSink& sink,
        const TValue& value,
        Diagnostic& diagnostic
    ) noexcept {
        using Value = std::remove_cv_t<TValue>;
        static_assert(
            IsSerialisableType<Value>,
            "CBOR encoding requires a SerialisableType source value"
        );

        if constexpr (std::is_same_v<Value, bool>) {
            return WriteCborByte(
                sink,
                value ? 0xF5U : 0xF4U,
                diagnostic
            );
        } else if constexpr (IsFixedWidthInteger<Value>) {
            return EncodeCborInteger(
                sink,
                value,
                diagnostic
            );
        } else if constexpr (IsSupportedFloatingPoint<Value>) {
            return EncodeCborFloating(
                sink,
                value,
                diagnostic
            );
        } else if constexpr (std::is_enum_v<Value>) {
            using Underlying = typename EnumSerialisationTraits<Value>::UnderlyingType;
            return EncodeCborInteger(
                sink,
                static_cast<Underlying>(value),
                diagnostic
            );
        } else if constexpr (OptionalValueTraits<Value>::IsValue) {
            if (!value.has_value()) {
                return WriteCborByte(
                    sink,
                    0xF6U,
                    diagnostic
                );
            }
            return EncodeCborValue(
                sink,
                value.value(),
                diagnostic
            );
        } else if constexpr (StandardArrayTraits<Value>::IsValue) {
            return EncodeCborArray(
                sink,
                StandardArrayTraits<Value>::Count,
                [&](std::size_t index) -> const auto& { return value[index]; },
                diagnostic
            );
        } else if constexpr (std::is_array_v<Value>) {
            return EncodeCborArray(
                sink,
                std::extent_v<Value>,
                [&](std::size_t index) -> const auto& { return value[index]; },
                diagnostic
            );
        } else if constexpr (BoundedStringTraits<Value>::IsValue) {
            return EncodeCborTextString(
                sink,
                value.Data(),
                value.Size(),
                diagnostic
            );
        } else if constexpr (BoundedBytesTraits<Value>::IsValue) {
            return EncodeCborByteString(
                sink,
                value.Data(),
                value.Size(),
                diagnostic
            );
        } else if constexpr (BoundedVectorTraits<Value>::IsValue) {
            return EncodeCborArray(
                sink,
                value.Size(),
                [&](std::size_t index) -> const auto& { return value[index]; },
                diagnostic
            );
        } else if constexpr (PotentialSchemaType<Value>) {
            static_assert(System::SchemaType<Value>);
            return EncodeCborSchema(
                sink,
                value,
                diagnostic
            );
        } else if constexpr (HasCanonicalRepresentation<Value>) {
            using Representation = std::remove_cv_t<typename CanonicalRepresentation<Value>::Type>;
            Representation representation{};
            using Adapter = Bounded::TypeConversionAdapter<Value, Representation>;
            const auto conversionResult = Adapter::Convert(
                value,
                representation
            );
            if (!Bounded::IsTypeConversionSuccessful<Value, Representation>(conversionResult)) {
                return CborEncodingStatus::AdaptationFailed;
            }
            return EncodeCborValue(
                sink,
                representation,
                diagnostic
            );
        } else {
            static_assert(IsSerialisableType<Value>);
            return CborEncodingStatus::ResourceLimitExceeded;
        }
    }

} // ESPressio::Serialisation::Detail
