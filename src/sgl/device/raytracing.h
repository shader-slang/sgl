// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/device/fwd.h"
#include "sgl/device/types.h"
#include "sgl/device/device_resource.h"
#include "sgl/device/resource.h"

#include "sgl/math/vector_types.h"
#include "sgl/math/matrix_types.h"

#include "sgl/core/macros.h"
#include "sgl/core/object.h"
#include "sgl/core/enum.h"
#include "sgl/core/static_vector.h"
#include "sgl/core/short_vector.h"

#include <slang-rhi.h>

#include <variant>

namespace sgl {

using AccelerationStructureHandle = rhi::AccelerationStructureHandle;

enum class AccelerationStructureGeometryFlags : uint32_t {
    none = static_cast<uint32_t>(rhi::AccelerationStructureGeometryFlags::None),
    opaque = static_cast<uint32_t>(rhi::AccelerationStructureGeometryFlags::Opaque),
    no_duplicate_any_hit_invocation
    = static_cast<uint32_t>(rhi::AccelerationStructureGeometryFlags::NoDuplicateAnyHitInvocation),
};

SGL_ENUM_CLASS_OPERATORS(AccelerationStructureGeometryFlags);
SGL_ENUM_INFO(
    AccelerationStructureGeometryFlags,
    {
        {AccelerationStructureGeometryFlags::none, "none"},
        {AccelerationStructureGeometryFlags::opaque, "opaque"},
        {AccelerationStructureGeometryFlags::no_duplicate_any_hit_invocation, "no_duplicate_any_hit_invocation"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureGeometryFlags);

enum class AccelerationStructureInstanceFlags : uint32_t {
    none = static_cast<uint32_t>(rhi::AccelerationStructureInstanceFlags::None),
    triangle_facing_cull_disable
    = static_cast<uint32_t>(rhi::AccelerationStructureInstanceFlags::TriangleFacingCullDisable),
    triangle_front_counter_clockwise
    = static_cast<uint32_t>(rhi::AccelerationStructureInstanceFlags::TriangleFrontCounterClockwise),
    force_opaque = static_cast<uint32_t>(rhi::AccelerationStructureInstanceFlags::ForceOpaque),
    no_opaque = static_cast<uint32_t>(rhi::AccelerationStructureInstanceFlags::NoOpaque),
};

SGL_ENUM_CLASS_OPERATORS(AccelerationStructureInstanceFlags);
SGL_ENUM_INFO(
    AccelerationStructureInstanceFlags,
    {
        {AccelerationStructureInstanceFlags::none, "none"},
        {AccelerationStructureInstanceFlags::triangle_facing_cull_disable, "triangle_facing_cull_disable"},
        {AccelerationStructureInstanceFlags::triangle_front_counter_clockwise, "triangle_front_counter_clockwise"},
        {AccelerationStructureInstanceFlags::force_opaque, "force_opaque"},
        {AccelerationStructureInstanceFlags::no_opaque, "no_opaque"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureInstanceFlags);

struct AccelerationStructureInstanceDesc {
    float3x4 transform;
    uint32_t instance_id : 24;
    uint32_t instance_mask : 8;
    uint32_t instance_contribution_to_hit_group_index : 24;
    AccelerationStructureInstanceFlags flags_ : 8;
    AccelerationStructureHandle acceleration_structure;

    AccelerationStructureInstanceFlags flags() const { return static_cast<AccelerationStructureInstanceFlags>(flags_); }
    void set_flags(AccelerationStructureInstanceFlags flags)
    {
        flags_ = static_cast<AccelerationStructureInstanceFlags>(flags);
    }
};
static_assert(sizeof(AccelerationStructureInstanceDesc) == sizeof(rhi::AccelerationStructureInstanceDescGeneric));

struct AccelerationStructureBuildInputInstances {
    BufferOffsetPair instance_buffer;
    uint32_t instance_stride{0};
    uint32_t instance_count{0};
};

struct AccelerationStructureBuildInputTriangles {
    /// List of vertex buffers, one for each motion step.
    static_vector<BufferOffsetPair, 16> vertex_buffers;
    Format vertex_format{Format::unknown};
    uint32_t vertex_count{0};
    uint32_t vertex_stride{0};

    BufferOffsetPair index_buffer;
    IndexFormat index_format{IndexFormat::uint32};
    uint32_t index_count{0};

    /// Optional buffer containing 3x4 transform matrix applied to each vertex.
    BufferOffsetPair pre_transform_buffer;

    AccelerationStructureGeometryFlags flags{AccelerationStructureGeometryFlags::none};
};

struct AccelerationStructureBuildInputProceduralPrimitives {
    /// List of AABB buffers, one for each motion step.
    static_vector<BufferOffsetPair, 16> aabb_buffers;
    uint32_t aabb_stride{0};
    uint32_t primitive_count{0};

    AccelerationStructureGeometryFlags flags{AccelerationStructureGeometryFlags::none};
};

using AccelerationStructureBuildInput = std::variant<
    AccelerationStructureBuildInputInstances,
    AccelerationStructureBuildInputTriangles,
    AccelerationStructureBuildInputProceduralPrimitives>;

struct AccelerationStructureBuildInputMotionOptions {
    uint32_t key_count{1};
    float time_start{0.f};
    float time_end{1.f};
};

enum class AccelerationStructureBuildMode : uint32_t {
    build = static_cast<uint32_t>(rhi::AccelerationStructureBuildMode::Build),
    update = static_cast<uint32_t>(rhi::AccelerationStructureBuildMode::Update),
};

SGL_ENUM_INFO(
    AccelerationStructureBuildMode,
    {
        {AccelerationStructureBuildMode::build, "build"},
        {AccelerationStructureBuildMode::update, "update"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureBuildMode);

enum class AccelerationStructureBuildFlags : uint32_t {
    none = static_cast<uint32_t>(rhi::AccelerationStructureBuildFlags::None),
    allow_update = static_cast<uint32_t>(rhi::AccelerationStructureBuildFlags::AllowUpdate),
    allow_compaction = static_cast<uint32_t>(rhi::AccelerationStructureBuildFlags::AllowCompaction),
    prefer_fast_trace = static_cast<uint32_t>(rhi::AccelerationStructureBuildFlags::PreferFastTrace),
    prefer_fast_build = static_cast<uint32_t>(rhi::AccelerationStructureBuildFlags::PreferFastBuild),
    minimize_memory = static_cast<uint32_t>(rhi::AccelerationStructureBuildFlags::MinimizeMemory),
};

SGL_ENUM_CLASS_OPERATORS(AccelerationStructureBuildFlags);
SGL_ENUM_INFO(
    AccelerationStructureBuildFlags,
    {
        {AccelerationStructureBuildFlags::none, "none"},
        {AccelerationStructureBuildFlags::allow_update, "allow_update"},
        {AccelerationStructureBuildFlags::allow_compaction, "allow_compaction"},
        {AccelerationStructureBuildFlags::prefer_fast_trace, "prefer_fast_trace"},
        {AccelerationStructureBuildFlags::prefer_fast_build, "prefer_fast_build"},
        {AccelerationStructureBuildFlags::minimize_memory, "minimize_memory"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureBuildFlags);

struct AccelerationStructureBuildDesc {
    /// List of build inputs. All inputs must be of the same type.
    std::vector<AccelerationStructureBuildInput> inputs;

    AccelerationStructureBuildInputMotionOptions motion_options;

    AccelerationStructureBuildMode mode{AccelerationStructureBuildMode::build};
    AccelerationStructureBuildFlags flags{AccelerationStructureBuildFlags::none};
};

struct AccelerationStructureBuildDescConverter {
    rhi::AccelerationStructureBuildDesc rhi_desc;
    // TODO(slang-rhi) probably use short_vector instead, but short_vector needs some more work
    std::vector<rhi::AccelerationStructureBuildInput> rhi_build_inputs;
    AccelerationStructureBuildDescConverter(const AccelerationStructureBuildDesc& desc);
};

#if 0

    struct RayTracingTrianglesDesc {
        DeviceAddress transform3x4;
        Format index_format;
        Format vertex_format;
        uint32_t index_count;
        uint32_t vertex_count;
        DeviceAddress index_data;
        DeviceAddress vertex_data;
        DeviceSize vertex_stride;
    };
    // clang-format off
    static_assert(sizeof(RayTracingTrianglesDesc) == sizeof(rhi::IAccelerationStructure::TriangleDesc));
    static_assert(offsetof(RayTracingTrianglesDesc, transform3x4) == offsetof(rhi::IAccelerationStructure::TriangleDesc, transform3x4));
    static_assert(offsetof(RayTracingTrianglesDesc, index_format) == offsetof(rhi::IAccelerationStructure::TriangleDesc, indexFormat));
    static_assert(offsetof(RayTracingTrianglesDesc, vertex_format) == offsetof(rhi::IAccelerationStructure::TriangleDesc, vertexFormat));
    static_assert(offsetof(RayTracingTrianglesDesc, index_count) == offsetof(rhi::IAccelerationStructure::TriangleDesc, indexCount));
    static_assert(offsetof(RayTracingTrianglesDesc, vertex_count) == offsetof(rhi::IAccelerationStructure::TriangleDesc, vertexCount));
    static_assert(offsetof(RayTracingTrianglesDesc, index_data) == offsetof(rhi::IAccelerationStructure::TriangleDesc, indexData));
    static_assert(offsetof(RayTracingTrianglesDesc, vertex_data) == offsetof(rhi::IAccelerationStructure::TriangleDesc, vertexData));
    static_assert(offsetof(RayTracingTrianglesDesc, vertex_stride) == offsetof(rhi::IAccelerationStructure::TriangleDesc, vertexStride));
    // clang-format on

    struct RayTracingAABB {
        float3 min;
        float3 max;
    };
    static_assert(sizeof(RayTracingAABB) == 24);
    static_assert(offsetof(RayTracingAABB, min) == offsetof(rhi::IAccelerationStructure::ProceduralAABB, minX));
    static_assert(offsetof(RayTracingAABB, max) == offsetof(rhi::IAccelerationStructure::ProceduralAABB, maxX));

    struct RayTracingAABBsDesc {
        /// Number of AABBs.
        uint32_t count;

        /// Pointer to an array of `RayTracingAABB` values in device memory.
        DeviceAddress data;

        /// Stride in bytes of the AABB values array.
        DeviceSize stride;
    };
    // clang-format off
    static_assert(sizeof(RayTracingAABBsDesc) == sizeof(rhi::IAccelerationStructure::ProceduralAABBDesc));
    static_assert(offsetof(RayTracingAABBsDesc, count) == offsetof(rhi::IAccelerationStructure::ProceduralAABBDesc, count));
    static_assert(offsetof(RayTracingAABBsDesc, data) == offsetof(rhi::IAccelerationStructure::ProceduralAABBDesc, data));
    static_assert(offsetof(RayTracingAABBsDesc, stride) == offsetof(rhi::IAccelerationStructure::ProceduralAABBDesc, stride));
    // clang-format on

    struct RayTracingGeometryDesc {
        RayTracingGeometryType type;
        AccelerationStructureGeometryFlags flags;
        union {
            RayTracingTrianglesDesc triangles;
            RayTracingAABBsDesc aabbs;
        };
    };
    // clang-format off
    static_assert(sizeof(RayTracingGeometryDesc) == sizeof(rhi::IAccelerationStructure::GeometryDesc));
    static_assert(offsetof(RayTracingGeometryDesc, type) == offsetof(rhi::IAccelerationStructure::GeometryDesc, type));
    static_assert(offsetof(RayTracingGeometryDesc, flags) == offsetof(rhi::IAccelerationStructure::GeometryDesc, flags));
    static_assert(offsetof(RayTracingGeometryDesc, triangles) == offsetof(rhi::IAccelerationStructure::GeometryDesc, content));
    static_assert(offsetof(RayTracingGeometryDesc, aabbs) == offsetof(rhi::IAccelerationStructure::GeometryDesc, content));
    // clang-format on

#endif


#if 0

    struct RayTracingInstanceDesc {
        float3x4 transform;
        uint32_t instance_id : 24;
        uint32_t instance_mask : 8;
        uint32_t instance_contribution_to_hit_group_index : 24;
        uint32_t flags_ : 8; // Combination of RayTracingInstanceFlags values.
        DeviceAddress acceleration_structure;

        RayTracingInstanceFlags flags() const { return static_cast<RayTracingInstanceFlags>(flags_); }
        void set_flags(RayTracingInstanceFlags flags) { flags_ = static_cast<uint32_t>(flags); }
    };
    static_assert(sizeof(RayTracingInstanceDesc) == 64);

#endif

enum class AccelerationStructureCopyMode : uint32_t {
    clone = static_cast<uint32_t>(rhi::AccelerationStructureCopyMode::Clone),
    compact = static_cast<uint32_t>(rhi::AccelerationStructureCopyMode::Compact),
};

SGL_ENUM_INFO(
    AccelerationStructureCopyMode,
    {
        {AccelerationStructureCopyMode::clone, "clone"},
        {AccelerationStructureCopyMode::compact, "compact"},
    }
);
SGL_ENUM_REGISTER(AccelerationStructureCopyMode);

#if 0

    enum class AccelerationStructureKind : uint32_t {
        top_level = static_cast<uint32_t>(rhi::IAccelerationStructure::Kind::TopLevel),
        bottom_level = static_cast<uint32_t>(rhi::IAccelerationStructure::Kind::BottomLevel),
    };

    SGL_ENUM_INFO(
        AccelerationStructureKind,
        {
            {AccelerationStructureKind::top_level, "top_level"},
            {AccelerationStructureKind::bottom_level, "bottom_level"},
        }
    );
    SGL_ENUM_REGISTER(AccelerationStructureKind);

#endif

struct AccelerationStructureSizes {
    DeviceSize acceleration_structure_size{0};
    DeviceSize scratch_size{0};
    DeviceSize update_scratch_size{0};
};

#if 0

    struct AccelerationStructureBuildInputs {
        AccelerationStructureKind kind;

        AccelerationStructureBuildFlags flags;

        uint32_t desc_count{0};

        /// Array of `RayTracingInstanceDesc` values in device memory.
        /// Used when `kind` is `top_level`.
        DeviceAddress instance_descs{0};

        /// Array of `RayTracingGeometryDesc` values.
        /// Used when `kind` is `bottom_level`.
        const RayTracingGeometryDesc* geometry_descs{nullptr};
    };
    // clang-format off
    static_assert(sizeof(AccelerationStructureBuildInputs) == sizeof(rhi::IAccelerationStructure::BuildInputs));
    static_assert(offsetof(AccelerationStructureBuildInputs, kind) == offsetof(rhi::IAccelerationStructure::BuildInputs, kind));
    static_assert(offsetof(AccelerationStructureBuildInputs, flags) == offsetof(rhi::IAccelerationStructure::BuildInputs, flags));
    static_assert(offsetof(AccelerationStructureBuildInputs, desc_count) == offsetof(rhi::IAccelerationStructure::BuildInputs, descCount));
    static_assert(offsetof(AccelerationStructureBuildInputs, instance_descs) == offsetof(rhi::IAccelerationStructure::BuildInputs, instanceDescs));
    static_assert(offsetof(AccelerationStructureBuildInputs, geometry_descs) == offsetof(rhi::IAccelerationStructure::BuildInputs, geometryDescs));
    // clang-format on

#endif


struct AccelerationStructureQueryDesc {
    QueryType query_type;
    QueryPool* query_pool;
    uint32_t first_query_index;
};

struct AccelerationStructureDesc {
    DeviceSize size{0};
    std::string label;
};

class SGL_API AccelerationStructure : public DeviceResource {
    SGL_OBJECT(AccelerationStructure)
public:
    AccelerationStructure(ref<Device> device, AccelerationStructureDesc desc);
    ~AccelerationStructure();

    const AccelerationStructureDesc& desc() const { return m_desc; }

    AccelerationStructureHandle handle() const;

    rhi::IAccelerationStructure* rhi_acceleration_structure() const { return m_rhi_acceleration_structure; }

    std::string to_string() const override;

private:
    AccelerationStructureDesc m_desc;
    Slang::ComPtr<rhi::IAccelerationStructure> m_rhi_acceleration_structure;
};

class SGL_API AccelerationStructureInstanceList : public DeviceResource {
    SGL_OBJECT(AccelerationStructureInstanceList)
public:
    AccelerationStructureInstanceList(ref<Device> device, size_t size = 0);
    ~AccelerationStructureInstanceList();

    size_t size() const { return m_instances.size(); }
    void resize(size_t size)
    {
        m_instances.resize(size);
        m_dirty = true;
    }

    AccelerationStructureInstanceDesc& operator[](size_t index) { return m_instances[index]; }
    const AccelerationStructureInstanceDesc& operator[](size_t index) const
    {
        m_dirty = true;
        return m_instances[index];
    }

    ref<Buffer> buffer() const;

    std::string to_string() const override;

private:
    std::vector<AccelerationStructureInstanceDesc> m_instances;
    mutable bool m_dirty{true};
    mutable ref<Buffer> m_buffer;
};

struct ShaderTableDesc {
    ref<ShaderProgram> program;
    std::vector<std::string> ray_gen_entry_points;
    std::vector<std::string> miss_entry_points;
    std::vector<std::string> hit_group_names;
    std::vector<std::string> callable_entry_points;
};

class SGL_API ShaderTable : public DeviceResource {
    SGL_OBJECT(ShaderTable)
public:
    ShaderTable(ref<Device> device, ShaderTableDesc desc);
    ~ShaderTable();

    rhi::IShaderTable* rhi_shader_table() const { return m_rhi_shader_table; }

    std::string to_string() const override;

private:
    Slang::ComPtr<rhi::IShaderTable> m_rhi_shader_table;
};

} // namespace sgl
