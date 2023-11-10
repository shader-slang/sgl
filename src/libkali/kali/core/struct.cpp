#include "struct.h"

#include "kali/core/format.h"
#include "kali/core/maths.h"
#include "kali/core/string.h"
#include "kali/core/hash.h"

#include "kali/math/float16.h"
#include "kali/math/colorspace.h"

#include <bit>
#include <limits>
#include <unordered_map>
#include <memory>

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
#define RANGE(type)                                                                                                    \
    std::make_pair<double, double>(                                                                                    \
        static_cast<double>(std::numeric_limits<type>::min()),                                                         \
        static_cast<double>(std::numeric_limits<type>::max())                                                          \
    )

    switch (type) {
    case Type::int8:
        return RANGE(int8_t);
    case Type::int16:
        return RANGE(int16_t);
    case Type::int32:
        return RANGE(int32_t);
    case Type::int64:
        return RANGE(int64_t);
    case Type::uint8:
        return RANGE(uint8_t);
    case Type::uint16:
        return RANGE(uint16_t);
    case Type::uint32:
        return RANGE(uint32_t);
    case Type::uint64:
        return RANGE(uint64_t);
    case Type::float16:
        return RANGE(math::float16_t);
    case Type::float32:
        return RANGE(float);
    case Type::float64:
        return RANGE(double);
    }
    KALI_THROW("Invalid type.");
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

/// Base class for compiling conversion programs.
class Compiler {
public:
    using ConvertFunc = void (*)(const void* src, void* dst, size_t count);

    const Program* compile(const Struct& src_struct, const Struct& dst_struct)
    {
        auto key = std::make_pair(src_struct, dst_struct);
        auto it = m_program_cache.find(key);
        if (it != m_program_cache.end())
            return it->second.get();
        auto [it2, inserted] = m_program_cache.emplace(key, compile_program(src_struct, dst_struct));
        return it2->second.get();
    }

    virtual std::unique_ptr<Program> compile_program(const Struct& src_struct, const Struct& dst_struct) const = 0;

private:
    std::unordered_map<
        std::pair<Struct, Struct>,
        std::unique_ptr<Program>,
        hasher<std::pair<Struct, Struct>>,
        comparator<std::pair<Struct, Struct>>>
        m_program_cache;
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
};

/// Compiler for the virtual machine.
class VMCompiler : public Compiler {
public:
    std::unique_ptr<Program> compile_program(const Struct& src_struct, const Struct& dst_struct) const override
    {
        auto program = std::make_unique<VMProgram>();
        program->code = generate_code(src_struct, dst_struct);
        program->src_size = src_struct.size();
        program->dst_size = dst_struct.size();
        return program;
    }

    static VMCompiler& get()
    {
        static VMCompiler instance;
        return instance;
    }
};

class JITCompiler : public Compiler {
public:
    JITCompiler() { }

    std::unique_ptr<Program> compile_program(const Struct& src_struct, const Struct& dst_struct) const override
    {
        KALI_UNUSED(src_struct, dst_struct);
        return {};
    }

    static JITCompiler& get()
    {
        static JITCompiler instance;
        return instance;
    }
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

#if 1
    Compiler& compiler = VMCompiler::get();
#else
    Compiler& compiler = JITCompiler::get();
#endif
    const Program* program = compiler.compile(*m_src, *m_dst);
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
