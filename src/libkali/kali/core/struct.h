#pragma once

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <utility>

namespace kali {

/**
 * @brief Structured data definition.
 *
 * This class is used to describe a structured data type layout.
 * It is used by the \ref StructConverter class to convert between different layouts.
 */
class KALI_API Struct : Object {
    KALI_OBJECT(Struct)
public:
    /// Struct field type.
    enum class Type {
        int8,
        int16,
        int32,
        int64,
        uint8,
        uint16,
        uint32,
        uint64,
        float16,
        float32,
        float64,
    };

    KALI_ENUM_INFO(
        Type,
        {
            {Type::int8, "int8"},
            {Type::int16, "int16"},
            {Type::int32, "int32"},
            {Type::int64, "int64"},
            {Type::uint8, "uint8"},
            {Type::uint16, "uint16"},
            {Type::uint32, "uint32"},
            {Type::uint64, "uint64"},
            {Type::float16, "float16"},
            {Type::float32, "float32"},
            {Type::float64, "float64"},
        }
    );

    /// Struct field flags.
    enum class Flags {
        none = 0,
        /// Integer fields represent normalized values in the range [0, 1].
        normalized = 1,
        /// Field encodes an sRGB value.
        srgb = 2,
        /// Use default when field is not present in the source struct during conversion.
        default_ = 4,
    };

    KALI_ENUM_INFO(
        Flags,
        {
            {Flags::none, "none"},
            {Flags::normalized, "normalized"},
            {Flags::srgb, "srgb"},
            {Flags::default_, "default"},
        }
    );

    /// Byte order.
    enum class ByteOrder {
        little_endian,
        big_endian,
        host,
    };

    KALI_ENUM_INFO(
        ByteOrder,
        {
            {ByteOrder::little_endian, "little_endian"},
            {ByteOrder::big_endian, "big_endian"},
            {ByteOrder::host, "host"},
        }
    );

    /// Struct field.
    struct KALI_API Field {
        /// Name of the field.
        std::string name;

        /// Type of the field.
        Type type{0};

        /// Field flags.
        Flags flags{Flags::none};

        /// Size of the field in bytes.
        size_t size{0};

        /// Offset of the field in bytes.
        size_t offset{0};

        /// Default value.
        double default_value{0.0};

        /// Check if the field is an integer type.
        bool is_integer() const { return Struct::is_integer(type); }

        /// Check if the field is a floating point type.
        bool is_float() const { return Struct::is_float(type); }

        /// Check if the field is an unsigned type.
        bool is_unsigned() const { return Struct::is_unsigned(type); }

        /// Check if the field is a signed type.
        bool is_signed() const { return Struct::is_signed(type); }

        /// Equality operator.
        bool operator==(const Field& other) const
        {
            return name == other.name && type == other.type && flags == other.flags && size == other.size
                && offset == other.offset && default_value == other.default_value;
        }

        /// Inequality operator.
        bool operator!=(const Field& other) const { return !operator==(other); }

        /// Compute hash from the field.
        KALI_API friend size_t hash(const Field& field);

        std::string to_string() const;
    };

    /// Constructor.
    /// @param pack If true, the struct will be packed.
    /// @param byte_order Byte order of the struct.
    Struct(bool pack = true, ByteOrder byte_order = ByteOrder::host);

    /// The byte order of the struct.
    ByteOrder byte_order() const { return m_byte_order; }

    /// Append a field to the struct.
    Struct& append(Field field);

    /// Append a field to the struct.
    /// @param name Name of the field.
    /// @param type Type of the field.
    /// @param flags Field flags.
    /// @param default_value Default value.
    /// @return Reference to the struct.
    Struct& append(std::string_view name, Type type, Flags flags = Flags::none, double default_value = 0.0);

    /// List of all fields.
    const std::vector<Field>& fields() const { return m_fields; }

    /// Access field by name. Throws if field is not found.
    const Field& field(std::string_view name) const;

    /// Check if a field with the specified name exists.
    bool has_field(std::string_view name) const;

    /// The size of the struct in bytes (with padding).
    size_t size() const;

    /// The alignment of the struct in bytes.
    size_t alignment() const;

    /// Equality operator.
    bool operator==(const Struct& other) const
    {
        return m_pack == other.m_pack && m_byte_order == other.m_byte_order && m_fields == other.m_fields;
    }

    /// Inequality operator.
    bool operator!=(const Struct& other) const { return !operator==(other); }

    /// Compute hash from the struct.
    KALI_API friend size_t hash(const Struct& struct_);

    std::string to_string() const override;

    /// The host byte order.
    static ByteOrder host_byte_order();

    /// Get the size of a type in bytes.
    static size_t type_size(Type type)
    {
        switch (type) {
        case Type::int8:
        case Type::uint8:
            return 1;
        case Type::int16:
        case Type::uint16:
        case Type::float16:
            return 2;
        case Type::int32:
        case Type::uint32:
        case Type::float32:
            return 4;
        case Type::int64:
        case Type::uint64:
        case Type::float64:
            return 8;
        }
        KALI_THROW("Invalid type.");
    }

    /// Get the numeric range of a type.
    static std::pair<double, double> type_range(Type type);

    /// Check if \c type is an integer type.
    static bool is_integer(Type type) { return !is_float(type); }

    /// Check if \c type is a floating point type.
    static bool is_float(Type type) { return type == Type::float16 || type == Type::float32 || type == Type::float64; }

    /// Check if \c type is an unsigned type.
    static bool is_unsigned(Type type)
    {
        return type == Type::uint8 || type == Type::uint16 || type == Type::uint32 || type == Type::uint64;
    }

    /// Check if \c type is a signed type.
    static bool is_signed(Type type) { return !is_unsigned(type); }


private:
    bool m_pack;
    ByteOrder m_byte_order;
    std::vector<Field> m_fields;
};

KALI_ENUM_REGISTER(Struct::Type);
KALI_ENUM_REGISTER(Struct::Flags);
KALI_ENUM_CLASS_OPERATORS(Struct::Flags);
KALI_ENUM_REGISTER(Struct::ByteOrder);

/**
 * @brief Struct converter.
 *
 * This helper class can be used to convert between structs with different layouts.
 */
class KALI_API StructConverter : Object {
    KALI_OBJECT(StructConverter)
public:
    /// Constructor.
    /// @param src Source struct definition.
    /// @param dst Destination struct definition.
    StructConverter(ref<const Struct> src, ref<const Struct> dst);

    /// The source struct definition.
    ref<const Struct> src() const { return m_src; }

    /// The destination struct definition.
    ref<const Struct> dst() const { return m_dst; }

    /// Convert data from source struct to destination struct.
    /// @param src Source data.
    /// @param dst Destination data.
    /// @param count Number of structs to convert.
    void convert(const void* src, void* dst, size_t count) const;

    std::string to_string() const override;

private:
    ref<const Struct> m_src;
    ref<const Struct> m_dst;
};

} // namespace kali