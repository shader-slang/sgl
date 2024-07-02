// SPDX-License-Identifier: Apache-2.0
#include "hotreload.h"

#include "sgl/core/file_system_watcher.h"
#include "sgl/device/shader.h"

namespace sgl {

HotReload::HotReload(ref<Device> device)
    : m_device(device)
{
    m_file_system_watcher = make_ref<FileSystemWatcher>();
    m_file_system_watcher->set_on_change([this](std::span<FileSystemWatchEvent> events)
                                         { on_file_system_event(events); });

#if SGL_WINDOWS
    m_file_system_watcher->add_watch({.directory = std::filesystem::current_path(), .recursive = true});
#else
    log_error("Hot reload is currently only supported on windows\n");
#endif
}

HotReload::~HotReload()
{

}

void HotReload::update()
{
    m_file_system_watcher->update();
}

void HotReload::on_file_system_event(std::span<FileSystemWatchEvent> events)
{
    int slang_count = 0;
    for (auto ev : events) {
        if (ev.path.extension() == ".slang")
            slang_count++;
    }
    if (slang_count == 0)
        return;

    reload_all_programs();

    // TODO(@ccummings): Once slang gives access to full module dependencies, reload specific modules
    /*
    std::set<std::filesystem::path> paths;
    for (auto ev : events)
        paths.insert(ev.path);

    for (auto session : m_all_slang_sessions)
        session->recreate_modules_referencing(paths);
    */
}


void HotReload::_register_slang_session(SlangSession* session)
{
    m_all_slang_sessions.insert(session);
}

void HotReload::_unregister_slang_session(SlangSession* session)
{
    m_all_slang_sessions.erase(session);
}

void HotReload::reload_all_programs()
{
    try {
        for (SlangSession* session : m_all_slang_sessions)
            session->recreate_session();
    } catch (SlangCompileError compile_error) {
        log_error(compile_error.what());
    }
}



}
