#pragma once

#include "platform.h"

#include <atomic>
#include <type_traits>
#include <cstdint>

namespace kali {

/**
 * Base class for reference counted object.
 */
class KALI_API Object {
public:
    /// Default constructor.
    Object() = default;

    /// Default destructor.
    virtual ~Object() { }

    /// Return current reference count.
    uint32_t ref_count() const { return m_ref_count; }

    /// Increment reference count.
    void inc_ref() const { ++m_ref_count; }

    /// Decrement reference count and deallocate if count goes to zero.
    void dec_ref(bool dealloc = true) const
    {
        --m_ref_count;
        if (dealloc && m_ref_count == 0) {
            delete this;
        }
    }

private:
    mutable std::atomic<uint32_t> m_ref_count{0};
};

template<typename T>
class ref {
public:
    // TODO move elsewhere so we can use ref with forward declared types
    // static_assert(std::is_base_of_v<Object, T>, "Cannot create reference to objects not inheriting from Object
    // class.");

    /// Default constructor.
    ref() = default;

    /// Construct a reference from a pointer.
    ref(T* ptr)
        : m_ptr(ptr)
    {
        if (m_ptr)
            static_cast<Object*>(m_ptr)->inc_ref();
    }

    /// Construct a reference from a convertible reference.
    template<typename T2>
    ref(const ref<T2>& r)
        : m_ptr(static_cast<T2*>(r.get()))
    {
        static_assert(
            std::is_convertible_v<T2*, T*>, "Cannot create reference to object from another unconvertible reference.");
        if (m_ptr)
            static_cast<Object*>(m_ptr)->inc_ref();
    }

    /// Copy constructor.
    ref(const ref& r)
        : m_ptr(r.m_ptr)
    {
        if (m_ptr)
            static_cast<Object*>(m_ptr)->inc_ref();
    }

    /// Move constructor.
    ref(ref&& r) noexcept
        : m_ptr(r.m_ptr)
    {
        r.m_ptr = nullptr;
    }

    /// Destructor.
    ~ref()
    {
        if (m_ptr)
            static_cast<Object*>(m_ptr)->dec_ref();
    }

    /// Copy assignment operator.
    ref& operator=(const ref& r) noexcept
    {
        if (m_ptr != r.m_ptr) {
            if (r.m_ptr)
                static_cast<Object*>(r.m_ptr)->inc_ref();
            if (m_ptr)
                static_cast<Object*>(m_ptr)->dec_ref();
            m_ptr = r.m_ptr;
        }
        return *this;
    }

    /// Move assignment operator.
    ref& operator=(ref&& r) noexcept
    {
        if (&r != this) {
            if (m_ptr)
                static_cast<Object*>(m_ptr)->dec_ref();
            m_ptr = r.m_ptr;
            r.m_ptr = nullptr;
        }
        return *this;
    }

    /// Assign pointer.
    ref& operator=(T* ptr) noexcept
    {
        if (m_ptr != ptr) {
            if (ptr)
                static_cast<Object*>(ptr)->inc_ref();
            if (m_ptr)
                static_cast<Object*>(m_ptr)->dec_ref();
            m_ptr = ptr;
        }
        return *this;
    }

    /// Compare this reference to another reference.
    bool operator==(const ref& r) const { return m_ptr == r.m_ptr; }

    /// Compare this reference to another reference.
    bool operator!=(const ref& r) const { return m_ptr != r.m_ptr; }

    /// Compare this reference to a pointer.
    bool operator==(const T* ptr) const { return m_ptr == ptr; }

    /// Compare this reference to a pointer.
    bool operator!=(const T* ptr) const { return m_ptr != ptr; }

    /// Access the object referenced by this reference.
    T* operator->() { return m_ptr; }

    /// Access the object referenced by this reference.
    const T* operator->() const { return m_ptr; }

    /// Return a C++ reference to the referenced object.
    T& operator*() { return *m_ptr; }

    /// Return a const C++ reference to the referenced object.
    const T& operator*() const { return *m_ptr; }

    /// Return a pointer to the referenced object.
    operator T*() { return m_ptr; }

    /// Return a pointer to the referenced object.
    operator const T*() const { return m_ptr; }

    /// Return a const pointer to the referenced object.
    T* get() { return m_ptr; }

    /// Return a pointer to the referenced object.
    const T* get() const { return m_ptr; }

    /// Check if the object is defined.
    operator bool() const { return m_ptr != nullptr; }

private:
    T* m_ptr{nullptr};
};

} // namespace kali
