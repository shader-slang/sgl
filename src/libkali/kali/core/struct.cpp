#include "struct.h"

#include "kali/core/format.h"
#include "kali/core/maths.h"
#include "kali/core/string.h"
#include "kali/core/hash.h"

#include "kali/math/float16.h"
#include "kali/math/colorspace.h"

#include <asmjit/asmjit.h>

#include <bit>
#include <limits>
#include <unordered_map>
#include <map>
#include <memory>
#include <mutex>

#define KALI_LOG_JIT_ASSEMBLY 1

namespace kali {

size_t hash(const Struct::Field& field)
{
    return hash(field.name, field.type, field.flags, field.size, field.offset, field.default_value);
}

std::string Struct::Field::to_string() const
{
    return fmt::format(
        "Field(name=\"{}\", type={}, flags={}, size={}, offset={}, default_value={})",
        name,
        type,
        flags,
        size,
        offset,
        default_value
    );
}

Struct::Struct(bool pack, ByteOrder byte_order)
    : m_pack(pack)
{
    m_byte_order = byte_order == ByteOrder::host ? host_byte_order() : byte_order;
}

Struct& Struct::append(Field field)
{
    KALI_CHECK(!field.name.empty(), "Field name cannot be empty.");
    KALI_CHECK(field.size == type_size(field.type), "Field size does not match type size.");
    KALI_CHECK(
        m_fields.empty() || field.offset >= m_fields.back().offset + m_fields.back().size,
        "Field offset overlaps with previous field."
    );
    m_fields.push_back(field);
    return *this;
}

Struct& Struct::append(std::string_view name, Type type, Flags flags, double default_value)
{
    size_t size = type_size(type);
    size_t offset = 0;
    if (!m_fields.empty()) {
        offset = m_fields.back().offset + m_fields.back().size;
        if (!m_pack)
            offset = align_to(size, offset);
    }
    return append({std::string(name), type, flags, size, offset, default_value});
}

const Struct::Field& Struct::field(std::string_view name) const
{
    for (const auto& field : m_fields)
        if (field.name == name)
            return field;
    KALI_THROW("Field \"{}\" not found.", name);
}

bool Struct::has_field(std::string_view name) const
{
    for (const auto& field : m_fields)
        if (field.name == name)
            return true;
    return false;
}

size_t Struct::size() const
{
    if (m_fields.empty())
        return 0;
    size_t size = 0;
    for (const auto& field : m_fields)
        size = std::max(size, field.offset + field.size);
    if (!m_pack)
        size = align_to(alignment(), size);
    return size;
}

size_t Struct::alignment() const
{
    if (m_pack)
        return 1;
    size_t alignment = 1;
    for (const auto& field : m_fields)
        alignment = std::max(alignment, field.size);
    return alignment;
}

size_t hash(const Struct& struct_)
{
    size_t hash = 0;
    for (const auto& field : struct_.fields())
        hash = hash_combine(hash, kali::hash(field));
    return hash;
}

std::string Struct::to_string() const
{
    return fmt::format(
        "Struct(\n"
        "    pack={},\n"
        "    byte_order={},\n"
        "    fields={},\n"
        "    size={},\n"
        "    alignment={}\n"
        ")",
        m_pack,
        m_byte_order,
        string::indent(string::list_to_string(std::span{m_fields})),
        size(),
        alignment()
    );
}

Struct::ByteOrder Struct::host_byte_order()
{
    if constexpr (std::endian::native == std::endian::little)
        return ByteOrder::little_endian;
    else
        return ByteOrder::big_endian;
}

std::pair<double, double> Struct::type_range(Type type)
{
    std::pair<double, double> range;

#define RANGE(type)                                                                                                    \
    range = std::make_pair<double, double>(                                                                            \
        static_cast<double>(std::numeric_limits<type>::min()),                                                         \
        static_cast<double>(std::numeric_limits<type>::max())                                                          \
    );                                                                                                                 \
    break;

    switch (type) {
    case Type::int8:
        RANGE(int8_t);
    case Type::int16:
        RANGE(int16_t);
    case Type::int32:
        RANGE(int32_t);
    case Type::int64:
        RANGE(int64_t);
    case Type::uint8:
        RANGE(uint8_t);
    case Type::uint16:
        RANGE(uint16_t);
    case Type::uint32:
        RANGE(uint32_t);
    case Type::uint64:
        RANGE(uint64_t);
    case Type::float16:
        RANGE(math::float16_t);
    case Type::float32:
        RANGE(float);
    case Type::float64:
        RANGE(double);
    default:
        KALI_THROW("Invalid type.");
    }

    if (is_integer(type)) {
        // Make bounds conservative to account for rounding errors.
        if (range.first < 0.0)
            range.first = std::nextafter(range.first, 0.0);
        range.second = std::nextafter(range.second, 0.f);
    }

    return range;
}

namespace detail {
    uint16_t byteswap(uint16_t v)
    {
#if KALI_MSVC
        return _byteswap_ushort(v);
#elif KALI_CLANG || KALI_GCC
        return __builtin_bswap16(v);
#else
        return (v >> 8) | (v << 8);
#endif
    }
    uint32_t byteswap(uint32_t v)
    {
#if KALI_MSVC
        return _byteswap_ulong(v);
#elif KALI_CLANG || KALI_GCC
        return __builtin_bswap32(v);
#else
        return (v >> 24) | ((v >> 8) & 0x0000FF00) | ((v << 8) & 0x00FF0000) | (v << 24);
#endif
    }
    uint64_t byteswap(uint64_t v)
    {
#if KALI_MSVC
        return _byteswap_uint64(v);
#elif KALI_CLANG || KALI_GCC
        return __builtin_bswap64(v);
#else
        return (v >> 56) | ((v >> 40) & 0x000000000000FF00) | ((v >> 24) & 0x0000000000FF0000)
            | ((v >> 8) & 0x00000000FF000000) | ((v << 8) & 0x000000FF00000000) | ((v << 24) & 0x0000FF0000000000)
            | ((v << 40) & 0x00FF000000000000) | (v << 56);
#endif
    }
    int16_t byteswap(int16_t v)
    {
        return std::bit_cast<int16_t>(byteswap(std::bit_cast<uint16_t>(v)));
    }
    int32_t byteswap(int32_t v)
    {
        return std::bit_cast<int32_t>(byteswap(std::bit_cast<uint32_t>(v)));
    }
    int64_t byteswap(int64_t v)
    {
        return std::bit_cast<int64_t>(byteswap(std::bit_cast<uint64_t>(v)));
    }
    float byteswap(float v)
    {
        return std::bit_cast<float>(byteswap(std::bit_cast<uint32_t>(v)));
    }
    double byteswap(double v)
    {
        return std::bit_cast<double>(byteswap(std::bit_cast<uint64_t>(v)));
    }
} // namespace detail


/// Op codes for conversion programs.
struct Op {
    enum class Type : uint8_t {
        load_mem,
        load_imm,
        save_mem,
        cast,
        linear_to_srgb,
        srgb_to_linear,
        multiply,
        round,
        clamp,
    };
    Type type;
    uint8_t reg;
    union {
        struct {
            size_t offset;
            Struct::Type type;
            bool swap;
        } load_mem;
        struct {
            double value;
        } load_imm;
        struct {
            size_t offset;
            Struct::Type type;
            bool swap;
        } save_mem;
        struct {
            Struct::Type from;
            Struct::Type to;
        } cast;
        struct {
            double value;
        } multiply;
        struct {
            double min;
            double max;
        } clamp;
    };
};

/// Virtual machine for running conversion programs.
struct VM {
    struct Value {
        Struct::Type type;
        union {
            /// Signed integer value (int8, int16, int32, int64).
            int64_t i;
            /// Unsigned integer value (uint8, uint16, uint32, uint64).
            uint64_t u;
            /// Single precision floating point value (float16, float32).
            float s;
            /// Double precision floating point value (float64).
            double d;
        };
    };

