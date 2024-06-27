// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/stream.h"
#include "sgl/core/enum.h"
#include "sgl/core/platform.h"

#include <map>
#include <functional>
#include <filesystem>
#include <chrono>

namespace sgl {

class FileSystemWatcher;
struct FileSystemWatchState;

enum class FileSystemWatcherChange {
    invalid,
    added,
    removed,
    modified,
    renamed,
};

struct FileSystemWatchDesc {
    std::filesystem::path path;
};

struct FileSystemWatchEvent {
    std::filesystem::path path;
    FileSystemWatcherChange change;
    std::chrono::system_clock::time_point time;
};

class FileSystemWatcher : public Object {
    SGL_OBJECT(FileSystemWatcher)
public:

    FileSystemWatcher();
    virtual ~FileSystemWatcher();

    void add_watch(const FileSystemWatchDesc& desc);
    void remove_watch(const std::filesystem::path& path);

    void set_on_change(std::function<void(std::vector<FileSystemWatchEvent>&)> on_change);

    void update();

    void _notify_change(const std::filesystem::path& path, FileSystemWatcherChange change);

private:
    int m_next_id = 1;
    std::map<int, FileSystemWatchState*> m_watches;
    std::function<void(std::vector<FileSystemWatchEvent>&)> m_on_change;
    std::vector<FileSystemWatchEvent> m_queued_events;
    std::chrono::system_clock::time_point m_last_event;

    FileSystemWatchState* get_watch(int id);
};


} // namespace sgl
