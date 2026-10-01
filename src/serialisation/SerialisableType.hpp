#pragma once

#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

#include <ESPressio_BoundedTypes.hpp>
#include <ESPressio_System.hpp>

#include "CanonicalRepresentation.hpp"
#include "EnumSerialisationTraits.hpp"
#include "SystemIdentifierAdapters.hpp"

namespace ESPressio::Serialisation {

    namespace Detail {

        /// Removes top-level cv-qualification before V1 Type classification.
        ///
        /// @tparam TValue Value Type being normalized.
        template<class TValue>
        using NormalizedType = std::remove_cv_t<TValue>;

        /// Indicates whether one Type is a member of the V1 fixed-width integer universe.
        ///
        /// @tparam TValue Value Type being inspected.
        template<class TValue>
        inline constexpr bool IsFixedWidthInteger =
            std::is_same_v<TValue, std::int8_t> ||
            std::is_same_v<TValue, std::int16_t> ||
            std::is_same_v<TValue, std::int32_t> ||
            std::is_same_v<TValue, std::int64_t> ||
            std::is_same_v<TValue, std::uint8_t> ||
            std::is_same_v<TValue, std::uint16_t> ||
            std::is_same_v<TValue, std::uint32_t> ||
            std::is_same_v<TValue, std::uint64_t>;

        /// Indicates whether one Type is a V1 IEEE floating-point domain.
        ///
        /// @tparam TValue Value Type being inspected.
        template<class TValue>
        inline constexpr bool IsSupportedFloatingPoint =
            std::is_same_v<TValue, float> ||
            std::is_same_v<TValue, double>;

        /// Classifies non-Optional Types as false.
        ///
        /// @tparam TValue Value Type being inspected.
        template<class TValue>
        struct IsOptional final : std::false_type {
        };

        /// Classifies standard Optional values as true.
        ///
        /// @tparam TValue Contained Optional value Type.
        template<class TValue>
        struct IsOptional<std::optional<TValue>> final : std::true_type {
        };

        /// Indicates whether a strong Type declares one canonical Serialisation representation.
        ///
        /// @tparam TValue Strong semantic Type being inspected.
        template<class TValue>
        concept HasCanonicalRepresentation = requires {
            typename CanonicalRepresentation<TValue>::Type;
        };

        /// Indicates whether an enum Type declares explicit Serialisation certification.
        ///
        /// @tparam TValue Candidate enum Type being inspected.
        template<class TValue>
        concept HasEnumSerialisationTraits = requires {
            typename EnumSerialisationTraits<TValue>::UnderlyingType;
        };

        /// Safely identifies Types which can be passed to the current System SchemaType predicate.
        ///
        /// The pre-gate prevents unrelated or malformed Types from instantiating System's
        /// focused schema diagnostics while Serialisation is performing Boolean qualification.
        ///
        /// @tparam TValue Candidate schema Type being inspected.
        template<class TValue>
        concept PotentialSchemaType =
            requires {
                TValue::Identifier;
                typename TValue::Fields;
            } &&
            std::is_same_v<
                std::remove_cv_t<decltype(TValue::Identifier)>,
                System::TypeIdentifier
            > &&
            requires {
                typename std::bool_constant<TValue::Identifier.IsValid()>;
            };

        /// Recursive V1 serialisability classifier.
        ///
        /// @tparam TValue Value Type being inspected.
        template<class TValue>
        struct SerialisableTypeTrait;

        /// Validates every canonical System schema Field recursively.
        ///
        /// @tparam TValue System schema Type whose Field values are being inspected.
        /// @return true only when every Field value satisfies the complete V1 Type universe.
        template<class TValue>
        consteval bool AreSchemaFieldsSerialisable() {
            bool result = true;

            System::ForEachField<TValue>([&]<class TField>() constexpr {
                using FieldValue = NormalizedType<System::FieldValueOf<TField>>;

                if constexpr (!SerialisableTypeTrait<FieldValue>::Value) { result = false; }
            });

            return result;
        }

        /// Validates explicit enum certification against the C++ enum declaration.
        ///
        /// @tparam TValue Candidate enum Type being inspected.
        /// @return true only when the declared and actual underlying Types match a V1 integer Type.
        template<class TValue>
        consteval bool IsCertifiedEnum() {
            if constexpr (!std::is_enum_v<TValue>) { return false; }
            if constexpr (!HasEnumSerialisationTraits<TValue>) { return false; }

            using Declared = typename EnumSerialisationTraits<TValue>::UnderlyingType;
            using Actual = std::underlying_type_t<TValue>;

            return
                std::is_same_v<Declared, Actual> &&
                IsFixedWidthInteger<Declared>;
        }