    const uint8_t* src;
    uint8_t* dst;
    Value registers[8];

    void run(std::span<const Op> code)
    {
        for (const Op& op : code) {
            Value& value = registers[op.reg];
            switch (op.type) {
            case Op::Type::load_mem:
                value = load(src, op.load_mem.offset, op.load_mem.type, op.load_mem.swap);
                break;
            case Op::Type::load_imm:
                value.type = Struct::Type::float64;
                value.d = op.load_imm.value;
                break;
            case Op::Type::save_mem:
                KALI_ASSERT(value.type == op.save_mem.type);
                save(dst, op.save_mem.offset, value, op.save_mem.swap);
                break;
            case Op::Type::cast: {
                KALI_ASSERT(value.type == op.cast.from);
                double tmp;
                if (Struct::is_integer(op.cast.from)) {
                    if (Struct::is_unsigned(op.cast.from))
                        tmp = static_cast<double>(value.u);
                    else
                        tmp = static_cast<double>(value.i);
                } else {
                    if (op.cast.from == Struct::Type::float64)
                        tmp = value.d;
                    else
                        tmp = value.s;
                }
                if (Struct::is_integer(op.cast.to)) {
                    if (Struct::is_unsigned(op.cast.to))
                        value.u = static_cast<uint64_t>(tmp);
                    else
                        value.i = static_cast<int64_t>(tmp);
                } else {
                    if (op.cast.to == Struct::Type::float64)
                        value.d = tmp;
                    else
                        value.s = static_cast<float>(tmp);
                }
                value.type = op.cast.to;
                break;
            }
            case Op::Type::linear_to_srgb:
                KALI_ASSERT(value.type == Struct::Type::float64);
                value.d = math::linear_to_srgb(value.d);
                break;
            case Op::Type::srgb_to_linear:
                KALI_ASSERT(value.type == Struct::Type::float64);
                value.d = math::srgb_to_linear(value.d);
                break;
            case Op::Type::multiply:
                KALI_ASSERT(value.type == Struct::Type::float64);
                value.d *= op.multiply.value;
                break;
            case Op::Type::round:
                KALI_ASSERT(value.type == Struct::Type::float64);
                value.d = std::rint(value.d);
                break;
            case Op::Type::clamp:
                KALI_ASSERT(value.type == Struct::Type::float64);
                value.d = std::clamp(value.d, op.clamp.min, op.clamp.max);
                break;
            }
        }
    }

