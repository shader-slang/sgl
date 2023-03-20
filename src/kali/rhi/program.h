#pragma once

#include "fwd.h"

#include "core/platform.h"
#include "core/object.h"

#include <slang-gfx.h>

namespace kali {

struct ProgramDesc {

};

class KALI_API Program : public Object {
public:
};

class KALI_API ProgramManager : public Object {
public:
private:
    Device* m_device;
    Slang::ComPtr<slang::IGlobalSession> m_slang_session;
};


}; // namespace kali
