#pragma once

// -------------------------------------------------------------------------------------------------
// D3D12 Agility SDK
// -------------------------------------------------------------------------------------------------

#if KALI_HAS_AGILITY_SDK
#define KALI_AGILITY_SDK_VERSION 611
#define KALI_AGILITY_SDK_PATH ".\\D3D12\\"
// To enable the D3D12 Agility SDK, this macro needs to be added to the main source file of the executable.
#define KALI_EXPORT_AGILITY_SDK                                                                                        \
    extern "C" {                                                                                                       \
    KALI_API_EXPORT extern const unsigned int D3D12SDKVersion = KALI_AGILITY_SDK_VERSION;                              \
    }                                                                                                                  \
    extern "C" {                                                                                                       \
    KALI_API_EXPORT extern const char* D3D12SDKPath = KALI_AGILITY_SDK_PATH;                                           \
    }
#else
#define KALI_EXPORT_AGILITY_SDK
#endif