    /// Load a value from memory.
    Value load(const uint8_t* base, size_t offset, Struct::Type type, bool swap)
    {
        Value value;
        value.type = type;

#define CASE(ctype, dst)                                                                                               \
    {                                                                                                                  \
        ctype v = *reinterpret_cast<const ctype*>(base + offset);                                                      \
        if (swap) [[unlikely]]                                                                                         \
            v = detail::byteswap(v);                                                                                   \
        dst = v;                                                                                                       \
        break;                                                                                                         \
    }

        switch (type) {
        case Struct::Type::int8:
            value.i = *reinterpret_cast<const int8_t*>(base + offset);
            break;
        case Struct::Type::int16:
            CASE(int16_t, value.i)
        case Struct::Type::int32:
            CASE(int32_t, value.i)
        case Struct::Type::int64:
            CASE(int64_t, value.i)
        case Struct::Type::uint8:
            value.u = *reinterpret_cast<const uint8_t*>(base + offset);
            break;
        case Struct::Type::uint16:
            CASE(uint16_t, value.u)
        case Struct::Type::uint32:
            CASE(uint32_t, value.u)
        case Struct::Type::uint64:
            CASE(uint64_t, value.u)
        case Struct::Type::float16: {
            uint16_t v = *reinterpret_cast<const uint16_t*>(base + offset);
            if (swap) [[unlikely]]
                v = detail::byteswap(v);
            value.s = math::float16_to_float32(v);
            break;
        }
        case Struct::Type::float32:
            CASE(float, value.s)
        case Struct::Type::float64:
            CASE(double, value.d)
        }

#undef CASE

        return value;
    }

    /// Save value to memory.
    void save(uint8_t* base, size_t offset, const Value& value, bool swap)
    {
#define CASE(ctype, src)                                                                                               \
    {                                                                                                                  \
        ctype v = static_cast<ctype>(src);                                                                             \
        if (swap) [[unlikely]]                                                                                         \
            v = detail::byteswap(v);                                                                                   \
        *reinterpret_cast<ctype*>(base + offset) = v;                                                                  \
        break;                                                                                                         \
    }

        switch (value.type) {
        case Struct::Type::int8:
            *reinterpret_cast<int8_t*>(base + offset) = static_cast<int8_t>(value.i);
            break;
        case Struct::Type::int16:
            CASE(int16_t, value.i)
        case Struct::Type::int32:
            CASE(int32_t, value.i)
        case Struct::Type::int64:
            CASE(int64_t, value.i)
        case Struct::Type::uint8:
            *reinterpret_cast<uint8_t*>(base + offset) = static_cast<uint8_t>(value.u);
            break;
        case Struct::Type::uint16:
            CASE(uint16_t, value.u)
        case Struct::Type::uint32:
            CASE(uint32_t, value.u)
        case Struct::Type::uint64:
            CASE(uint64_t, value.u)
        case Struct::Type::float16: {
            uint16_t v = math::float32_to_float16(value.s);
            if (swap) [[unlikely]]
                v = detail::byteswap(v);
            *reinterpret_cast<uint16_t*>(base + offset) = v;
            break;
        }
        case Struct::Type::float32:
            CASE(float, value.s)
        case Struct::Type::float64:
            CASE(double, value.d)
        }

#undef CASE
    }
};

/// Generate conversion program for converting from \c src_struct to \c dst_struct.
/// This generates code for the virtual machine.
/// If using a JIT compiler, this code can be compiled to native code.
std::vector<Op> generate_code(const Struct& src_struct, const Struct& dst_struct)
{
    std::vector<Op> code;

    const bool src_swap = src_struct.byte_order() != Struct::host_byte_order();
    const bool dst_swap = dst_struct.byte_order() != Struct::host_byte_order();

    for (const auto& dst_field : dst_struct.fields()) {

        if (src_struct.has_field(dst_field.name)) {
            const auto& src_field = src_struct.field(dst_field.name);

            // Load value from source struct.
            code.push_back(
                {.type = Op::Type::load_mem, .reg = 0, .load_mem = {src_field.offset, src_field.type, src_swap}}
            );

            // Convert value if types don't match.
            Struct::Flags flag_mask = Struct::Flags::normalized | Struct::Flags::srgb;
            if (src_field.type != dst_field.type || (src_field.flags & flag_mask) != (dst_field.flags & flag_mask)) {
                const auto src_range = Struct::type_range(src_field.type);
                const auto dst_range = Struct::type_range(dst_field.type);

                // Convert to double.
                code.push_back({.type = Op::Type::cast, .reg = 0, .cast = {src_field.type, Struct::Type::float64}});

                // Normalize source value.
                if (Struct::is_integer(src_field.type) && is_set(src_field.flags, Struct::Flags::normalized))
                    code.push_back({.type = Op::Type::multiply, .reg = 0, .multiply = {1.0 / src_range.second}});

                // Linearize source value.
                if (is_set(src_field.flags, Struct::Flags::srgb))
                    code.push_back({.type = Op::Type::srgb_to_linear, .reg = 0});

                // De-linearize destination value.
                if (is_set(dst_field.flags, Struct::Flags::srgb))
                    code.push_back({.type = Op::Type::linear_to_srgb, .reg = 0});

                // De-normalize destination value.
                if (Struct::is_integer(dst_field.type) && is_set(dst_field.flags, Struct::Flags::normalized))
                    code.push_back({.type = Op::Type::multiply, .reg = 0, .multiply = {dst_range.second}});

                // Round and clamp integers.
                if (Struct::is_integer(dst_field.type)) {
                    code.push_back({.type = Op::Type::round, .reg = 0});
                    code.push_back({.type = Op::Type::clamp, .reg = 0, .clamp = {dst_range.first, dst_range.second}});
                }

                code.push_back({.type = Op::Type::cast, .reg = 0, .cast = {Struct::Type::float64, dst_field.type}});
            }

            // Save value to source struct.
            code.push_back(
                {.type = Op::Type::save_mem, .reg = 0, .save_mem = {dst_field.offset, dst_field.type, dst_swap}}
            );
        } else if (is_set(dst_field.flags, Struct::Flags::default_)) {
            // Set default value.
            code.push_back({.type = Op::Type::load_imm, .reg = 0, .load_imm = {dst_field.default_value}});
            code.push_back({.type = Op::Type::cast, .reg = 0, .cast = {Struct::Type::float64, dst_field.type}});
            code.push_back(
                {.type = Op::Type::save_mem, .reg = 0, .save_mem = {dst_field.offset, dst_field.type, dst_swap}}
            );
        } else {
            KALI_THROW("Field \"{}\" not found in source struct.", dst_field.name);
        }
    }

    return code;
}


/// Interface for conversion programs.
struct Program {
    virtual ~Program() = default;
    virtual void execute(const void* src, void* dst, size_t count) const = 0;
};

/// Conversion program for the virtual machine.
struct VMProgram : public Program {
    std::vector<Op> code;
    size_t src_size;
    size_t dst_size;

