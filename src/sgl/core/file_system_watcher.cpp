// SPDX-License-Identifier: Apache-2.0

#include "sgl/core/file_system_watcher.h"

#include "sgl/core/error.h"

#include <fstream>
#include <iostream>

#if SGL_WINDOWS
#include <windows.h>
#endif

namespace sgl {

struct FileSystemWatchState {
#if SGL_WINDOWS
    OVERLAPPED overlapped;
    HANDLE directory_handle;
    char buffer[32*1024];
#endif
    FileSystemWatcher* watcher;
    FileSystemWatchDesc desc;
};

#if SGL_WINDOWS
// Windows completion routine called with buffer of filesytem events
void CALLBACK FileIOCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
)
{
    // Check we have data
    if (dwErrorCode == ERROR_SUCCESS && dwNumberOfBytesTransfered > 0) {
        // Overlapped is pointer to FileSystemWatchState with windows OVERLAPPED structure at start
        FileSystemWatchState* state = (FileSystemWatchState*)lpOverlapped;

        FILE_NOTIFY_INFORMATION* pNotify;
        int offset = 0;

        // Iterate over events, and call '_notify_change' on the watcher for each one
        do {
            pNotify = (FILE_NOTIFY_INFORMATION*)((char*)state->buffer + offset);
            std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));

            std::filesystem::path path = fileName;
            FileSystemWatcherChange change = FileSystemWatcherChange::invalid;

            switch (pNotify->Action) {
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
            state->watcher->_notify_change(path, change);

            offset += pNotify->NextEntryOffset;
        } while (pNotify->NextEntryOffset != 0);

        // Reissue the read request
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
}
#endif

FileSystemWatcher::FileSystemWatcher() {

}

FileSystemWatcher::~FileSystemWatcher()
{

}

void FileSystemWatcher::add_watch(const FileSystemWatchDesc& desc)
{
    // Init a new watcher state object
    int id = m_next_id++;
    FileSystemWatchState* state = new FileSystemWatchState();
    state->desc = desc;
    state->watcher = this;
    m_watches.insert(std::pair(id, state));

#if SGL_WINDOWS
    // On wiondows, open a directory handle then start the directory changed monitoring process
    auto wtxt = state->desc.path.wstring();
    state->directory_handle = CreateFile(
        wtxt.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    memset(&state->overlapped, 0, sizeof(OVERLAPPED));
    state->overlapped.hEvent = this;
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
#else
    SGL_THROW("File system watcher is only implemented on windows platforms")
#endif
}

void FileSystemWatcher::remove_watch(const std::filesystem::path& path)
{
    for (auto pair : m_watches) {
        if (pair.second->desc.path == path) {
            FileSystemWatchState* state = pair.second;

#if SGL_WINDOWS
            // do stuff
            CloseHandle(state->directory_handle);
#endif

            delete state;
            m_watches.erase(pair.first);

        }
    }
}

void FileSystemWatcher::set_on_change(
    std::function<void(std::vector<FileSystemWatchEvent>&)> on_change
)
{
    m_on_change = on_change;
}

FileSystemWatchState* FileSystemWatcher::get_watch(int id)
{
    return m_watches[id];
}

void FileSystemWatcher::_notify_change(const std::filesystem::path& path, FileSystemWatcherChange change)
{
    auto now = std::chrono::system_clock::now();
    FileSystemWatchEvent event = {.path = path, .change = change, .time = now};
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
        if (millis > 1000) {
            m_on_change(m_queued_events);
            m_queued_events.resize(0);
        }
    }
}

} // namespace sgl
