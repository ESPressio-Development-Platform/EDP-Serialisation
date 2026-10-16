#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "Operations.hpp"

namespace ESPressio::Serialisation {

    namespace Detail {

        /// Encodes the ordered heterogeneous argument pack as one definite CBOR array.
        ///
        /// The canonical CBOR value encoder, rather than a Logging-specific duplicate,
        /// owns every scalar, String, byte or strong-Type representation.
        template<class TSink, SerialisableType... TValues>
        CborEncodingStatus EncodePositionalCborValues(
            TSink& sink,
            Diagnostic& diagnostic,
            const TValues&... values
        ) noexcept {
            auto status = EncodeCborHead(
                sink,
                4U,
                static_cast<std::uint64_t>(sizeof...(TValues)),
                diagnostic
            );
            if (status != CborEncodingStatus::Succeeded) return status;

            // Left-to-right short-circuit: never encode more after one failure.
            const bool succeeded = ((
                status == CborEncodingStatus::Succeeded &&
                ((status = EncodeCborValue(sink, values, diagnostic)) ==
                 CborEncodingStatus::Succeeded)
            ) && ...);
            static_cast<void>(succeeded);
            return status;
        }

    } // ESPressio::Serialisation::Detail


    /// Measures one heterogeneous, compiler-typed positional CBOR array exactly.
    ///
    /// Compile-time parameter types must satisfy existing SerialisableType.
    /// No runtime variant, vtable, heap or duplicate CBOR scalar writer is added.
    ///
    /// @tparam TMaximumArguments Application-enforced upper bound on arity.
    /// @tparam TValues Heterogeneous serialisable argument Types in wire order.
    template<std::size_t TMaximumArguments, SerialisableType... TValues>
    [[nodiscard]] MeasurementResult MeasurePositionalCbor(
        const TValues&... values
    ) noexcept {
        static_assert(
            sizeof...(TValues) <= TMaximumArguments,
            "Positional argument count exceeds application schema limit"
        );

        Detail::EncodingCountingSink sink;
        Diagnostic diagnostic;
        const auto status = Detail::EncodePositionalCborValues(
            sink, diagnostic, values...
        );
        const auto publicStatus = Detail::ToMeasurementStatus(status);
        return {
            publicStatus,
            publicStatus == MeasurementStatus::Succeeded ? sink.Size() : 0U,
            diagnostic
        };
    }


    /// Transactionally preflights and emits a heterogeneous canonical CBOR array
    /// to caller-owned storage. On source/size preflight failure, the destination
    /// bytes are UNTOUCHED. No new underlying platform provider is selected:
    /// default ByteOperations reuses the established Serialisation owner choice.
    ///
    /// Source values must remain stable between measurement and writing,
    /// as required by existing EDP-Serialisation::Serialise contracts.
    ///
    /// @tparam TMaximumArguments Compile-time application argument capacity.
    /// @tparam TByteOperationsProvider Stateless app-approved byte-copy provider.
    /// @tparam TValues Heterogeneous serialisable argument Types.
    template<
        std::size_t TMaximumArguments,
        class TByteOperationsProvider = ESPressio::Platform::Portable::Memory::ByteOperationsProvider,
        SerialisableType... TValues
    >
    [[nodiscard]] SerialisationResult SerialisePositionalCbor(
        std::span<std::byte> destination,
        const TValues&... values
    ) noexcept {
        const auto measured = MeasurePositionalCbor<TMaximumArguments>(values...);
        if (!measured.IsSuccessful()) {
            return {
                Detail::ToSerialisationStatus(measured.Status),
                0U,
                0U,
                measured.Detail
            };
        }

        if (destination.size() < measured.RequiredBytes) {
            return {
                SerialisationStatus::OutputBufferTooSmall,
                measured.RequiredBytes,
                0U,
                {}
            };
        }

        Detail::EncodingBufferSink<TByteOperationsProvider> sink{
            reinterpret_cast<std::uint8_t*>(destination.data()),
            destination.size()
        };
        Diagnostic diagnostic;
        const auto status = Detail::EncodePositionalCborValues(
            sink, diagnostic, values...
        );
        const auto publicStatus = Detail::ToSerialisationStatus(status);

        if (publicStatus != SerialisationStatus::Succeeded) {
            return {
                publicStatus,
                measured.RequiredBytes,
                0U,
                diagnostic
            };
        }
        return {
            SerialisationStatus::Succeeded,
            measured.RequiredBytes,
            sink.Size(),
            {}
        };
    }