    void execute(const void* src, void* dst, size_t count) const override
    {
        VM vm;
        vm.src = static_cast<const uint8_t*>(src);
        vm.dst = static_cast<uint8_t*>(dst);

        for (size_t i = 0; i < count; ++i) {
            vm.run(code);
            vm.src += src_size;
            vm.dst += dst_size;
        }
    }

    static std::unique_ptr<Program> compile(const Struct& src_struct, const Struct& dst_struct)
    {
        auto program = std::make_unique<VMProgram>();
        program->code = generate_code(src_struct, dst_struct);
        program->src_size = src_struct.size();
        program->dst_size = dst_struct.size();
        return program;
    }
};

/// Conversion program running just-in-time compiled X86 code.
struct X86Program : public Program {
    using ConvertFunc = void (*)(const void* src, void* dst, size_t count);

    ConvertFunc func;

    void execute(const void* src, void* dst, size_t count) const override { func(src, dst, count); }

    static std::unique_ptr<Program> compile(const Struct& src_struct, const Struct& dst_struct)
    {
        asmjit::CodeHolder code;
        code.init(runtime().environment(), runtime().cpuFeatures());

#if KALI_LOG_JIT_ASSEMBLY
        log_info("Compiling x86 program for converting from {} to {}", src_struct.to_string(), dst_struct.to_string());
        asmjit::StringLogger logger;
        logger.setFlags(asmjit::FormatFlags::kMachineCode);
        code.setLogger(&logger);
#endif
        asmjit::x86::Compiler c(&code);

        Builder builder(c);
        builder.build(src_struct, dst_struct);

        asmjit::Error err = c.finalize();
        if (err != asmjit::kErrorOk) {
            KALI_THROW("AsmJit failed: {}", asmjit::DebugUtils::errorAsString(err));
        }

#if KALI_LOG_JIT_ASSEMBLY
        log_info(logger.content().data());
#endif

        ConvertFunc func;
        runtime().add(&func, &code);

        auto program = std::make_unique<X86Program>();
        program->func = func;
        return program;
    }

private:
    /// Helper class to build X86 code.
    struct Builder {
        asmjit::x86::Compiler& c;
        bool has_avx;
        bool has_f16c;

        // Each register can hold either a 64-bit integer or a 64-bit floating point value.
        struct Register {
            uint32_t index;
            asmjit::x86::Gp gp;
            asmjit::x86::Xmm xmm;
        };

        std::map<uint32_t, Register> registers;

        Builder(asmjit::x86::Compiler& c)
            : c(c)
        {
            has_avx = runtime().cpuFeatures().x86().hasAVX();
            // has_avx = false;
            has_f16c = runtime().cpuFeatures().x86().hasF16C();
        }

        Register& get_register(uint32_t reg)
        {
            auto it = registers.find(reg);
            if (it != registers.end())
                return it->second;
            auto [it2, inserted] = registers.emplace(reg, Register{reg});
            return it2->second;
        }

        /// Load a constant value.
        asmjit::x86::Mem const_(double value) { return c.newDoubleConst(asmjit::ConstPoolScope::kGlobal, value); }

        /// Move operation (double word).
        template<typename X, typename Y>
        void movd(const X& x, const Y& y)
        {
            has_avx ? c.vmovd(x, y) : c.movd(x, y);
        }

        /// Move operation (quad word).
        template<typename X, typename Y>
        void movq(const X& x, const Y& y)
        {
            has_avx ? c.vmovq(x, y) : c.movq(x, y);
        }

        /// Move operation (scalar single).
        template<typename X, typename Y>
        void movss(const X& x, const Y& y)
        {
            has_avx ? c.vmovss(x, y) : c.movss(x, y);
        }

        /// Move operation (scalar double).
        template<typename X, typename Y>
        void movsd(const X& x, const Y& y)
        {
            has_avx ? c.vmovsd(x, y) : c.movsd(x, y);
        }

        void movsd(const asmjit::x86::Xmm& x, const asmjit::x86::Xmm& y)
        {
            has_avx ? c.vmovsd(x, x, y) : c.movsd(x, y);
        }

        /// Floating point comparison.
        template<typename X, typename Y>
        void ucomisd(const X& x, const Y& y)
        {
            has_avx ? c.vucomisd(x, y) : c.ucomisd(x, y);
        }

