// SPDX-License-Identifier: Apache-2.0

#include "types.h"

#include "sgl/core/format.h"

namespace sgl {

std::string Viewport::to_string() const
{
    return fmt::format(
        "Viewport(x={}, y={}, width={}, height={}, min_depth={}, max_depth={})",
        x,
        y,
        width,
        height,
        min_depth,
        max_depth
    );
}

std::string ScissorRect::to_string() const
{
    return fmt::format("ScissorRect(min_x={}, min_y={}, max_x={}, max_y={})", min_x, min_y, max_x, max_y);
}

} // namespace sgl
