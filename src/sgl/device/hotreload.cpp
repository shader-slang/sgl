// SPDX-License-Identifier: Apache-2.0
#include "hotreload.h"

#include "sgl/core/file_system_watcher.h"
#include "sgl/device/shader.h"

namespace sgl {

HotReload::HotReload(ref<Device> device)
    : m_device(device)
    , m_auto_detect_changes(true)
    , m_last_build_failed(false)
{
    // Create file system monitor + hook up change event.
    m_file_system_watcher = make_ref<FileSystemWatcher>();
    m_file_system_watcher->set_on_change([this](std::span<FileSystemWatchEvent> events)
                                         { on_file_system_event(events); });

    // Start a recursive monitor on working directory.
#if SGL_WINDOWS
    m_file_system_watcher->add_watch({.directory = std::filesystem::current_path(), .recursive = true});
#else
    log_error("Hot reload is currently only supported on windows\n");
#endif
}

void HotReload::update()
{
    // Update file system watcher, which in turn my cause on_file_system_event
    // to be called.
    m_file_system_watcher->update();
}

void HotReload::on_file_system_event(std::span<FileSystemWatchEvent> events)
{
    // Simple check to see if any events involved .slang files.
    int slang_count = 0;
    for (auto ev : events) {
        if (ev.path.extension() == ".slang")
            slang_count++;
    }
    if (slang_count == 0)
        return;

    // If slang files detected, recreate all existing sessions
    if (m_auto_detect_changes)
        recreate_all_sessions();
}


void HotReload::_register_slang_session(SlangSession* session)
{
    m_all_slang_sessions.insert(session);
}

void HotReload::_unregister_slang_session(SlangSession* session)
{
    m_all_slang_sessions.erase(session);
}

void HotReload::recreate_all_sessions()
{
    // Iterate over sessions and build each one. This is in a try/catch
    // statement as we don't want python programs to except as a result
    // of hot-reload compile errors. Instead, the error should be
    // logged and application carry on as usual.
    try {
        m_last_build_failed = false;
        for (SlangSession* session : m_all_slang_sessions)
            session->recreate_session();
    } catch (SlangCompileError compile_error) {
        log_error("Hot reload failed due to compile error");
        log_error(compile_error.what());
        m_last_build_failed = true;
    } catch (std::runtime_error runtime_error) {
        log_error("Hot reload failed due to incompatible shader modification");
        log_error(runtime_error.what());
        m_last_build_failed = true;
    }
}



}
