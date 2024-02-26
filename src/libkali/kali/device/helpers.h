// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/core/error.h"

#include <slang-gfx.h>

#define SLANG_CALL(call)                                                                                               \
    {                                                                                                                  \
        SlangResult result_ = call;                                                                                    \
        if (SLANG_FAILED(result_))                                                                                     \
            KALI_THROW("Slang call {} failed with error: {}", #call, result_);                                         \
    }
