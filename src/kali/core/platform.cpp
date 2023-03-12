#include "platform.h"

#include <fmt/format.h>

#if KALI_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#elif KALI_LINUX
#include <sstream>
#include <execinfo.h>
#include <cxxabi.h>
#endif

namespace kali {

#if KALI_WINDOWS

std::vector<StackTraceItem> backtrace(size_t skip_frames)
{
    void* stack[100];
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, true);
    size_t frame_count = CaptureStackBackTrace(DWORD(skip_frames), 100, stack, nullptr);

    struct Symbol : SYMBOL_INFO {
        char name_storage[1023];
    } symbol{};
    symbol.MaxNameLen = 1024;
    symbol.SizeOfStruct = sizeof(SYMBOL_INFO);

    IMAGEHLP_MODULE64 module{};
    module.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    std::vector<StackTraceItem> trace;
    trace.reserve(frame_count);
    for (size_t i = 0; i < frame_count; i++) {
        uint64_t address = reinterpret_cast<uint64_t>(stack[i]);
        uint64_t offset = 0ull;
        bool has_symbol = SymFromAddr(process, address, &offset, &symbol);
        bool has_module = SymGetModuleInfo64(process, symbol.ModBase, &module);
        StackTraceItem item{
            .module = has_module ? module.ModuleName : "?",
            .address = address,
            .symbol = has_symbol ? symbol.Name : "?",
            .offset = offset,
        };
        trace.emplace_back(std::move(item));
    }
    return trace;
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

    void* trace[100];
    int count = ::backtrace(trace, 100);
    if (skip_frames >= count)
        return {};

    char** info = ::backtrace_symbols(trace, count);

    std::vector<StackTraceItem> trace;
    trace.reserve(count - skip_frames);
    for (size_t i = skip_frames; i < count; i++) {
        std::istringstream iss{info[i]};
        char plus = '+';
        StackTraceItem item{};
        iss >> item.module >> std::hex >> item.address >> item.symbol >> plus >> std::dec >> item.offset;
        item.symbol = demangle(item.symbol.c_str());
        trace.emplace_back(std::move(item));
    }
    free(info);

    return trace;
}

#endif

std::string format_stacktrace(std::span<const StackTraceItem> trace)
{
    std::string result;
    for (const auto& item : trace)
        result += fmt::format("{:08x}: {}({}) in {}\n", item.address, item.symbol, item.offset, item.module);
    return result;
}

} // namespace kali
