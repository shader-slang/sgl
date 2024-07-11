// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/error.h"

#include <slang-gfx.h>


namespace sgl {
SGL_API std::string build_slang_failed_message(const char* call, SlangResult result);
}

#define SLANG_CALL(call)                                                                                               \
    {                                                                                                                  \
        SlangResult result_ = call;                                                                                    \
        if (SLANG_FAILED(result_)) {                                                                                   \
            SGL_THROW(build_slang_failed_message(#call, result_));                                                     \
        }                                                                                                              \
    }
