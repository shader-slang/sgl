#include "program.h"

namespace kali {

ProgramManager::ProgramManager(Device* device, slang::IGlobalSession* slang_session)
    : m_device(device)
    , m_slang_session(slang_session)
{
}

ref<Program> ProgramManager::create_program(const ProgramDesc& desc)
{
    KALI_UNUSED(desc);
    return nullptr;
}

} // namespace kali
