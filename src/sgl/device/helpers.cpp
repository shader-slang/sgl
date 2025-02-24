// SPDX-License-Identifier: Apache-2.0

#include "helpers.h"

#include "sgl/core/config.h"
#include "sgl/core/macros.h"
#include "sgl/core/format.h"

#include <string>

#if SGL_HAS_D3D12
#include <dxgidebug.h>
#include <dxgi1_3.h>
#endif

namespace sgl {


// Reads last error from graphics layer.
std::string get_last_rhi_layer_error()
{
#if SGL_HAS_D3D12
    IDXGIDebug* dxgiDebug = nullptr;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
    if (!dxgiDebug)
        return "";

    IDXGIInfoQueue* dxgiInfoQueue = nullptr;
    dxgiDebug->QueryInterface(IID_PPV_ARGS(&dxgiInfoQueue));
    if (!dxgiInfoQueue)
        return "";

    UINT64 messageCount = dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    if (messageCount == 0)
        return "";

    SIZE_T messageLength = 0;
    dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, messageCount - 1, nullptr, &messageLength);
    DXGI_INFO_QUEUE_MESSAGE* pMessage = (DXGI_INFO_QUEUE_MESSAGE*)malloc(messageLength);
    dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, messageCount - 1, pMessage, &messageLength);
    auto res = std::string(pMessage->pDescription);
    free(pMessage);
    return res;
#else
    // TODO: Get useful error information for other platforms if possible
    return "";
#endif
}

// Builds the user friendly message that is passed into a slang failure exception,
// used by SLANG_CALL.
std::string build_slang_failed_message(const char* call, SlangResult result)
{
    auto msg = fmt::format("Slang call {} failed with error: {}\n", call, result);
    if (static_cast<uint32_t>(result) >= 0x80000000U) {
        std::string rhi_error = get_last_rhi_layer_error();
        if (!rhi_error.empty()) {
            msg += "\nLast graphics layer error:\n" + rhi_error;
        }
    }
    return msg;
}

} // namespace sgl
