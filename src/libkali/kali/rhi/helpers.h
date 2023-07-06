#pragma once

#include "kali/error.h"

#include <slang-gfx.h>

#define SLANG_CALL(call)                                                                                               \
    {                                                                                                                  \
        SlangResult result_ = call;                                                                                    \
        if (SLANG_FAILED(result_))                                                                                     \
            KALI_THROW(RuntimeError("Slang call {} failed with error: {}", #call, result_));                           \
    }