        /// Floating point addition operation.
        template<typename X, typename Y>
        void addsd(const X& x, const Y& y)
        {
            has_avx ? c.vaddsd(x, x, y) : c.addsd(x, y);
        }

        /// Floating point subtraction operation.
        template<typename X, typename Y, typename Z>
        void subsd(const X& x, const Y& y, const Z& z)
        {
            if (has_avx) {
                c.vsubsd(x, y, z);
            } else {
                c.movsd(x, y);
                c.subsd(x, z);
            }
        }

        /// Floating point multiplication operation.
        template<typename X, typename Y>
        void mulsd(const X& x, const Y& y)
        {
            has_avx ? c.vmulsd(x, x, y) : c.mulsd(x, y);
        }

        /// Floating point division operation.
        template<typename X, typename Y>
        void divsd(const X& x, const Y& y)
        {
            has_avx ? c.vdivsd(x, x, y) : c.divsd(x, y);
        }

        /// Floating point square root operation.
        template<typename X, typename Y>
        void sqrtsd(const X& x, const Y& y)
        {
            has_avx ? c.vsqrtsd(x, x, y) : c.sqrtsd(x, y);
        }

        /// Floating point maximum operation.
        template<typename X, typename Y>
        void maxsd(const X& x, const Y& y)
        {
            has_avx ? c.vmaxsd(x, x, y) : c.maxsd(x, y);
        }

        /// Floating point minimum operation.
        template<typename X, typename Y>
        void minsd(const X& x, const Y& y)
        {
            has_avx ? c.vminsd(x, x, y) : c.minsd(x, y);
        }

        /// Floating point rounding operation.
        template<typename X, typename Y>
        void roundsd(const X& x, const Y& y, asmjit::x86::RoundImm mode)
        {
            has_avx ? c.vroundsd(x, x, y, mode) : c.roundsd(x, y, mode);
        }

        /// Convert scalar double to signed integer.
        template<typename X, typename Y>
        void cvtsd2si(const X& x, const Y& y)
        {
            has_avx ? c.vcvtsd2si(x, y) : c.cvtsd2si(x, y);
        }

        /// Convert signed integer to scalar double.
        template<typename X, typename Y>
        void cvtsi2sd(const X& x, const Y& y)
        {
            has_avx ? c.vcvtsi2sd(x, x, y) : c.cvtsi2sd(x, y);
        }

        /// Convert scalar double to scalar float.
        template<typename X, typename Y>
        void cvtsd2ss(const X& x, const Y& y)
        {
            has_avx ? c.vcvtsd2ss(x, x, y) : c.cvtsd2ss(x, y);
        }

        /// Floating point fused multiply-add operation.
        template<typename X, typename Y, typename Z>
        void fmadd213sd(const X& x, const Y& y, const Z& z)
        {
            if (has_avx) {
                c.vfmadd213sd(x, y, z);
            } else {
                c.mulsd(x, y);
                c.addsd(x, z);
            }
        }

        void build(const Struct& src_struct, const Struct& dst_struct)
        {
            std::vector<Op> ops = generate_code(src_struct, dst_struct);

            auto comment = [this](std::string text)
            {
                text = "### " + text;
                c.comment(text.c_str());
            };

            auto node = c.addFunc(asmjit::FuncSignatureT<void, const void*, void*, size_t>(asmjit::CallConvId::kHost));
            auto src = c.newIntPtr("src");
            auto dst = c.newIntPtr("dst");
            auto count = c.newInt64("count");
            auto idx = c.newInt64("idx");

            node->setArg(0, src);
            node->setArg(1, dst);
            node->setArg(2, count);

            asmjit::Label loop_start = c.newLabel();
            asmjit::Label loop_end = c.newLabel();

            c.test(count, count);
            c.jz(loop_end);
            c.xor_(idx, idx);

            c.bind(loop_start);

            for (const Op& op : ops) {
                using namespace asmjit;

                switch (op.type) {
                case Op::Type::load_mem: {
                    Register& reg = get_register(op.reg);
                    comment(fmt::format(
                        "load_mem (reg={}, type={}, offset={}, swap={})",
                        reg.index,
                        op.load_mem.type,
                        op.load_mem.offset,
                        op.load_mem.swap
                    ));
                    load(reg, src, static_cast<int32_t>(op.load_mem.offset), op.load_mem.type, op.load_mem.swap);
                    break;
                }
                case Op::Type::load_imm: {
                    Register& reg = get_register(op.reg);
                    comment(fmt::format("load_imm (reg={}, value={})", reg.index, op.load_imm.value));
                    reg.xmm = c.newXmm();
                    movsd(reg.xmm, const_(op.load_imm.value));
                    break;
                }
                case Op::Type::save_mem: {
                    const Register& reg = get_register(op.reg);
                    comment(fmt::format(
                        "save_mem (reg={}, type={}, offset={}, swap={})",
                        reg.index,
                        op.save_mem.type,
                        op.save_mem.offset,
                        op.save_mem.swap
                    ));
                    save(reg, dst, static_cast<int32_t>(op.save_mem.offset), op.save_mem.type, op.save_mem.swap);
                    break;
                }
                case Op::Type::cast: {
                    Register& reg = get_register(op.reg);
                    comment(fmt::format("cast (reg={}, from={}, to={})", reg.index, op.cast.from, op.cast.to));
                    cast(reg, op.cast.from, op.cast.to);
                    break;
                }
                case Op::Type::linear_to_srgb: {
                    Register& reg = get_register(op.reg);
                    comment(fmt::format("linear_to_srgb (reg={})", op.reg));
                    reg.xmm = gamma(reg.xmm, true);
                    break;
                }
                case Op::Type::srgb_to_linear: {
                    Register& reg = get_register(op.reg);
                    comment(fmt::format("srgb_to_linear (reg={})", op.reg));
                    reg.xmm = gamma(reg.xmm, false);
                    break;
                }
                case Op::Type::multiply: {
                    const Register& reg = get_register(op.reg);
                    comment(fmt::format("multiply (reg={}, value={})", reg.index, op.multiply.value));
                    mulsd(reg.xmm, const_(op.multiply.value));
                    break;
                }
                case Op::Type::round: {
                    const Register& reg = get_register(op.reg);
                    comment(fmt::format("round (reg={})", reg.index));
                    roundsd(reg.xmm, reg.xmm, asmjit::x86::RoundImm::kNearest);
                    break;
                }
                case Op::Type::clamp: {
                    const Register& reg = get_register(op.reg);
                    comment(fmt::format("clamp (reg={}, min={}, max={})", reg.index, op.clamp.min, op.clamp.max));
                    maxsd(reg.xmm, const_(op.clamp.min));
                    minsd(reg.xmm, const_(op.clamp.max));
                    break;
                }
                }
            }

            c.inc(idx);
            c.add(src, asmjit::Imm(src_struct.size()));
            c.add(dst, asmjit::Imm(dst_struct.size()));
            c.cmp(idx, count);
            c.jne(loop_start);

            c.bind(loop_end);

            c.ret();
            c.endFunc();
        };