    namespace Detail {

        /// Uses the EXISTING canonical numeric-profile JSON encoder for
        /// each compiler-typed heterogeneous value, in argument order.
        template<class TSink, SerialisableType... TValues>
        JsonEncodingStatus EncodePositionalJsonValues(
            TSink& sink,
            Diagnostic& diagnostic,
            const TValues&... values
        ) noexcept {
            auto status = WriteJsonByte(sink, static_cast<std::uint8_t>('['), diagnostic);
            if (status != JsonEncodingStatus::Succeeded) return status;

            std::size_t emitted = 0U;
            [[maybe_unused]] const auto append = [&](const auto& value) noexcept {
                if (emitted != 0U) {
                    status = WriteJsonByte(
                        sink, static_cast<std::uint8_t>(','), diagnostic
                    );
                    if (status != JsonEncodingStatus::Succeeded) return false;
                }
                ++emitted;
                status = EncodeJsonValue(sink, value, diagnostic);
                return status == JsonEncodingStatus::Succeeded;
            };

            const bool succeeded = (append(values) && ...);
            if (!succeeded) return status;
            return WriteJsonByte(sink, static_cast<std::uint8_t>(']'), diagnostic);
        }

    } // ESPressio::Serialisation::Detail


    /// Exact measurement of one heterogeneous canonical JSON positional array.
    template<std::size_t TMaximumArguments, SerialisableType... TValues>
    [[nodiscard]] MeasurementResult MeasurePositionalJson(
        const TValues&... values
    ) noexcept {
        static_assert(
            sizeof...(TValues) <= TMaximumArguments,
            "Positional argument count exceeds application schema limit"
        );
        Detail::EncodingCountingSink sink;
        Diagnostic diagnostic;
        const auto status = Detail::EncodePositionalJsonValues(
            sink, diagnostic, values...
        );
        const auto publicStatus = Detail::ToMeasurementStatus(status);
        return {
            publicStatus,
            publicStatus == MeasurementStatus::Succeeded ? sink.Size() : 0U,
            diagnostic
        };
    }


    /// Preflights and writes a canonical heterogeneous JSON positional array.
    /// Invalid source or insufficient capacity leaves output bytes untouched.
    template<
        std::size_t TMaximumArguments,
        class TByteOperationsProvider = ESPressio::Platform::Portable::Memory::ByteOperationsProvider,
        SerialisableType... TValues
    >
    [[nodiscard]] SerialisationResult SerialisePositionalJson(
        std::span<std::byte> destination,
        const TValues&... values
    ) noexcept {
        const auto measured = MeasurePositionalJson<TMaximumArguments>(values...);
        if (!measured.IsSuccessful()) {
            return {
                Detail::ToSerialisationStatus(measured.Status),
                0U,
                0U,
                measured.Detail
            };
        }
        if (destination.size() < measured.RequiredBytes) {
            return {
                SerialisationStatus::OutputBufferTooSmall,
                measured.RequiredBytes,
                0U,
                {}
            };
        }

        Detail::EncodingBufferSink<TByteOperationsProvider> sink{
            reinterpret_cast<std::uint8_t*>(destination.data()),
            destination.size()
        };
        Diagnostic diagnostic;
        const auto status = Detail::EncodePositionalJsonValues(
            sink, diagnostic, values...
        );
        const auto publicStatus = Detail::ToSerialisationStatus(status);
        if (publicStatus != SerialisationStatus::Succeeded) {
            return {
                publicStatus,
                measured.RequiredBytes,
                0U,
                diagnostic
            };
        }
        return {
            SerialisationStatus::Succeeded,
            measured.RequiredBytes,
            sink.Size(),
            {}
        };
    }

} // namespace ESPressio::Serialisation
