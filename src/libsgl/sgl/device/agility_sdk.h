// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/config.h"

// -------------------------------------------------------------------------------------------------
// D3D12 Agility SDK
// -------------------------------------------------------------------------------------------------

#if SGL_HAS_AGILITY_SDK
#define SGL_AGILITY_SDK_VERSION 611
#define SGL_AGILITY_SDK_PATH ".\\d3d12\\"
// To enable the D3D12 Agility SDK, this macro needs to be added to the main source file of the executable.
#define SGL_EXPORT_AGILITY_SDK                                                                                         \
    extern "C" {                                                                                                       \
    SGL_API_EXPORT extern const unsigned int D3D12SDKVersion = SGL_AGILITY_SDK_VERSION;                                \
    }                                                                                                                  \
    extern "C" {                                                                                                       \
    SGL_API_EXPORT extern const char* D3D12SDKPath = SGL_AGILITY_SDK_PATH;                                             \
    }
#else
#define SGL_EXPORT_AGILITY_SDK
#endif
