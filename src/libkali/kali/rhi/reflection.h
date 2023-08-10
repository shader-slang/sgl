#pragma once

#include "kali/core/macros.h"

#include "kali/math/vector_types.h"

#include <slang.h>

#include <map>
#include <string>

namespace kali {

class ProgramVersion;

class KALI_API ProgramLayout {
public:
    ProgramLayout(const ProgramVersion* program_version, slang::ProgramLayout* layout)
        : m_program_version(program_version)
        , m_layout(layout)
    {
    }

    /// Return a hash to string map of all hashed strings in the program.
    std::map<uint32_t, std::string> get_hashed_strings() const;

    void dump();

private:
    const ProgramVersion* m_program_version;
    slang::ProgramLayout* m_layout;
};

class KALI_API EntryPointLayout {
public:
    EntryPointLayout(const ProgramVersion* program_version, uint32_t entry_point, slang::EntryPointLayout* layout)
        : m_program_version(program_version)
        , m_entry_point(entry_point)
        , m_layout(layout)
    {
    }

    uint3 get_compute_thread_group_size() const;

private:
    const ProgramVersion* m_program_version;
    uint32_t m_entry_point;
    slang::EntryPointLayout* m_layout;
};

} // namespace kali
