#include "object.h"

#if KALI_ENABLE_OBJECT_TRACKING
#include "kali/core/error.h"
#include "kali/core/logger.h"
#include <set>
#include <mutex>
#endif

namespace kali {

static void (*object_inc_ref_py)(PyObject*) noexcept = nullptr;
static void (*object_dec_ref_py)(PyObject*) noexcept = nullptr;

#if KALI_ENABLE_OBJECT_TRACKING
static std::mutex s_tracked_objects_mutex;
static std::set<const Object*> s_tracked_objects;
#endif

void Object::inc_ref() const noexcept
{
    uintptr_t value = m_state.load(std::memory_order_relaxed);

    // TODO check this
#if KALI_ENABLE_OBJECT_TRACKING
    if (m_state == 1) {
        std::lock_guard<std::mutex> lock(s_tracked_objects_mutex);
        s_tracked_objects.insert(this);
    }
#endif

    while (true) {
        if (value & 1) {
            if (!m_state.compare_exchange_weak(value, value + 2, std::memory_order_relaxed, std::memory_order_relaxed))
                continue;
        } else {
            object_inc_ref_py((PyObject*)value);
        }

        break;
    }
}

void Object::dec_ref(bool dealloc) const noexcept
{
    uintptr_t value = m_state.load(std::memory_order_relaxed);

    while (true) {
        if (value & 1) {
            if (value == 1) {
                fprintf(stderr, "Object::dec_ref(%p): reference count underflow!", this);
                abort();
            } else if (value == 3) {
                if (dealloc) {
#if KALI_ENABLE_OBJECT_TRACKING
                    {
                        std::lock_guard<std::mutex> lock(s_tracked_objects_mutex);
                        s_tracked_objects.erase(this);
                    }
#endif
                    delete this;
                } else {
                    m_state.store(1, std::memory_order_relaxed);
                }
            } else {
                if (!m_state
                         .compare_exchange_weak(value, value - 2, std::memory_order_relaxed, std::memory_order_relaxed))
                    continue;
            }
        } else {
            object_dec_ref_py((PyObject*)value);
        }
        break;
    }
}

uint64_t Object::ref_count() const
{
    uintptr_t value = m_state.load(std::memory_order_relaxed);
    if (value & 1)
        return value >> 1;
    else
        return 0;
}

void Object::set_self_py(PyObject* o) noexcept
{
    uintptr_t value = m_state.load(std::memory_order_relaxed);
    if (value & 1) {
        value >>= 1;
        for (uintptr_t i = 0; i < value; ++i)
            object_inc_ref_py(o);

        m_state.store((uintptr_t)o);
    } else {
        fprintf(stderr, "Object::set_self_py(%p): a Python object was already present!", this);
        abort();
    }
}

std::string Object::to_string() const
{
    return fmt::format("{}({})", class_name(), fmt::ptr(this));
}

PyObject* Object::self_py() const noexcept
{
    uintptr_t value = m_state.load(std::memory_order_relaxed);
    if (value & 1)
        return nullptr;
    else
        return (PyObject*)value;
}

#if KALI_ENABLE_OBJECT_TRACKING

void Object::report_alive_objects()
{
    std::lock_guard<std::mutex> lock(s_tracked_objects_mutex);
    log_info("Alive objects:");
    for (const Object* object : s_tracked_objects)
        object->report_refs();
}

void Object::report_refs() const
{
    log_info("Object (class={} address={}) has {} reference(s)", class_name(), fmt::ptr(this), ref_count());
#if KALI_ENABLE_REF_TRACKING
    std::lock_guard<std::mutex> lock(m_ref_trackers_mutex);
    for (const auto& it : m_ref_trackers) {
        log_info("ref={} count={}\n{}\n", it.first, it.second.count, format_stacktrace(it.second.stack_trace));
    }
#endif
}

#endif // KALI_ENABLE_OBJECT_TRACKING

#if KALI_ENABLE_REF_TRACKING

void Object::inc_ref_tracked(uint64_t ref_id) const
{
    if (m_enable_ref_tracking) {
        std::lock_guard<std::mutex> lock(m_ref_trackers_mutex);
        auto it = m_ref_trackers.find(ref_id);
        if (it != m_ref_trackers.end()) {
            it->second.count++;
        } else {
            m_ref_trackers.emplace(ref_id, RefTracker{1, backtrace()});
        }
    }

    inc_ref();
}

void Object::dec_ref_tracked(uint64_t ref_id, bool dealloc) const noexcept
{
    if (m_enable_ref_tracking) {
        std::lock_guard<std::mutex> lock(m_ref_trackers_mutex);
        auto it = m_ref_trackers.find(ref_id);
        KALI_ASSERT(it != m_ref_trackers.end());
        if (--it->second.count == 0) {
            m_ref_trackers.erase(it);
        }
    }

    dec_ref(dealloc);
}

void Object::set_enable_ref_tracking(bool enable)
{
    m_enable_ref_tracking = enable;
}

#endif // KALI_ENABLE_REF_TRACKING

void object_init_py(void (*object_inc_ref_py_)(PyObject*) noexcept, void (*object_dec_ref_py_)(PyObject*) noexcept)
{
    object_inc_ref_py = object_inc_ref_py_;
    object_dec_ref_py = object_dec_ref_py_;
}

} // namespace kali
