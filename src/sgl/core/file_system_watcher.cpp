// SPDX-License-Identifier: Apache-2.0

#include "sgl/core/file_system_watcher.h"

#include "sgl/core/error.h"

#include <map>

#if SGL_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif SGL_LINUX
#include <sys/inotify.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace sgl {

struct FileSystemWatchState {
#if SGL_WINDOWS
    // NOTE: Windows relies on the OVERLAPPED structure being the first structure in the state.
    OVERLAPPED overlapped;
    HANDLE directory_handle;
    char buffer[32 * 1024];
    bool is_shutdown;
#elif SGL_LINUX
    int watch_descriptor;
#endif
#if !SGL_WINDOWS && !SGL_LINUX
    std::map<std::filesystem::path, std::filesystem::file_time_type> files;
#endif
    FileSystemWatcher* watcher;
    FileSystemWatchDesc desc;
};

#if SGL_WINDOWS
static_assert(offsetof(FileSystemWatchState, overlapped) == 0);
#endif

#if SGL_WINDOWS
// Windows completion routine called with buffer of filesytem events.
static void CALLBACK
FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
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

                // This is an error that shouldn't happen on windows, but handling it just in case.
                if ((offset + sizeof(FILE_NOTIFY_INFORMATION)) > dwNumberOfBytesTransfered) {
                    log_warn("Offset has invalid value and would overrun buffer: {}", offset);
                    break;
                }

                // Cast notify information to buffer
                notify_information = (FILE_NOTIFY_INFORMATION*)((char*)state->buffer + offset);

                // The strings in notify info follow it in the buffer, so use the 'NextEntryOffset'
                // to check we won't go out of range reading from the buffer
                if ((offset + notify_information->NextEntryOffset) > dwNumberOfBytesTransfered) {
                    log_warn("Next offset has invalid value and would overrun buffer: {}", offset);
                    break;
                }

                // Additional checks to catch string going out of range
                size_t fn_offset = reinterpret_cast<char*>(notify_information->FileName) - state->buffer;
                size_t fn_end = fn_offset + notify_information->FileNameLength;
                if (fn_end > dwNumberOfBytesTransfered) {
                    log_warn("Filename would overrun buffer: {}", offset);
                    break;
                }

                // Now safely read string
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
                    FALSE,
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
#endif // SGL_WINDOWS


#if !SGL_WINDOWS && !SGL_LINUX
static std::map<std::filesystem::path, std::filesystem::file_time_type>
get_directory_files(const std::filesystem::path& directory)
{
    if (!std::filesystem::exists(directory))
        return {};

    std::map<std::filesystem::path, std::filesystem::file_time_type> files;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::error_code ec;
            std::filesystem::path rel_path = std::filesystem::relative(entry.path(), directory, ec);
            if (ec) {
                log_warn("Failed to get relative path for file \"{}\"", entry.path());
                continue;
            }
            std::filesystem::file_time_type write_time = std::filesystem::last_write_time(entry.path(), ec);
            if (ec) {
                log_warn("Failed to get last write time for file \"{}\"", entry.path());
                continue;
            }
            files.emplace(rel_path, write_time);
        }
    }
    return files;
}
#endif

FileSystemWatcher::FileSystemWatcher()
{
#if SGL_LINUX
    m_inotify_file_descriptor = inotify_init();
    if (m_inotify_file_descriptor < 0) {
        SGL_THROW("Failed to initialize inotify file descriptor");
    }

    int flags = fcntl(m_inotify_file_descriptor, F_GETFL, 0);
    if (flags == -1) {
        close(m_inotify_file_descriptor);
        SGL_THROW("Failed to get inotify file descriptor flags");
    }

    flags |= O_NONBLOCK;
    if (fcntl(m_inotify_file_descriptor, F_SETFL, flags) == -1) {
        close(m_inotify_file_descriptor);
        SGL_THROW("Failed to set inotify file descriptor flags to non-blocking");
    }
#endif

#if !SGL_WINDOWS && !SGL_LINUX
    m_thread = std::thread([this]() { thread_func(); });
#endif
}

FileSystemWatcher::~FileSystemWatcher()
{
    for (const auto& pair : m_watches) {
        stop_watch(pair.second);
    }
#if SGL_LINUX
    close(m_inotify_file_descriptor);
#endif

#if !SGL_WINDOWS && !SGL_LINUX
    m_stop_thread = true;
    if (m_thread.joinable())
        m_thread.join();
#endif
}

uint32_t FileSystemWatcher::add_watch(const FileSystemWatchDesc& desc)
{
#if !SGL_WINDOWS && !SGL_LINUX
    std::lock_guard<std::mutex> lock(m_watches_mutex);
#endif

    // Check watch doesn't already exist
    for (const auto& pair : m_watches) {
        if (pair.second->desc.directory == desc.directory) {
            SGL_THROW("A watch already exists for {}.", desc.directory);
        }
    }

    // Check that directory exists.
    if (!std::filesystem::exists(desc.directory))
        SGL_THROW("Directory {} does not exist", desc.directory);

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
    std::memset(&state->overlapped, 0, sizeof(OVERLAPPED));
    state->overlapped.hEvent = this;
    state->is_shutdown = false;
    if (!ReadDirectoryChangesW(
            state->directory_handle,
            state->buffer,
            sizeof(state->buffer),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES
                | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SECURITY,
            NULL,
            &state->overlapped,
            &FileIOCompletionRoutine
        )) {
        SGL_THROW("ReadDirectoryChangesW failed. Error: {}\n", GetLastError());
    }
#elif SGL_LINUX
    // On linux, add a watch to the inotify file descriptor.
    state->watch_descriptor = inotify_add_watch(
        m_inotify_file_descriptor,
        state->desc.directory.c_str(),
        IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO
    );
    if (state->watch_descriptor < 0) {
        SGL_THROW("Failed to add watch to inotify file descriptor");
    }
#endif

#if !SGL_WINDOWS && !SGL_LINUX
    state->files = get_directory_files(state->desc.directory);
#endif

    m_watches[id] = std::move(state);
    return id;
}

