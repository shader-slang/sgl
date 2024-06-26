// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/stream.h"
#include "sgl/core/enum.h"
#include "sgl/core/platform.h"

#include <map>
#include <functional>
#include <filesystem>

namespace sgl {

class FileSystemWatcher;
struct FileSystemWatchState;

enum class FileSystemWatcherChange {
    Added,
    Removed,
    Modified,
    Renamed,
    Invalid
};

struct FileSystemWatchDesc {
    std::filesystem::path path;
};

class FileSystemWatcher : public Object {
    SGL_OBJECT(FileSystemWatcher)
public:

    FileSystemWatcher();
    virtual ~FileSystemWatcher();

    void add_watch(const FileSystemWatchDesc& desc);
    void remove_watch(const std::filesystem::path& path);

    void set_on_change(std::function<void(const std::filesystem::path&, FileSystemWatcherChange)> on_change);

    void notify_change(const std::filesystem::path& path, FileSystemWatcherChange change);

private:
    int m_next_id = 1;
    std::map<int, FileSystemWatchState*> m_watches;
    std::function<void(const std::filesystem::path&, FileSystemWatcherChange)> m_on_change;

    FileSystemWatchState* get_watch(int id);
};


} // namespace sgl
