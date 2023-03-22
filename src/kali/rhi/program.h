#pragma once

#include "fwd.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

#include <string>
#include <vector>
#include <filesystem>

namespace kali {

struct ProgramDesc {
    std::vector<std::filesystem::path> files;
    std::vector<std::string> entrypoints;

    static ProgramDesc create() { return {}; }

    ProgramDesc& add_file(std::filesystem::path path)
    {
        files.emplace_back(std::move(path));
        return *this;
    }

    ProgramDesc& add_entrypoint(std::string name)
    {
        entrypoints.emplace_back(std::move(name));
        return *this;
    }
};

class KALI_API Program : public Object {
public:
};

class KALI_API ProgramManager : public Object {
public:
    ProgramManager(Device* device, slang::IGlobalSession* slang_session);

    ref<Program> create_program(const ProgramDesc& desc);

private:
    Device* m_device;
    slang::IGlobalSession* m_slang_session;
};


}; // namespace kali
