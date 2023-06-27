#include "platform.h"
#include "format.h"

#if KALI_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#elif KALI_LINUX
#include <regex>
#include <execinfo.h>
#include <cxxabi.h>
#endif

namespace kali {

#if KALI_WINDOWS

StackTrace backtrace(size_t skip_frames)
{
    StackFrame stack[1024];
    size_t frame_count = CaptureStackBackTrace(DWORD(skip_frames), 1024, reinterpret_cast<PVOID*>(stack), nullptr);
    return {stack, stack + frame_count};
}

ResolvedStackTrace resolve_stacktrace(std::span<const StackFrame> trace)
{
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, true);

    struct Symbol : SYMBOL_INFO {
        char name_storage[1023];
    } symbol{};
    symbol.MaxNameLen = 1024;
    symbol.SizeOfStruct = sizeof(SYMBOL_INFO);

    IMAGEHLP_MODULE64 module{};
    module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    IMAGEHLP_LINE line{};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

    ResolvedStackTrace resolved_trace(trace.size());
    for (size_t i = 0; i < trace.size(); i++) {
        ResolvedStackFrame& resolved = resolved_trace[i];
        resolved.address = trace[i];
        resolved.offset = 0ull;
        if (SymFromAddr(process, resolved.address, &resolved.offset, &symbol))
            resolved.symbol = symbol.Name;
        if (SymGetModuleInfo64(process, symbol.ModBase, &module))
            resolved.module = module.ModuleName;
        DWORD displacement;
        if (SymGetLineFromAddr64(process, resolved.address, &displacement, &line)) {
            resolved.source = line.FileName ? line.FileName : "";
            resolved.line = line.LineNumber;
        }
    }

    return resolved_trace;
}

#elif KALI_LINUX

std::vector<StackTraceItem> backtrace(size_t skip_frames)
{
    auto demangle = [](const char* name)
    {
        int status = 0;
        char* buffer = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        std::string demangled{buffer ? buffer : name};
        free(buffer);
        return demangled;
    };

    void* raw_trace[100];
    int count = ::backtrace(raw_trace, 100);
    if (skip_frames >= count)
        return {};

    char** info = ::backtrace_symbols(raw_trace, count);

    std::regex re("(\\S+)\\((\\S*)\\+(0x[0-9a-f]*)\\)\\s+\\[(0x[0-9a-f]+)\\].*");

    std::vector<StackTraceItem> trace;
    trace.reserve(count - skip_frames);
    for (size_t i = skip_frames; i < count; i++) {
        std::cmatch m;
        StackTraceItem item{};
        if (std::regex_match(info[i], m, re)) {
            item.module = m[1];
            item.symbol = m[2];
            item.symbol = demangle(item.symbol.c_str());
            item.offset = std::stoul(m[3], nullptr, 16);
            item.address = std::stoul(m[4], nullptr, 16);
        } else {
            item.symbol = info[i];
        }
        trace.emplace_back(std::move(item));
    }
    free(info);

    return trace;
}

#endif

std::string format_stacktrace(std::span<const ResolvedStackFrame> trace)
{
    std::string result;
    for (const auto& item : trace) {
        if (item.source.empty()) {
            result += fmt::format("{:08x}: {}+{:#x} in {}\n", item.address, item.symbol, item.offset, item.module);
        } else {
            result += fmt::format(
                "{}({}): {}+{:#x} in {}\n",
                item.source,
                item.line,
                item.symbol,
                item.offset,
                item.module
            );
        }
    }
    return result;
}

std::string format_stacktrace(std::span<const StackFrame> trace)
{
    return format_stacktrace(resolve_stacktrace(trace));
}


} // namespace kali
