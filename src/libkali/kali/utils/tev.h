#pragma once

#include "kali/core/macros.h"
#include "kali/core/fwd.h"

#include "kali/device/fwd.h"

#include <string>
#include <optional>
#include <cstdint>

namespace kali::utils {

/**
 * @brief Show an image in the tev viewer (https://github.com/Tom94/tev).
 *
 * This will block until the image is sent over.
 *
 * @param bitmap Bitmap to show.
 * @param name Name of the image in tev. If not specified, a unique name will be generated.
 * @param host Host to connect to.
 * @param port Port to connect to.
 * @param max_retries Maximum number of retries.
 * @return True if successful.
 */
KALI_API bool show_in_tev(
    const Bitmap* bitmap,
    std::optional<std::string> name,
    const std::string& host = "127.0.0.1",
    uint16_t port = 14158,
    uint32_t max_retries = 3
);

KALI_API bool show_in_tev(
    const Texture* texture,
    std::optional<std::string> name,
    const std::string& host = "127.0.0.1",
    uint16_t port = 14158,
    uint32_t max_retries = 3
);

KALI_API void show_in_tev_async(
    const Bitmap* bitmap,
    std::optional<std::string> name,
    const std::string& host = "127.0.0.1",
    uint16_t port = 14158,
    uint32_t max_retries = 3
);

KALI_API void show_in_tev_async(
    const Texture* texture,
    std::optional<std::string> name,
    const std::string& host = "127.0.0.1",
    uint16_t port = 14158,
    uint32_t max_retries = 3
);

} // namespace kali::utils
