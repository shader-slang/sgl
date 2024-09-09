#include "sgl/device/cursor_utils.h"

namespace sgl {

// Helper class for checking if implicit conversion between scalar types is allowed.
// Note that only conversion between types of the same size is allowed.
struct ScalarConversionTable {
    static_assert(size_t(TypeReflection::ScalarType::COUNT) < 32, "Not enough bits to represent all scalar types");
    constexpr ScalarConversionTable()
    {
        for (uint32_t i = 0; i < uint32_t(TypeReflection::ScalarType::COUNT); ++i)
            table[i] = 1 << i;

        auto add_conversion = [&](TypeReflection::ScalarType from, auto... to)
        {
            uint32_t flags{0};
            ((flags |= 1 << uint32_t(to)), ...);
            table[uint32_t(from)] |= flags;
        };

        using ST = TypeReflection::ScalarType;
        add_conversion(ST::int32, ST::uint32, ST::bool_);
        add_conversion(ST::uint32, ST::int32, ST::bool_);
        add_conversion(ST::int64, ST::uint64);
        add_conversion(ST::uint64, ST::int64);
        add_conversion(ST::int8, ST::uint8);
        add_conversion(ST::uint8, ST::int8);
        add_conversion(ST::int16, ST::uint16);
        add_conversion(ST::uint16, ST::int16);
    }

    constexpr bool allow_conversion(TypeReflection::ScalarType from, TypeReflection::ScalarType to) const
    {
        return (table[uint32_t(from)] & (1 << uint32_t(to))) != 0;
    }

    uint32_t table[size_t(TypeReflection::ScalarType::COUNT)]{};
};

bool allow_scalar_conversion(TypeReflection::ScalarType from, TypeReflection::ScalarType to)
{
    static constexpr ScalarConversionTable table;
    return table.allow_conversion(from, to);
}

size_t get_scalar_type_size(TypeReflection::ScalarType type)
{
    switch (type) {
    case TypeReflection::ScalarType::int8:
    case TypeReflection::ScalarType::uint8:
        return 1;
    case TypeReflection::ScalarType::int16:
    case TypeReflection::ScalarType::uint16:
    case TypeReflection::ScalarType::float16:
        return 2;
    case TypeReflection::ScalarType::bool_:
    case TypeReflection::ScalarType::int32:
    case TypeReflection::ScalarType::uint32:
    case TypeReflection::ScalarType::float32:
        return 4;
    case TypeReflection::ScalarType::int64:
    case TypeReflection::ScalarType::uint64:
    case TypeReflection::ScalarType::float64:
        return 8;
    default:
        return 0;
    }
}

} // namespace sgl
