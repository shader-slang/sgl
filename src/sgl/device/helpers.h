// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/error.h"

#include <slang-gfx.h>
#include <format>


namespace sgl {
    SGL_API std::string BuildSlangFailedMessage(const char* call, SlangResult result);
}

#define SLANG_CALL(call)                                                                                               \
    {                                                                                                                  \
        SlangResult result_ = call;                                                                                    \
        if (SLANG_FAILED(result_)) {                                                                                   \
            SGL_THROW(BuildSlangFailedMessage(#call, result_));                                                        \
        }                                                                                                              \
    }
