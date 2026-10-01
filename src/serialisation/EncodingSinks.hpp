#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <ESPressio_Memory.hpp>
#include <ESPressio_Platform_Portable_ByteOperations.hpp>

namespace ESPressio::Serialisation::Detail {

    /// Internal outcomes produced while appending bytes to a codec-neutral encoding sink.
    enum class EncodingSinkWriteResult : std::uint8_t {
        /// The requested bytes were accounted for or retained successfully.
        Succeeded = 0U,
        /// The sink cannot represent or retain the requested additional bytes.
        CapacityExceeded = 1U
    };

    /// Zero-storage sink which computes the exact encoded byte count for any codec.
    class EncodingCountingSink final {
    private:

        // Measurement state.

        /// Exact number of encoded bytes accounted for so far.
        std::size_t _size = 0U;

    public:

        // Measurement operations.

        /// Accounts for one encoded byte while protecting size_t from overflow.
        EncodingSinkWriteResult WriteByte(
            std::uint8_t value
        ) noexcept {
            static_cast<void>(value);
            if (_size == std::numeric_limits<std::size_t>::max()) {
                return EncodingSinkWriteResult::CapacityExceeded;
            }
            ++_size;
            return EncodingSinkWriteResult::Succeeded;
        }

        /// Accounts for one encoded byte range while protecting size_t from overflow.
        EncodingSinkWriteResult WriteBytes(
            const char* source,
            std::size_t length
        ) noexcept {
            static_cast<void>(source);
            if (length > std::numeric_limits<std::size_t>::max() - _size) {
                return EncodingSinkWriteResult::CapacityExceeded;
            }
            _size += length;
            return EncodingSinkWriteResult::Succeeded;
        }

        /// Returns the exact encoded bytes accounted for so far.
        [[nodiscard]] constexpr std::size_t Size() const noexcept {
            return _size;
        }

    };

    /// Caller-buffer sink shared by encoding codecs after successful exact pre-measurement.
    ///
    /// @tparam TByteOperationsProvider Stateless EDP-Memory ByteOperations provider selected at compile time.
    template<
        class TByteOperationsProvider = ESPressio::Platform::Portable::Memory::ByteOperationsProvider
    >
    class EncodingBufferSink final {
    private:

        // Provider contract.

        static_assert(
            sizeof(ESPressio::Memory::Detail::ByteOperationsProviderTraits<TByteOperationsProvider>) > 0U,
            "Encoding output requires a provider satisfying the EDP-Memory ByteOperations contract"
        );
        static_assert(
            std::is_empty_v<TByteOperationsProvider>,
            "Encoding output requires a stateless ByteOperations provider because no provider state is retained"
        );
        static_assert(
            std::is_nothrow_default_constructible_v<TByteOperationsProvider>,
            "Encoding output requires a nothrow default-constructible ByteOperations provider"
        );

        // Caller-owned output state.

        /// First byte of caller-owned output storage.
        std::uint8_t* _output = nullptr;

        /// Total writable capacity of caller-owned output storage.
        std::size_t _capacity = 0U;

        /// Number of bytes committed to caller-owned storage so far.
        std::size_t _size = 0U;

    public:

        // Construction.

        /// Binds the sink to caller-owned storage for one complete serialisation pass.
        EncodingBufferSink(
            std::uint8_t* output,
            std::size_t capacity
        ) noexcept :
            _output(output),
            _capacity(capacity) {
        }

        // Output operations.

        /// Writes one encoded byte when caller-owned capacity remains available.
        EncodingSinkWriteResult WriteByte(
            std::uint8_t value
        ) noexcept {
            if (_size >= _capacity) { return EncodingSinkWriteResult::CapacityExceeded; }
            _output[_size] = value;
            ++_size;
            return EncodingSinkWriteResult::Succeeded;
        }

        /// Writes a complete encoded byte range through the selected EDP-Memory provider.
        EncodingSinkWriteResult WriteBytes(
            const char* source,
            std::size_t length
        ) noexcept {
            if (_size > _capacity || length > _capacity - _size) {
                return EncodingSinkWriteResult::CapacityExceeded;
            }
            TByteOperationsProvider{}.CopyBytes(
                _output + _size,
                source,
                length
            );
            _size += length;
            return EncodingSinkWriteResult::Succeeded;
        }

        /// Returns the number of bytes committed to caller-owned storage so far.
        [[nodiscard]] constexpr std::size_t Size() const noexcept {
            return _size;
        }

    };

} // ESPressio::Serialisation::Detail