        /// Validates one direct strong-Type adaptation without permitting a second adaptation hop.
        ///
        /// @tparam TValue Strong semantic Type being inspected.
        /// @return true only when the canonical surrogate is serialisable and both conversions
        ///         are available, success-aware, and noexcept.
        template<class TValue>
        consteval bool IsCanonicalAdaptation() {
            if constexpr (!HasCanonicalRepresentation<TValue>) {
                return false;
            } else {
                using Representation = typename CanonicalRepresentation<TValue>::Type;
                using NormalizedRepresentation = NormalizedType<Representation>;

                if constexpr (std::is_reference_v<Representation>) {
                    return false;
                } else if constexpr (std::is_same_v<TValue, NormalizedRepresentation>) {
                    return false;
                } else if constexpr (HasCanonicalRepresentation<NormalizedRepresentation>) {
                    // A canonical surrogate may contain adapted nested values but may not itself
                    // be another strong Type requiring a second canonical adaptation hop.
                    return false;
                } else if constexpr (!std::is_nothrow_default_constructible_v<NormalizedRepresentation>) {
                    // Forward encoding and reverse validation both create bounded canonical-surrogate
                    // storage which the existing TypeConversionAdapter populates by reference.
                    return false;
                } else if constexpr (
                    !std::is_nothrow_default_constructible_v<TValue> &&
                    !std::is_nothrow_copy_constructible_v<TValue>
                ) {
                    // Reverse validation needs temporary semantic storage. Existing destination state can
                    // seed that storage by nothrow copy when the semantic Type intentionally has no default.
                    return false;
                } else if constexpr (!SerialisableTypeTrait<NormalizedRepresentation>::Value) {
                    return false;
                } else if constexpr (!Bounded::IsTypeConversionAvailable<TValue, NormalizedRepresentation>) {
                    return false;
                } else if constexpr (!Bounded::IsTypeConversionAvailable<NormalizedRepresentation, TValue>) {
                    return false;
                } else if constexpr (!Bounded::HasTypeConversionSuccessPredicate<TValue, NormalizedRepresentation>) {
                    return false;
                } else if constexpr (!Bounded::HasTypeConversionSuccessPredicate<NormalizedRepresentation, TValue>) {
                    return false;
                } else {
                    using Forward = Bounded::TypeConversionAdapter<TValue, NormalizedRepresentation>;
                    using Reverse = Bounded::TypeConversionAdapter<NormalizedRepresentation, TValue>;

                    return Forward::IsNoexcept && Reverse::IsNoexcept;
                }
            }
        }

        /// Classifies one non-container Type against scalar, enum, schema, and adaptation rules.
        ///
        /// @tparam TValue Value Type being inspected.
        /// @return true only when TValue satisfies one complete V1 representation category.
        template<class TValue>
        consteval bool IsSerialisableScalarSchemaOrAdaptation() {
            if constexpr (std::is_same_v<TValue, bool>) {
                return true;
            } else if constexpr (IsFixedWidthInteger<TValue>) {
                return true;
            } else if constexpr (IsSupportedFloatingPoint<TValue>) {
                return true;
            } else if constexpr (std::is_enum_v<TValue>) {
                return IsCertifiedEnum<TValue>();
            } else if constexpr (PotentialSchemaType<TValue>) {
                if constexpr (System::SchemaType<TValue>) {
                    return AreSchemaFieldsSerialisable<TValue>();
                } else {
                    return false;
                }
            } else {
                return IsCanonicalAdaptation<TValue>();
            }
        }

        /// Default recursive classifier for scalar, schema, enum, or adapted semantic Types.
        ///
        /// @tparam TValue Value Type being inspected.
        template<class TValue>
        struct SerialisableTypeTrait final {

            // Qualification state.

            /// Complete V1 serialisability qualification for TValue.
            static constexpr bool Value = IsSerialisableScalarSchemaOrAdaptation<NormalizedType<TValue>>();

        };

        /// Recursively qualifies a fixed-size standard array.
        ///
        /// @tparam TValue Array element Type.
        /// @tparam TLength Exact array extent.
        template<class TValue, std::size_t TLength>
        struct SerialisableTypeTrait<std::array<TValue, TLength>> final {

            // Qualification state.

