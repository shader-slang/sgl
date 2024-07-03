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

#if !SGL_WINDOWS
    log_error("Hot reload is currently only supported on windows\n");
#endif
}

void HotReload::update()
{
    // Update file system watcher, which in turn may cause on_file_system_event
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

void HotReload::_on_session_modules_changed(SlangSession* session)
{
    update_watched_paths_for_session(session);
}

uint32_t HotReload::auto_detect_delay() const
{
    return m_file_system_watcher->delay();
}
void HotReload::set_auto_detect_delay(uint32_t delay_ms)
{
    m_file_system_watcher->set_delay(delay_ms);
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

void HotReload::update_watched_paths_for_session(SlangSession* session)
{
    // Iterate over all the dependencies of all modules in the session.
    slang::ISession* slang_session = session->get_slang_session();
    SlangInt module_count = slang_session->getLoadedModuleCount();
    for (SlangInt module_index = 0; module_index < module_count; module_index++) {
        slang::IModule* slang_module = slang_session->getLoadedModule(module_index);
        SlangInt32 dependency_count = slang_module->getDependencyFileCount();
        for (SlangInt32 dependency_index = 0; dependency_index < dependency_count; dependency_index++) {
            {
                // Get the dependency as an FS path, verify it is absolute and turn into directory path.
                std::filesystem::path abs_path = slang_module->getDependencyFilePath(dependency_index);
                log_info(abs_path.string());
                if (!abs_path.is_absolute()) {
                    log_warn("None absolute path in hot reload: {}", abs_path);
                    continue;
                }
                abs_path = abs_path.parent_path().make_preferred();

                // If not already monitoring this path, add a watch for it
                if (!m_watched_paths.contains(abs_path)) {
                    m_file_system_watcher->add_watch({
                        .directory = abs_path,
                        .recursive = false,
                    });
                    m_watched_paths.insert(abs_path);
                }
            }
        }
    }
}

void HotReload::_clear_file_watches()
{
    for (auto& path : m_watched_paths) {
        m_file_system_watcher->remove_watch(path);
    }
    m_watched_paths.clear();
}

} // namespace sgl
