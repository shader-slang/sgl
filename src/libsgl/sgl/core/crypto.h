// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/macros.h"

#include <array>
#include <string>
#include <string_view>
#include <cstdint>
#include <cstdlib>

namespace sgl {

/**
 * Helper to compute SHA-1 hash.
 */
class SGL_API SHA1 {
public:
    /// Message digest.
    using Digest = std::array<uint8_t, 20>;

    SHA1();

    SHA1(const void* data, size_t len)
        : SHA1()
    {
        update(data, len);
    }

    SHA1(std::string_view str)
        : SHA1()
    {
        update(str);
    }

    SHA1(const SHA1&) = default;
    SHA1(SHA1&&) = delete;

    SHA1& operator=(const SHA1&) = default;
    SHA1& operator=(SHA1&&) = delete;

    /**
     * Update hash by adding one byte.
     * \param byte Byte to hash.
     */
    SHA1& update(uint8_t byte);

    /**
     * Update hash by adding the given data.
     * \param data Data to hash.
     * \param len Length of data in bytes.
     */
    SHA1& update(const void* data, size_t len);

    /**
     * Update hash by adding the given string.
     * \param str String to hash.
     */
    SHA1& update(std::string_view str) { return update(str.data(), str.size()); }

    /**
     * Update hash by adding the given basic value.
     * \param value to hash.
     */
    template<typename T>
    SHA1& update(const T& value)
        requires std::is_fundamental_v<T> || std::is_enum_v<T>
    {
        return update(&value, sizeof(value));
    }

    /// Return the message digest.
    Digest digest() const;

    /// Return the message digest as a hex string.
    std::string hex_digest() const;

private:
    void add_byte(uint8_t x);
    void process_block(const uint8_t* ptr);
    Digest finalize();

    uint32_t m_index;
    uint64_t m_bits;
    uint32_t m_state[5];
    uint8_t m_buf[64];
};
}; // namespace sgl