        /// Load a value from memory (base + offset) to a register.
        /// Optionally swaps the endianess of the value.
        /// Integer values are stored in a general purpose register.
        /// Floating point values are stored in a XMM register.
        /// Half precision floating point values are converted to single precision,
        /// either using the F16C instruction set or a software implementation.
        void load(Register& reg, const asmjit::x86::Gp& base, int32_t offset, Struct::Type type, bool swap)
        {
            using namespace asmjit;

            if (Struct::is_integer(type))
                reg.gp = c.newInt64();
            else
                reg.xmm = c.newXmm();

            switch (type) {
            case Struct::Type::int8:
                c.movsx(reg.gp, x86::byte_ptr(base, offset));
                break;
            case Struct::Type::int16:
                if (swap) {
                    x86::Gp tmp = c.newUInt16();
                    c.movzx(tmp, x86::word_ptr(base, offset));
                    c.xchg(tmp.r8Lo(), tmp.r8Hi());
                    c.movsx(reg.gp, tmp);
                } else {
                    c.movsx(reg.gp, x86::word_ptr(base, offset));
                }
                break;
            case Struct::Type::int32:
                if (swap) {
                    x86::Gp tmp = c.newUInt32();
                    c.mov(tmp, x86::dword_ptr(base, offset));
                    c.bswap(tmp);
                    c.movsxd(reg.gp, tmp);
                } else {
                    c.movsxd(reg.gp.r32(), x86::dword_ptr(base, offset));
                }
                break;
            case Struct::Type::int64:
                c.mov(reg.gp, x86::qword_ptr(base, offset));
                if (swap)
                    c.bswap(reg.gp.r64());
                break;
            case Struct::Type::uint8:
                c.movzx(reg.gp, x86::byte_ptr(base, offset));
                break;
            case Struct::Type::uint16:
                c.movzx(reg.gp, x86::word_ptr(base, offset));
                if (swap)
                    c.xchg(reg.gp.r8Lo(), reg.gp.r8Hi());
                break;
            case Struct::Type::uint32:
                c.mov(reg.gp.r32(), x86::dword_ptr(base, offset));
                if (swap)
                    c.bswap(reg.gp.r32());
                break;
            case Struct::Type::uint64:
                c.mov(reg.gp, x86::qword_ptr(base, offset));
                if (swap)
                    c.bswap(reg.gp.r64());
                break;
            case Struct::Type::float16: {
                x86::Gp tmp = c.newUInt16();
                c.movzx(tmp, x86::word_ptr(base, offset));
                if (swap)
                    c.xchg(tmp.r8Lo(), tmp.r8Hi());
                if (has_avx && has_f16c) {
                    c.vmovq(reg.xmm, tmp);
                    c.vcvtph2ps(reg.xmm, reg.xmm);
                } else {
                    InvokeNode* node;
                    c.invoke(
                        &node,
                        imm((void*)math::float16_to_float32),
                        FuncSignatureT<float, uint16_t>(CallConvId::kHost)
                    );
                    node->setArg(0, tmp);
                    node->setRet(0, reg.xmm);
                }
                break;
            }
            case Struct::Type::float32:
                if (swap) {
                    x86::Gp tmp = c.newUInt32();
                    c.mov(tmp, x86::dword_ptr(base, offset));
                    c.bswap(tmp);
                    movd(reg.xmm, tmp);
                } else {
                    movss(reg.xmm, x86::dword_ptr(base, offset));
                }
                break;
            case Struct::Type::float64:
                if (swap) {
                    x86::Gp tmp = c.newUInt64();
                    c.mov(tmp, x86::qword_ptr(base, offset));
                    c.bswap(tmp);
                    movq(reg.xmm, tmp);
                } else {
                    movsd(reg.xmm, x86::qword_ptr(base, offset));
                }
                break;
            }
        }

