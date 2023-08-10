#include "reflection.h"

#include "kali/core/type_utils.h"

namespace kali {


std::map<uint32_t, std::string> ProgramLayout::get_hashed_strings() const
{
    std::map<uint32_t, std::string> result;
    for (uint32_t i = 0; i < m_layout->getHashedStringCount(); ++i) {
        size_t size;
        const char* str = m_layout->getHashedString(i, &size);
        uint32_t hash = spComputeStringHash(str, size);
        result.emplace(hash, std::string(str, str + size));
    }
    return result;
}

void ProgramLayout::dump()
{
    auto param_count = m_layout->getParameterCount();
    KALI_PRINT(param_count);
    for (uint32_t i = 0; i < param_count; ++i) {
        auto param = m_layout->getParameterByIndex(i);
        auto name = param->getName();
        auto type = param->getTypeLayout();
        auto type_name = type->getName();
        KALI_PRINT(name);
        KALI_PRINT(type_name);
    }
}

uint3 EntryPointLayout::get_compute_thread_group_size() const
{
    SlangUInt size[3];
    m_layout->getComputeThreadGroupSize(3, size);
    return uint3(narrow_cast<uint32_t>(size[0]), narrow_cast<uint32_t>(size[1]), narrow_cast<uint32_t>(size[2]));
}

} // namespace kali
