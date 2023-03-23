#pragma once

#include "core/error.h"

#include <slang-gfx.h>

#define SLANG_CALL(call)                                                                                               \
    {                                                                                                                  \
        SlangResult result = call;                                                                                     \
        if (SLANG_FAILED(result))                                                                                      \
            KALI_THROW(Exception("Slang call {} failed with error: {}", #call, result));                               \
    }
