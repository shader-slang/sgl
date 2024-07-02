// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/fwd.h"

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/reflection.h"
#include "sgl/device/device_resource.h"

#include "sgl/core/object.h"
#include "sgl/core/enum.h"

#include <exception>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <slang.h>

namespace sgl {

class SGL_API HotReload : public Object {
    SGL_OBJECT(HotReload)
public:
    HotReload(ref<Device> device);
    ~HotReload();

    void reload_all_programs();

    void update();

    void _register_slang_session(SlangSession* session);
    void _unregister_slang_session(SlangSession* session);

private:

    void on_file_system_event(std::span<FileSystemWatchEvent> events);

    breakable_ref<Device> m_device;
    ref<FileSystemWatcher> m_file_system_watcher;
    std::set<SlangSession*> m_all_slang_sessions;
};

} // namespace sgl
