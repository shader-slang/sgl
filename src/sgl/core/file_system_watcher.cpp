// SPDX-License-Identifier: Apache-2.0

#include "sgl/core/file_system_watcher.h"

#include "sgl/core/error.h"

#include <fstream>
#include <iostream>

#if SGL_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace sgl {

struct FileSystemWatchState {
#if SGL_WINDOWS
    OVERLAPPED overlapped;
    HANDLE directory_handle;
    char buffer[32 * 1024];
    bool is_shutdown;
#endif
    FileSystemWatcher* watcher;
    FileSystemWatchDesc desc;
};


#if SGL_WINDOWS
// Windows completion routine called with buffer of filesytem events.
void CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    // Overlapped is pointer to FileSystemWatchState with windows OVERLAPPED structure at start.
    FileSystemWatchState* state = (FileSystemWatchState*)lpOverlapped;

    // Check we have data.
    if (dwErrorCode == ERROR_SUCCESS) {
        if (dwNumberOfBytesTransfered > 0) {
            FILE_NOTIFY_INFORMATION* notify_information;
            int offset = 0;

            // Iterate over events, and call '_notify_change' on the watcher for each one.
            do {
                notify_information = (FILE_NOTIFY_INFORMATION*)((char*)state->buffer + offset);
                std::wstring file_name(
                    notify_information->FileName,
                    notify_information->FileNameLength / sizeof(WCHAR)
                );

                std::filesystem::path path{file_name};
                FileSystemWatcherChange change = FileSystemWatcherChange::invalid;

                switch (notify_information->Action) {
                case FILE_ACTION_ADDED:
                    change = FileSystemWatcherChange::added;
                    break;
                case FILE_ACTION_REMOVED:
                    change = FileSystemWatcherChange::removed;
                    break;
                case FILE_ACTION_MODIFIED:
                    change = FileSystemWatcherChange::modified;
                    break;
                case FILE_ACTION_RENAMED_NEW_NAME:
                    change = FileSystemWatcherChange::renamed;
                    break;
                default:
                    break;
                }

                if (change != FileSystemWatcherChange::invalid)
                    state->watcher->_notify_change(state, path, change);

                offset += notify_information->NextEntryOffset;
            } while (notify_information->NextEntryOffset != 0);

            // Reissue the read request.
            if (!ReadDirectoryChangesW(
                    state->directory_handle,
                    state->buffer,
                    sizeof(state->buffer),
                    TRUE,
                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES
                        | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SECURITY,
                    NULL,
                    &state->overlapped,
                    &FileIOCompletionRoutine
                )) {
                log_error("ReadDirectoryChangesW failed. Error: {}\n", GetLastError());
            }
        }
    } else {

        state->is_shutdown = true;
    }
}
#endif

FileSystemWatcher::FileSystemWatcher()
{
#if !SGL_WINDOWS
    // TODO(@ccummings): File system watcher linux support
    SGL_THROW("File system watcher is only implemented on windows platforms")
#endif
}

FileSystemWatcher::~FileSystemWatcher()
{
    for (const auto& pair : m_watches) {
        stop_watch(pair.second);
    }
}

uint32_t FileSystemWatcher::add_watch(const FileSystemWatchDesc& desc)
{
    // Check watch doesn't already exist
    for (const auto& pair : m_watches) {
        if (pair.second->desc.directory == desc.directory) {
            SGL_THROW("A watch already exists for {}.", desc.directory);
        }
    }

    // Init a new watcher state object
    uint32_t id = m_next_id++;
    auto state = std::make_unique<FileSystemWatchState>();
    state->desc = desc;
    state->watcher = this;

#if SGL_WINDOWS
    // On windows, open a directory handle then start the directory changed monitoring process.
    auto wtxt = state->desc.directory.wstring();
    state->directory_handle = CreateFile(
        wtxt.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    if (state->directory_handle == INVALID_HANDLE_VALUE) {
        SGL_THROW("Failed to open directory for file watcher");
    }
    BOOL recursive = state->desc.recursive;
    std::memset(&state->overlapped, 0, sizeof(OVERLAPPED));
    state->overlapped.hEvent = this;
    state->is_shutdown = false;
    if (!ReadDirectoryChangesW(
            state->directory_handle,
            state->buffer,
            sizeof(state->buffer),
            recursive,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES
                | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SECURITY,
            NULL,
            &state->overlapped,
            &FileIOCompletionRoutine
        )) {
        SGL_THROW("ReadDirectoryChangesW failed. Error: {}\n", GetLastError());
    }
#endif

    m_watches[id] = std::move(state);
    return id;
}

void FileSystemWatcher::remove_watch(uint32_t id)
{
    auto& state = m_watches[id];
    stop_watch(state);
    m_watches.erase(id);
}

void FileSystemWatcher::remove_watch(const std::filesystem::path& directory)
{
    for (const auto& pair : m_watches) {
        if (pair.second->desc.directory == directory) {
            stop_watch(pair.second);
            m_watches.erase(pair.first);
            break;
        }
    }
}

void FileSystemWatcher::stop_watch(const std::unique_ptr<FileSystemWatchState>& state)
{
#if SGL_WINDOWS
    // On windows, CancelIO.
    // Note: the loop below could be a while(!state->is_shutdown), but making
    // it a fixed number of iterations lets us throw an exception if shutdown fails
    CancelIo(state->directory_handle);
    for (int it = 0; it < 100; it++) {
        if (state->is_shutdown)
            break;
        SleepEx(5, TRUE);
    }
    if (!state->is_shutdown)
        SGL_THROW("File system watch failed to shutdown after 500ms");
    CloseHandle(state->directory_handle);
#endif
}

void FileSystemWatcher::set_on_change(ChangeCallback on_change)
{
    m_on_change = on_change;
}

void FileSystemWatcher::_notify_change(
    FileSystemWatchState* state,
    const std::filesystem::path& path,
    FileSystemWatcherChange change
)
{
    std::filesystem::path abs_path(state->desc.directory.string() + "/" + path.string());
    abs_path = std::filesystem::absolute(abs_path);

    auto now = std::chrono::system_clock::now();
    FileSystemWatchEvent event = {.path = path, .absolute_path = abs_path, .change = change, .time = now};
    m_queued_events.push_back(event);
    m_last_event = now;
}

void FileSystemWatcher::update()
{
#if SGL_WINDOWS
    SleepEx(0, TRUE);
#endif
    if (m_queued_events.size() > 0) {
        auto duration = std::chrono::system_clock::now() - m_last_event;
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        if (millis > m_output_delay_ms) {
            std::span events(m_queued_events);
            m_on_change(events);
            m_queued_events.resize(0);
        }
    }
}


} // namespace sgl