        /// Save a value from a register to memory (base + offset).
        /// Optionally swaps the endianess of the value.
        void save(const Register& reg, const asmjit::x86::Gp& base, int32_t offset, Struct::Type type, bool swap)
        {
            using namespace asmjit;

            switch (type) {
            case Struct::Type::int8:
            case Struct::Type::uint8:
                c.mov(x86::byte_ptr(base, offset), reg.gp.r8());
                break;
            case Struct::Type::int16:
            case Struct::Type::uint16:
                if (swap) {
                    x86::Gp tmp = c.newUInt16();
                    c.mov(tmp, reg.gp.r16());
                    c.xchg(tmp.r8Lo(), tmp.r8Hi());
                    c.mov(x86::word_ptr(base, offset), tmp.r16());
                } else {
                    c.mov(x86::word_ptr(base, offset), reg.gp.r16());
                }
                break;
            case Struct::Type::int32:
            case Struct::Type::uint32:
                if (swap) {
                    x86::Gp tmp = c.newUInt32();
                    c.mov(tmp, reg.gp.r32());
                    c.bswap(tmp);
                    c.mov(x86::dword_ptr(base, offset), tmp.r32());
                } else {
                    c.mov(x86::dword_ptr(base, offset), reg.gp.r32());
                }
                break;
            case Struct::Type::int64:
            case Struct::Type::uint64:
                if (swap) {
                    x86::Gp tmp = c.newUInt64();
                    c.mov(tmp, reg.gp);
                    c.bswap(tmp);
                    c.mov(x86::qword_ptr(base, offset), tmp);
                } else {
                    c.mov(x86::qword_ptr(base, offset), reg.gp);
                }
                break;
            case Struct::Type::float16: {
                x86::Gp tmp2 = c.newUInt16();
                if (has_avx && has_f16c) {
                    x86::Xmm tmp3 = c.newXmm();
                    c.vcvtps2ph(tmp3, reg.xmm, 0);
                    c.vmovd(tmp2, tmp3);
                } else {
                    InvokeNode* node;
                    c.invoke(
                        &node,
                        imm((void*)math::float32_to_float16),
                        FuncSignatureT<uint16_t, float>(CallConvId::kHost)
                    );
                    node->setArg(0, reg.xmm);
                    node->setRet(0, tmp2);
                }
                if (swap)
                    c.xchg(tmp2.r8Lo(), tmp2.r8Hi());
                c.mov(x86::word_ptr(base, offset), tmp2.r16());
                break;
            }
            case Struct::Type::float32:
                if (swap) {
                    x86::Gp tmp = c.newUInt32();
                    movd(tmp, reg.xmm);
                    c.bswap(tmp);
                    c.mov(x86::dword_ptr(base, offset), tmp);
                } else {
                    movss(x86::dword_ptr(base, offset), reg.xmm);
                }
                break;
            case Struct::Type::float64:
                if (swap) {
                    x86::Gp tmp = c.newUInt64();
                    movq(tmp, reg.xmm);
                    c.bswap(tmp);
                    c.mov(x86::qword_ptr(base, offset), tmp);
                } else {
                    movsd(x86::qword_ptr(base, offset), reg.xmm);
                }
                break;
            }
        }

        /// Convert a register from one type to another.
        void cast(Register& reg, Struct::Type from, Struct::Type to)
        {
            using namespace asmjit;

            x86::Xmm dbl = c.newXmm();
            if (Struct::is_integer(from)) {
                if (from == Struct::Type::uint32 || from == Struct::Type::int64) {
                    cvtsi2sd(dbl, reg.gp.r64());
                } else if (from == Struct::Type::uint64) {
                    auto tmp = c.newUInt64();
                    c.mov(tmp, reg.gp.r64());
                    auto tmp2 = c.newUInt64Const(asmjit::ConstPoolScope::kGlobal, 0x7fffffffffffffffull);
                    c.and_(tmp, tmp2);
                    cvtsi2sd(dbl, tmp.r64());
                    c.test(reg.gp.r64(), reg.gp.r64());
                    Label done = c.newLabel();
                    c.jns(done);
                    addsd(dbl, c.newUInt64Const(asmjit::ConstPoolScope::kGlobal, 0x8000000000000000ull));
                    c.bind(done);
                } else {
                    cvtsi2sd(dbl, reg.gp.r32());
                }
            } else {
                if (from == Struct::Type::float64)
                    movsd(dbl, reg.xmm);
                else
                    c.cvtss2sd(dbl, reg.xmm);
            }
            if (Struct::is_integer(to)) {
                if (to == Struct::Type::uint32 || to == Struct::Type::int64) {
                    reg.gp = c.newInt64();
                    cvtsd2si(reg.gp, dbl);
                } else if (to == Struct::Type::uint64) {
                    reg.gp = c.newInt64();
                    cvtsd2si(reg.gp.r64(), dbl);

                    x86::Xmm large_thresh = c.newXmm();
                    movsd(large_thresh, const_(9.223372036854776e18 /* 2^63 - 1 */));

                    x86::Xmm tmp = c.newXmm();
                    subsd(tmp, dbl, large_thresh);

                    x86::Gp tmp2 = c.newInt64();
                    cvtsd2si(tmp2, tmp);

                    x86::Gp large_result = c.newInt64();
                    c.mov(large_result, Imm(0x7fffffffffffffffull));
                    c.add(large_result, tmp2);

                    ucomisd(dbl, large_thresh);
                    c.cmovnb(reg.gp.r64(), large_result);
                } else {
                    reg.gp = c.newInt32();
                    cvtsd2si(reg.gp, dbl);
                }
            } else {
                reg.xmm = c.newXmm();
                if (to == Struct::Type::float64)
                    movsd(reg.xmm, dbl);
                else
                    cvtsd2ss(reg.xmm, dbl);
            }
        }

