// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <utility>
#include <algorithm>

namespace sgl {

/**
 * \brief A vector that stores a small number of elements on the stack.
 *
 * \tparam T Element type
 * \tparam N Size of the short vector
 */
template<typename T, std::size_t N>
class short_vector {
public:
    static_assert(N > 0, "short_vector must have a size greater than zero.");

    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = value_type*;
    using const_iterator = const value_type*;

    /// Default constructor.
    short_vector() noexcept
        : m_data(m_short_data)
        , m_size(0)
        , m_capacity(N)
    {
    }

    /// Size constructor.
    short_vector(size_type size, const value_type& value)
        : m_data(m_short_data)
        , m_size(size)
        , m_capacity(N)
    {
        if (size > m_capacity)
            grow(size);
        for (size_type i = 0; i < size; ++i)
            m_data[i] = value;
    }

    /// Initializer list constructor.
    short_vector(std::initializer_list<value_type> list)
        : m_data(m_short_data)
        , m_size(0)
        , m_capacity(N)
    {
        for (const auto& value : list)
            push_back(value);
    }

    ~short_vector()
    {
        if (m_data != m_short_data)
            delete[] m_data;
    }

    short_vector(const short_vector& other) = delete;
    short_vector(short_vector&& other) = delete;

    short_vector& operator=(const short_vector& other) = delete;
    short_vector& operator=(short_vector&& other) = delete;

    reference operator[](size_type index) noexcept { return m_data[index]; }
    const_reference operator[](size_type index) const noexcept { return m_data[index]; }

    reference front() noexcept { return m_data[0]; }
    const_reference front() const noexcept { return m_data[0]; }

    reference back() noexcept { return m_data[m_size - 1]; }
    const_reference back() const noexcept { return m_data[m_size - 1]; }

    iterator begin() noexcept { return m_data; }
    const_iterator begin() const noexcept { return m_data; }

    iterator end() noexcept { return m_data + m_size; }
    const_iterator end() const noexcept { return m_data + m_size; }

    bool empty() const noexcept { return m_size == 0; }

    value_type* data() noexcept { return m_data; }
    const value_type* data() const noexcept { return m_data; }

    size_type size() const noexcept { return m_size; }
    size_type capacity() const noexcept { return m_capacity; }

    void clear() noexcept { m_size = 0; }

    void reserve(size_type new_capacity) { grow(new_capacity); }

    void push_back(const value_type& value)
    {
        if (m_size == m_capacity)
            grow(m_capacity * 2);
        m_data[m_size++] = value;
    }

    void push_back(value_type&& value)
    {
        if (m_size == m_capacity)
            grow(m_capacity * 2);
        m_data[m_size++] = std::move(value);
    }

    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        if (m_size == m_capacity)
            grow(m_capacity * 2);
        m_data[m_size++] = value_type(std::forward<Args>(args)...);
    }

    void pop_back() { --m_size; }

private:
    void grow(size_type new_capacity)
    {
        if (new_capacity <= m_capacity)
            return;
        m_capacity = new_capacity;
        value_type* new_data = new value_type[m_capacity];
        std::move(m_data, m_data + m_size, new_data);
        if (m_data != m_short_data)
            delete[] m_data;
        m_data = new_data;
    }

    value_type m_short_data[N];
    value_type* m_data;
    size_type m_size;
    size_type m_capacity;
};

} // namespace sgl