            /// Complete V1 serialisability qualification for every array element.
            static constexpr bool Value = SerialisableTypeTrait<NormalizedType<TValue>>::Value;

        };

        /// Recursively qualifies a native fixed-extent C array.
        ///
        /// @tparam TValue Array element Type.
        /// @tparam TLength Exact array extent.
        template<class TValue, std::size_t TLength>
        struct SerialisableTypeTrait<TValue[TLength]> final {

            // Qualification state.

            /// Complete V1 serialisability qualification for every array element.
            static constexpr bool Value = SerialisableTypeTrait<NormalizedType<TValue>>::Value;

        };

        /// Recursively qualifies one non-nested Optional value.
        ///
        /// @tparam TValue Optional element Type.
        template<class TValue>
        struct SerialisableTypeTrait<std::optional<TValue>> final {

            // Element metadata.

            /// Top-level cv-normalized Optional element Type.
            using Element = NormalizedType<TValue>;

            // Qualification state.

            /// Complete V1 qualification; directly nested Optional values are intentionally rejected.
            static constexpr bool Value =
                !IsOptional<Element>::value &&
                std::is_nothrow_default_constructible_v<Element> &&
                SerialisableTypeTrait<Element>::Value;

        };

        /// Qualifies bounded String values as the V1 bounded-text category.
        ///
        /// @tparam TCapacity Maximum String payload bytes.
        /// @tparam TProvider Stateless ByteOperations provider selected by the bounded String.
        template<std::size_t TCapacity, class TProvider>
        struct SerialisableTypeTrait<Bounded::String<TCapacity, TProvider>> final {

            // Qualification state.

            /// Bounded String is an explicitly admitted V1 Type family.
            static constexpr bool Value = true;

        };

        /// Qualifies bounded Bytes values as the V1 arbitrary-octet category.
        ///
        /// @tparam TCapacity Maximum binary payload bytes.
        /// @tparam TProvider Stateless ByteOperations provider selected by the bounded Bytes.
        template<std::size_t TCapacity, class TProvider>
        struct SerialisableTypeTrait<Bounded::Bytes<TCapacity, TProvider>> final {

            // Qualification state.

            /// Bounded Bytes is an explicitly admitted V1 Type family.
            static constexpr bool Value = true;

        };

        /// Recursively qualifies one bounded variable-length sequence.
        ///
        /// @tparam TValue Vector element Type.
        /// @tparam TCapacity Maximum logical element count.
        template<class TValue, std::size_t TCapacity>
        struct SerialisableTypeTrait<Bounded::Vector<TValue, TCapacity>> final {

            // Qualification state.

            /// Complete V1 qualification for elements which can be activated transactionally in inline slots.
            static constexpr bool Value =
                std::is_nothrow_default_constructible_v<NormalizedType<TValue>> &&
                SerialisableTypeTrait<NormalizedType<TValue>>::Value;

        };

    } // ESPressio::Serialisation::Detail

    static_assert(
        CHAR_BIT == 8,
        "EDP-Serialisation V1 requires eight-bit bytes"
    );
    static_assert(sizeof(std::int8_t) == 1U && sizeof(std::uint8_t) == 1U);
    static_assert(sizeof(std::int16_t) == 2U && sizeof(std::uint16_t) == 2U);
    static_assert(sizeof(std::int32_t) == 4U && sizeof(std::uint32_t) == 4U);
    static_assert(sizeof(std::int64_t) == 8U && sizeof(std::uint64_t) == 8U);
    static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4U);
    static_assert(std::numeric_limits<float>::radix == 2 && std::numeric_limits<float>::digits == 24);
    static_assert(std::numeric_limits<float>::min_exponent == -125 && std::numeric_limits<float>::max_exponent == 128);
    static_assert(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8U);
    static_assert(std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits == 53);
    static_assert(std::numeric_limits<double>::min_exponent == -1021 && std::numeric_limits<double>::max_exponent == 1024);

    /// Boolean compile-time qualification for the complete V1 serialisable value universe.
    ///
    /// @tparam TValue Value Type being classified.
    template<class TValue>
    inline constexpr bool IsSerialisableType = Detail::SerialisableTypeTrait<std::remove_cv_t<TValue>>::Value;

    /// Predicate identifying Types accepted by the complete V1 Serialisation value contract.
    ///
    /// @tparam TValue Value Type being classified.
    template<class TValue>
    concept SerialisableType = IsSerialisableType<TValue>;

} // ESPressio::Serialisation
