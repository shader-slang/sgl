#pragma once

#include "kali/device/device_resource.h"
#include "kali/device/resource.h"

#include "kali/math/vector_types.h"
#include "kali/math/matrix_types.h"

#include "kali/core/macros.h"
#include "kali/core/object.h"
#include "kali/core/enum.h"

#include <slang-gfx.h>

namespace kali {

class KALI_API AccelerationStructure : public DeviceResource {
    KALI_OBJECT(AccelerationStructure)
public:
    enum class Kind : uint32_t {
        top_level = static_cast<uint32_t>(gfx::IAccelerationStructure::Kind::TopLevel),
        bottom_level = static_cast<uint32_t>(gfx::IAccelerationStructure::Kind::BottomLevel),
    };

    KALI_ENUM_INFO(
        Kind,
        {
            {Kind::top_level, "top_level"},
            {Kind::bottom_level, "bottom_level"},
        }
    );

    enum class BuildFlags : uint32_t {
        none = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::None),
        allow_update = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::AllowUpdate),
        allow_compaction = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::AllowCompaction),
        prefer_fast_trace = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::PreferFastTrace),
        prefer_fast_build = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::PreferFastBuild),
        minimize_memory = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::MinimizeMemory),
        perform_update = static_cast<uint32_t>(gfx::IAccelerationStructure::BuildFlags::PerformUpdate),
    };

    KALI_ENUM_INFO(
        BuildFlags,
        {
            {BuildFlags::none, "none"},
            {BuildFlags::allow_update, "allow_update"},
            {BuildFlags::allow_compaction, "allow_compaction"},
            {BuildFlags::prefer_fast_trace, "prefer_fast_trace"},
            {BuildFlags::prefer_fast_build, "prefer_fast_build"},
            {BuildFlags::minimize_memory, "minimize_memory"},
            {BuildFlags::perform_update, "perform_update"},
        }
    );

    enum class GeometryType : uint32_t {
        triangles = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryType::Triangles),
        procedurel_primitives = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryType::ProcedurePrimitives),
    };

    KALI_ENUM_INFO(
        GeometryType,
        {
            {GeometryType::triangles, "triangles"},
            {GeometryType::procedurel_primitives, "procedurel_primitives"},
        }
    );

    enum class GeometryFlags : uint32_t {
        none = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryFlags::None),
        opaque = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryFlags::Opaque),
        no_duplicate_any_hit_invocation
        = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryFlags::NoDuplicateAnyHitInvocation),
    };

    KALI_ENUM_INFO(
        GeometryFlags,
        {
            {GeometryFlags::none, "none"},
            {GeometryFlags::opaque, "opaque"},
            {GeometryFlags::no_duplicate_any_hit_invocation, "no_duplicate_any_hit_invocation"},
        }
    );

    struct TriangleDesc {
        DeviceAddress transform3x4;
        Format index_format;
        Format vertex_format;
        uint32_t index_count;
        uint32_t vertex_count;
        DeviceAddress index_data;
        DeviceAddress vertex_data;
        DeviceSize vertex_stride;
    };

    struct ProceduralAABB {
        float3 min;
        float3 max;
    };
    static_assert(sizeof(ProceduralAABB) == 24);

    struct ProceduralAABBDesc {
        /// Number of AABBs.
        uint32_t count;

        /// Pointer to an array of `ProceduralAABB` values in device memory.
        DeviceAddress data;

        /// Stride in bytes of the AABB values array.
        DeviceSize stride;
    };

    struct GeometryDesc {
        GeometryType type;
        GeometryFlags flags;
        union {
            TriangleDesc triangles;
            ProceduralAABBDesc procedural_aabbs;
        } content;
    };

    enum class InstanceFlags : uint32_t {
        none = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::None),
        triangle_facing_cull_disable
        = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::TriangleFacingCullDisable),
        triangle_front_counter_clockwise
        = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::TriangleFrontCounterClockwise),
        force_opaque = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::ForceOpaque),
        no_opaque = static_cast<uint32_t>(gfx::IAccelerationStructure::GeometryInstanceFlags::NoOpaque),
    };

    KALI_ENUM_INFO(
        InstanceFlags,
        {
            {InstanceFlags::none, "none"},
            {InstanceFlags::triangle_facing_cull_disable, "triangle_facing_cull_disable"},
            {InstanceFlags::triangle_front_counter_clockwise, "triangle_front_counter_clockwise"},
            {InstanceFlags::force_opaque, "force_opaque"},
            {InstanceFlags::no_opaque, "no_opaque"},
        }
    );

    struct InstanceDesc {
        float3x4 transform;
        uint32_t instance_id : 24;
        uint32_t instance_mask : 8;
        uint32_t instance_contribution_to_hit_group_index : 24;
        uint32_t flags_ : 8; // Combination of InstanceFlags values.
        DeviceAddress acceleration_structure;

        InstanceFlags flags() const { return static_cast<InstanceFlags>(flags_); }
        void set_flags(InstanceFlags flags) { flags_ = static_cast<uint32_t>(flags); }
    };
    static_assert(sizeof(InstanceDesc) == 64);

    struct PrebuildInfo {
        DeviceSize result_data_max_size;
        DeviceSize scratch_data_size;
        DeviceSize update_scratch_data_size;
    };

    struct BuildInputs {
        Kind kind;

        BuildFlags flags;

        uint32_t desc_count;

        /// Array of `InstanceDesc` values in device memory.
        /// Used when `kind` is `TopLevel`.
        DeviceAddress instance_descs;

        /// Array of `GeometryDesc` values.
        /// Used when `kind` is `BottomLevel`.
        const GeometryDesc* geometry_descs;
    };

    struct Desc {
        Kind kind;
        ref<Buffer> buffer;
        DeviceOffset offset;
        DeviceSize size;
    };

    struct BuildDesc {
        BuildInputs inputs;
        AccelerationStructure* source;
        AccelerationStructure* dest;
        DeviceAddress scratch_data;
    };

    AccelerationStructure(ref<Device> device, Desc desc);

    const Desc& desc() const { return m_desc; }

    Kind kind() const { return m_desc.kind; }

    DeviceAddress device_address() const;

    gfx::IAccelerationStructure* gfx_acceleration_structure() const { return m_gfx_acceleration_structure; }

    std::string to_string() const override;

private:
    Desc m_desc;
    Slang::ComPtr<gfx::IAccelerationStructure> m_gfx_acceleration_structure;
};

KALI_ENUM_REGISTER(AccelerationStructure::Kind);
KALI_ENUM_REGISTER(AccelerationStructure::BuildFlags);
KALI_ENUM_REGISTER(AccelerationStructure::GeometryType);
KALI_ENUM_REGISTER(AccelerationStructure::GeometryFlags);
KALI_ENUM_REGISTER(AccelerationStructure::InstanceFlags);

KALI_ENUM_CLASS_OPERATORS(AccelerationStructure::BuildFlags);
KALI_ENUM_CLASS_OPERATORS(AccelerationStructure::GeometryFlags);
KALI_ENUM_CLASS_OPERATORS(AccelerationStructure::InstanceFlags);

} // namespace kali
