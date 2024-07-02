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

/// Shader hot reload management, detects when relevant slang files
/// have been editor and triggers session recreates as necessary.
class SGL_API HotReload : public Object {
    SGL_OBJECT(HotReload)
public:
    HotReload(ref<Device> device);

    /// Force immediate recreation of all registered sessions and
    /// any module/programs they've loaded/linked.
    void recreate_all_sessions();

    /// Updates internal file system monitor for change detection.
    void update();

    // Internal functions called from session constructor/destructor
    // to register sessions with hot reload system.
    void _register_slang_session(SlangSession* session);
    void _unregister_slang_session(SlangSession* session);

    // Enable/disable auto rebuild in response to file system events.
    // Mainly just for testing system to have control over when
    // certain changes are applied.
    bool auto_detect_changes() const { return m_auto_detect_changes; }
    void set_auto_detect_changes(bool val) { m_auto_detect_changes = val; }

private:

    void on_file_system_event(std::span<FileSystemWatchEvent> events);

    breakable_ref<Device> m_device;
    bool m_auto_detect_changes;
    ref<FileSystemWatcher> m_file_system_watcher;
    std::set<SlangSession*> m_all_slang_sessions;
};

} // namespace sgl
