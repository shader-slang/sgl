#include "nanobind.h"

#include "kali/utils/tev.h"
#include "kali/core/bitmap.h"
#include "kali/device/resource.h"

#include "kali/core/thread.h" // TODO Remove

namespace kali {
class TestObject : public Object {
public:
    std::string value;
};
} // namespace kali

KALI_PY_EXPORT(utils_tev)
{
    using namespace kali;

    nb::module_ utils = m.attr("utils");

    utils.def(
        "show_in_tev",
        nb::overload_cast<const Bitmap*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev
        ),
        "bitmap"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );
    utils.def(
        "show_in_tev",
        nb::overload_cast<const Texture*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev
        ),
        "texture"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );
    utils.def(
        "show_in_tev_async",
        nb::overload_cast<const Bitmap*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev_async
        ),
        "bitmap"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );
    utils.def(
        "show_in_tev_async",
        nb::overload_cast<const Texture*, std::optional<std::string>, const std::string&, uint16_t, uint32_t>(
            &utils::show_in_tev_async
        ),
        "texture"_a,
        "name"_a = std::optional<std::string>{},
        "host"_a = "127.0.0.1",
        "port"_a = 14158,
        "max_retries"_a = 3
    );

    nb::class_<TestObject, Object>(utils, "TestObject").def(nb::init<>()).def_rw("value", &TestObject::value);

    utils.def(
        "test_async",
        [](ref<TestObject> object)
        {
            static uint64_t counter;
            uint64_t id = counter++;
            log_info("schedule task {} ({})", id, object->value);

#if 0
            TestObject* object_ptr = object;
            object_ptr->inc_ref();
            thread::do_async(
                [id, object_ptr]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds((id * 33) % 500));
                    log_info("task done {} ({})", id, object_ptr->value);
                    object_ptr->dec_ref();
                }
            );
#else
            thread::do_async(
                [id, object]()
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds((id * 33) % 500));
                    log_info("task done {} ({})", id, object->value);
                }
            );
#endif
        },
        nb::call_guard<nb::gil_scoped_release>()
    );
}
