#pragma once

#include "platform.h"

#if KALI_MSVC
#pragma warning(push)
#pragma warning(disable : 4061 4459)
#endif
#include <fmt/format.h>
#if KALI_MSVC
#pragma warning(pop)
#endif

#include <filesystem>
#include <optional>

template<>
struct fmt::formatter<std::filesystem::path> : formatter<std::string> {
    template<typename FormatContext>
    auto format(const std::filesystem::path& p, FormatContext& ctx)
    {
        return formatter<std::string>::format(p.string(), ctx);
    }
};

template<typename T>
struct fmt::formatter<std::optional<T>> : formatter<T> {
    template<typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx)
    {
        if (opt) {
            formatter<T>::format(*opt, ctx);
            return ctx.out();
        }
        return fmt::format_to(ctx.out(), "nullopt");
    }
};