void FileSystemWatcher::remove_watch(uint32_t id)
{
#if !SGL_WINDOWS && !SGL_LINUX
    std::lock_guard<std::mutex> lock(m_watches_mutex);
#endif

    stop_watch(m_watches[id]);
    m_watches.erase(id);
}

void FileSystemWatcher::remove_watch(const std::filesystem::path& directory)
{
#if !SGL_WINDOWS && !SGL_LINUX
    std::lock_guard<std::mutex> lock(m_watches_mutex);
#endif

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
    // Note: on windows this is normally a few milliseconds, but sometimes it
    // is failing to report shutdown correctly.
    // TODO: Look into more robust shutdown detection.
    CancelIo(state->directory_handle);
    for (int it = 0; it < 100; it++) {
        if (state->is_shutdown)
            break;
        SleepEx(5, TRUE);
    }
    if (!state->is_shutdown)
        log_warn("File system watch failed to shutdown after 500ms");
    CloseHandle(state->directory_handle);
#elif SGL_LINUX
    inotify_rm_watch(m_inotify_file_descriptor, state->watch_descriptor);
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
    std::filesystem::path abs_path = std::filesystem::absolute(state->desc.directory / path);

    auto now = std::chrono::system_clock::now();
    FileSystemWatchEvent event{
        .path = path,
        .absolute_path = abs_path,
        .change = change,
        .time = now,
    };
    {
#if !SGL_WINDOWS && !SGL_LINUX
        std::lock_guard<std::mutex> lock(m_queued_events_mutex);
#endif
        m_queued_events.push_back(event);
    }
    m_last_event = now;
}

void FileSystemWatcher::update()
{
#if SGL_WINDOWS
    SleepEx(0, TRUE);
#elif SGL_LINUX
    // Attempt non-blocking read from inotify, and return if no data.
    char buffer[4096];
    int length = read(m_inotify_file_descriptor, buffer, sizeof(buffer));
    if (length >= 0) {
        // Iterate over the inotify events and call '_notify_change' on the watcher for each one.
        int offset = 0;
        while (offset < length) {
            auto event = reinterpret_cast<inotify_event*>(buffer + offset);
            std::filesystem::path path{event->name};
            FileSystemWatcherChange change = FileSystemWatcherChange::invalid;

            if (event->mask & IN_CREATE) {
                change = FileSystemWatcherChange::added;
            } else if (event->mask & IN_DELETE) {
                change = FileSystemWatcherChange::removed;
            } else if (event->mask & IN_MODIFY) {
                change = FileSystemWatcherChange::modified;
            } else if (event->mask & IN_MOVED_TO) {
                change = FileSystemWatcherChange::renamed;
            }

            if (change != FileSystemWatcherChange::invalid) {
                auto it = std::find_if(
                    m_watches.begin(),
                    m_watches.end(),
                    [event](const auto& pair) { return pair.second->watch_descriptor == event->wd; }
                );
                if (it != m_watches.end()) {
                    _notify_change(it->second.get(), path, change);
                }
            }
            offset += sizeof(struct inotify_event) + event->len;
        }
    } else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EBADF) {
        SGL_THROW("Failed to read from inotify file descriptor");
    }
#endif

    // Process queued events.
    {
#if !SGL_WINDOWS && !SGL_LINUX
        std::lock_guard<std::mutex> lock(m_queued_events_mutex);
#endif
        if (m_queued_events.size() > 0) {
            auto duration = std::chrono::system_clock::now() - m_last_event;
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            if (millis > m_output_delay_ms) {
                m_on_change(m_queued_events);
                m_queued_events.clear();
            }
        }
    }
}

#if !SGL_WINDOWS && !SGL_LINUX
void FileSystemWatcher::thread_func()
{
    uint32_t counter = 0;
    while (!m_stop_thread) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (counter++ % 10 != 0)
            continue;

        std::lock_guard<std::mutex> lock(m_watches_mutex);
        for (const auto& [_, state] : m_watches) {
            std::map<std::filesystem::path, std::filesystem::file_time_type> files
                = get_directory_files(state->desc.directory);

            // Detect added and modified files.
            for (const auto& [path, write_time] : files) {
                auto it = state->files.find(path);
                if (it == state->files.end()) {
                    _notify_change(state.get(), path, FileSystemWatcherChange::added);
                } else if (it->second != write_time) {
                    _notify_change(state.get(), path, FileSystemWatcherChange::modified);
                }
            }
            // Detect removed files.
            for (const auto& [path, write_time] : state->files) {
                auto it = files.find(path);
                if (it == files.end()) {
                    _notify_change(state.get(), path, FileSystemWatcherChange::removed);
                }
            }

            state->files = std::move(files);
        }
    }
}
#endif

} // namespace sgl