        /// Forward/inverse gamma correction using the sRGB profile
        asmjit::x86::Xmm gamma(asmjit::x86::Xmm x, bool to_srgb)
        {
            using namespace asmjit;

            x86::Xmm a = c.newXmm();
            x86::Xmm b = c.newXmm();

            movsd(a, const_(to_srgb ? 12.92 : (1.0 / 12.92)));
            ucomisd(x, const_(to_srgb ? 0.0031308 : 0.04045));

            Label low_value = c.newLabel();
            c.jb(low_value);

            x86::Xmm y;
            if (to_srgb) {
                y = c.newXmm();
                sqrtsd(y, x);
            } else {
                y = x;
            }

            // Rational polynomial fit, rel.err = 8*10^-15
            double to_srgb_coeffs[2][11] = {
                {
                    -0.0031151377052754843,
                    0.5838023820686707,
                    8.450947414259522,
                    27.901125077137042,
                    32.44669922192121,
                    15.374469584296442,
                    3.0477578489880823,
                    0.2263810267005674,
                    0.002531335520959116,
                    -0.00021805827098915798,
                    -3.7113872202050023e-6,
                },
                {
                    1.,
                    10.723011300050162,
                    29.70548706952188,
                    30.50364355650628,
                    13.297981743005433,
                    2.575446652731678,
                    0.21749170309546628,
                    0.007244514696840552,
                    0.00007045228641004039,
                    -8.387527630781522e-9,
                    2.2380622409188757e-11,
                },
            };

            // Rational polynomial fit, rel.err = 1.5*10^-15
            double from_srgb_coeffs[2][10] = {
                {
                    -342.62884098034357,
                    -3483.4445569178347,
                    -9735.250875334352,
                    -10782.158977031822,
                    -5548.704065887224,
                    -1446.951694673217,
                    -200.19589605282445,
                    -14.786385491859248,
                    -0.5489744177844188,
                    -0.008042950896814532,
                },
                {
                    1.,
                    -84.8098437770271,
                    -1884.7738197074218,
                    -8059.219012060384,
                    -11916.470977597566,
                    -7349.477378676199,
                    -2013.8039726540235,
                    -237.47722999429413,
                    -9.646075249097724,
                    -2.2132610916769585e-8,
                },
            };

            size_t ncoeffs
                = to_srgb ? std::extent_v<decltype(to_srgb_coeffs), 1> : std::extent_v<decltype(from_srgb_coeffs), 1>;

            for (size_t i = 0; i < ncoeffs; ++i) {
                for (int j = 0; j < 2; ++j) {
                    x86::Xmm& v = (j == 0) ? a : b;
                    x86::Mem coeff = const_(to_srgb ? to_srgb_coeffs[j][i] : from_srgb_coeffs[j][i]);
                    if (i == 0) {
                        movsd(v, coeff);
                    } else {
                        fmadd213sd(v, y, coeff);
                    }
                }
            }

            divsd(a, b);
            c.bind(low_value);
            mulsd(a, x);

            return a;
        }
    };

    static asmjit::JitRuntime& runtime()
    {
        static asmjit::JitRuntime instance;
        return instance;
    }
};


class ProgramCache {
public:
    const Program* get_program(const Struct& src_struct, const Struct& dst_struct)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto key = std::make_pair(src_struct, dst_struct);
        auto it = m_programs.find(key);
        if (it != m_programs.end())
            return it->second.get();
        auto [it2, inserted] = m_programs.emplace(key, compile_program(src_struct, dst_struct));
        return it2->second.get();
    }

    static ProgramCache& get()
    {
        static ProgramCache instance;
        return instance;
    }

private:
    std::unique_ptr<Program> compile_program(const Struct& src_struct, const Struct& dst_struct)
    {
        std::unique_ptr<Program> program;

#if KALI_X86_64
        program = X86Program::compile(src_struct, dst_struct);
#endif
        if (!program)
            program = VMProgram::compile(src_struct, dst_struct);

        return program;
    }

    std::mutex m_mutex;
    std::unordered_map<
        std::pair<Struct, Struct>,
        std::unique_ptr<Program>,
        hasher<std::pair<Struct, Struct>>,
        comparator<std::pair<Struct, Struct>>>
        m_programs;
};


StructConverter::StructConverter(ref<const Struct> src, ref<const Struct> dst)
    : m_src(std::move(src))
    , m_dst(std::move(dst))
{
}

void StructConverter::convert(const void* src, void* dst, size_t count) const
{
    // Direct copy if source and destination struct are the same.
    if (*m_src == *m_dst) {
        std::memcpy(dst, src, m_src->size() * count);
        return;
    }

    const Program* program = ProgramCache::get().get_program(*m_src, *m_dst);
    KALI_CHECK(program, "Failed to compile conversion program.");
    program->execute(src, dst, count);
}

std::string StructConverter::to_string() const
{
    return fmt::format(
        "StructConverter(\n"
        "    src={},\n"
        "    dst={}\n"
        ")",
        string::indent(m_src->to_string()),
        string::indent(m_dst->to_string())
    );
}

} // namespace kali