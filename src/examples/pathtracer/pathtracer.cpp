// SPDX-License-Identifier: Apache-2.0

#include "kali/kali.h"

#include "kali/core/window.h"
#include "kali/core/input.h"
#include "kali/core/bitmap.h"
#include "kali/core/timer.h"

#include "kali/math/vector.h"
#include "kali/math/matrix.h"

#include "kali/device/device.h"
#include "kali/device/resource.h"
#include "kali/device/raytracing.h"
#include "kali/device/command.h"
#include "kali/device/shader.h"
#include "kali/device/pipeline.h"
#include "kali/device/kernel.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/swapchain.h"
#include "kali/device/agility_sdk.h"

#include "kali/utils/tev.h"

#include <map>
#include <memory>

KALI_EXPORT_AGILITY_SDK

static const std::filesystem::path EXAMPLE_DIR(KALI_EXAMPLE_DIR);

using namespace kali;

inline float3 random_float3()
{
    return float3(std::rand(), std::rand(), std::rand()) * (1.f / float(RAND_MAX));
}

struct Camera {
    uint32_t width{100};
    uint32_t height{100};
    float aspect_ratio{1.0f};
    float3 position{1, 1, 1};
    float3 target{0, 0, 0};
    float3 fwd;
    float3 right;
    float3 up{0, 1, 0};
    float fov{70.0f};

    float3 image_u;
    float3 image_v;
    float3 image_w;

    Camera() { recompute(); }

    void recompute()
    {
        aspect_ratio = float(width) / float(height);

        fwd = math::normalize(target - position);
        right = math::normalize(math::cross(fwd, up));
        up = math::normalize(math::cross(right, fwd));

        float fov_rad = math::radians(fov);

        image_u = right * math::tan(fov_rad * 0.5f) * aspect_ratio;
        image_v = up * math::tan(fov_rad * 0.5f);
        image_w = fwd;
    }

    void bind(ShaderCursor cursor) const
    {
        cursor["position"] = position;
        cursor["image_u"] = image_u;
        cursor["image_v"] = image_v;
        cursor["image_w"] = image_w;
    }
};


struct CameraController {
    Camera& camera;
    bool mouse_down{false};
    float2 mouse_pos;
    std::map<KeyCode, bool> key_state;
    bool shift_down{false};
    float3 move_delta{0.f};
    float2 rotate_delta{0.f};
    float move_speed{1.0f};
    float rotate_speed{0.002f};

    static constexpr float MOVE_SHIFT_FACTOR = 10.0f;
    static inline const std::map<KeyCode, float3> MOVE_KEYS = {
        {KeyCode::a, float3(-1, 0, 0)},
        {KeyCode::d, float3(1, 0, 0)},
        {KeyCode::e, float3(0, 1, 0)},
        {KeyCode::q, float3(0, -1, 0)},
        {KeyCode::w, float3(0, 0, 1)},
        {KeyCode::s, float3(0, 0, -1)},
    };

    CameraController(Camera& camera)
        : camera(camera)
    {
    }

    bool update(float dt)
    {
        bool changed = false;
        float3 position = camera.position;
        float3 fwd = camera.fwd;
        float3 up = camera.up;
        float3 right = camera.right;

        // Move
        if (math::length(move_delta) > 0) {
            float3 offset = right * move_delta.x;
            offset += up * move_delta.y;
            offset += fwd * move_delta.z;
            float factor = shift_down ? MOVE_SHIFT_FACTOR : 1.0f;
            offset *= move_speed * factor * dt;
            position += offset;
            changed = true;
        }

        // Rotate
        if (math::length(rotate_delta) > 0) {
            float yaw = math::atan2(fwd.z, fwd.x);
            float pitch = math::asin(fwd.y);
            yaw += rotate_speed * rotate_delta.x;
            pitch -= rotate_speed * rotate_delta.y;
            fwd = float3(math::cos(yaw) * math::cos(pitch), math::sin(pitch), math::sin(yaw) * math::cos(pitch));
            rotate_delta = float2();
            changed = true;
        }

        if (changed) {
            camera.position = position;
            camera.target = position + fwd;
            camera.up = float3(0, 1, 0);
            camera.recompute();
        }

        return changed;
    }

    void on_keyboard_event(const KeyboardEvent& event)
    {
        if (event.is_key_press() || event.is_key_release()) {
            bool down = event.is_key_press();
            if (MOVE_KEYS.contains(event.key)) {
                key_state[event.key] = down;
            } else if (event.key == KeyCode::left_shift) {
                shift_down = down;
            }
        }
        move_delta = float3(0.f);
        for (auto& [key, state] : key_state) {
            if (state) {
                move_delta += MOVE_KEYS.at(key);
            }
        }
    }

    void on_mouse_event(const MouseEvent& event)
    {
        rotate_delta = float2();
        if (event.is_button_down() && event.button == MouseButton::left) {
            mouse_down = true;
        }
        if (event.is_button_up() && event.button == MouseButton::left) {
            mouse_down = false;
        }
        if (event.is_move()) {
            float2 mouse_delta = event.pos - mouse_pos;
            if (mouse_down) {
                rotate_delta = mouse_delta;
            }
            mouse_pos = event.pos;
        }
    }
};

struct Material {
    float3 base_color;

    Material(float3 base_color = float3(0.5))
        : base_color(base_color)
    {
    }
};

struct Mesh {
    struct Vertex {
        float3 position;
        float3 normal;
        float2 uv;
    };
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
        : vertices(std::move(vertices))
        , indices(std::move(indices))
    {
    }

    uint32_t vertex_count() const { return narrow_cast<uint32_t>(vertices.size()); }
    uint32_t index_count() const { return narrow_cast<uint32_t>(indices.size()); }

    static Mesh create_quad(float2 size = float2(1))
    {
        std::vector<Vertex> vertices{
            // position, normal, uv
            {{-0.5, 0, -0.5}, {0, 1, 0}, {0, 0}},
            {{+0.5, 0, -0.5}, {0, 1, 0}, {1, 0}},
            {{-0.5, 0, +0.5}, {0, 1, 0}, {0, 1}},
            {{+0.5, 0, +0.5}, {0, 1, 0}, {1, 1}},
        };
        for (auto& vertex : vertices) {
            vertex.position *= float3(size.x, 0.f, size.y);
        }
        std::vector<uint32_t> indices{
            2,
            1,
            0,
            1,
            2,
            3,
        };
        return Mesh(vertices, indices);
    }

    static Mesh create_cube(float3 size = float3(1))
    {
        std::vector<Vertex> vertices{
            // position, normal, uv
            // left
            {{-0.5, -0.5, -0.5}, {0, -1, 0}, {0, 0}},
            {{-0.5, -0.5, +0.5}, {0, -1, 0}, {1, 0}},
            {{+0.5, -0.5, +0.5}, {0, -1, 0}, {1, 1}},
            {{+0.5, -0.5, -0.5}, {0, -1, 0}, {0, 1}},
            // right
            {{-0.5, +0.5, +0.5}, {0, +1, 0}, {0, 0}},
            {{-0.5, +0.5, -0.5}, {0, +1, 0}, {1, 0}},
            {{+0.5, +0.5, -0.5}, {0, +1, 0}, {1, 1}},
            {{+0.5, +0.5, +0.5}, {0, +1, 0}, {0, 1}},
            // back
            {{-0.5, +0.5, -0.5}, {0, 0, -1}, {0, 0}},
            {{-0.5, -0.5, -0.5}, {0, 0, -1}, {1, 0}},
            {{+0.5, -0.5, -0.5}, {0, 0, -1}, {1, 1}},
            {{+0.5, +0.5, -0.5}, {0, 0, -1}, {0, 1}},
            // front
            {{+0.5, +0.5, +0.5}, {0, 0, +1}, {0, 0}},
            {{+0.5, -0.5, +0.5}, {0, 0, +1}, {1, 0}},
            {{-0.5, -0.5, +0.5}, {0, 0, +1}, {1, 1}},
            {{-0.5, +0.5, +0.5}, {0, 0, +1}, {0, 1}},
            // bottom
            {{-0.5, +0.5, +0.5}, {-1, 0, 0}, {0, 0}},
            {{-0.5, -0.5, +0.5}, {-1, 0, 0}, {1, 0}},
            {{-0.5, -0.5, -0.5}, {-1, 0, 0}, {1, 1}},
            {{-0.5, +0.5, -0.5}, {-1, 0, 0}, {0, 1}},
            // top
            {{+0.5, +0.5, -0.5}, {+1, 0, 0}, {0, 0}},
            {{+0.5, -0.5, -0.5}, {+1, 0, 0}, {1, 0}},
            {{+0.5, -0.5, +0.5}, {+1, 0, 0}, {1, 1}},
            {{+0.5, +0.5, +0.5}, {+1, 0, 0}, {0, 1}},
        };
        for (auto& vertex : vertices) {
            vertex.position *= size;
        }
        std::vector<uint32_t> indices{
            0,  2,  1,  0,  3,  2,  4,  6,  5,  4,  7,  6,  8,  10, 9,  8,  11, 10,
            12, 14, 13, 12, 15, 14, 16, 18, 17, 16, 19, 18, 20, 22, 21, 20, 23, 22,
        };
        return Mesh(vertices, indices);
    }
};

struct Transform {
    float3 translation{0};
    float3 scaling{1};
    float3 rotation{0};
    float4x4 matrix;

    void update_matrix()
    {
        float4x4 T = math::matrix_from_translation(translation);
        float4x4 S = math::matrix_from_scaling(scaling);
        float4x4 R = math::matrix_from_rotation_xyz(rotation);
        matrix = math::mul(math::mul(T, R), S);
    }
};

struct Stage {
    Camera camera;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Transform> transforms;
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> instances;

    uint32_t add_material(const Material& material)
    {
        uint32_t material_id = uint32_t(materials.size());
        materials.push_back(material);
        return material_id;
    }

    uint32_t add_mesh(const Mesh& mesh)
    {
        uint32_t mesh_id = uint32_t(meshes.size());
        meshes.push_back(mesh);
        return mesh_id;
    }

    uint32_t add_transform(const Transform& transform)
    {
        uint32_t transform_id = uint32_t(transforms.size());
        transforms.push_back(transform);
        return transform_id;
    }

    uint32_t add_instance(uint32_t mesh_id, uint32_t material_id, uint32_t transform_id)
    {
        uint32_t instance_id = uint32_t(instances.size());
        instances.push_back(std::make_tuple(mesh_id, material_id, transform_id));
        return instance_id;
    }

    static std::unique_ptr<Stage> demo()
    {
        std::unique_ptr<Stage> stage = std::make_unique<Stage>();
        stage->camera.target = float3(0, 1, 0);
        stage->camera.position = float3(2, 1, 2);

        uint32_t floor_material = stage->add_material(Material(float3(0.5)));
        uint32_t floor_mesh = stage->add_mesh(Mesh::create_quad(float2(5, 5)));
        uint32_t floor_transform = stage->add_transform(Transform());
        stage->add_instance(floor_mesh, floor_material, floor_transform);

        std::vector<uint32_t> cube_materials;
        for (uint32_t i = 0; i < 10; ++i)
            cube_materials.push_back(stage->add_material(Material(random_float3())));
        uint32_t cube_mesh = stage->add_mesh(Mesh::create_cube(float3(0.1)));
        for (uint32_t i = 0; i < 1000; ++i) {
            Transform transform;
            transform.translation = random_float3() * 2.f - 1.f;
            transform.translation.y += 1;
            transform.scaling = random_float3() + 0.5f;
            transform.rotation = random_float3() * 10.f;
            transform.update_matrix();
            uint32_t cube_transform = stage->add_transform(transform);
            stage->add_instance(cube_mesh, cube_materials[i % cube_materials.size()], cube_transform);
        }

        return stage;
    }
};

struct Scene {
    struct MaterialDesc {
        float3 base_color;
    };

    struct MeshDesc {
        uint32_t vertex_count;
        uint32_t index_count;
        uint32_t vertex_offset;
        uint32_t index_offset;
    };

    struct InstanceDesc {
        uint32_t mesh_id;
        uint32_t material_id;
        uint32_t transform_id;
    };

    ref<Device> device;
    const Stage& stage;
    const Camera& camera;

    std::vector<MaterialDesc> material_descs;
    ref<Buffer> material_descs_buffer;
    std::vector<MeshDesc> mesh_descs;
    ref<Buffer> mesh_descs_buffer;
    std::vector<InstanceDesc> instance_descs;
    ref<Buffer> instance_descs_buffer;
    ref<Buffer> vertex_buffer;
    ref<Buffer> index_buffer;
    std::vector<float4x4> transforms;
    std::vector<float4x4> inverse_transpose_transforms;
    ref<Buffer> transforms_buffer;
    ref<Buffer> inverse_transpose_transforms_buffer;
    ref<Buffer> identity_buffer;
    std::vector<ref<AccelerationStructure>> blases;
    ref<AccelerationStructure> tlas;

    Scene(ref<Device> device, const Stage& stage)
        : device(device)
        , stage(stage)
        , camera(stage.camera)
    {
        // Prepare material descriptors
        material_descs.resize(stage.materials.size());
        for (size_t i = 0; i < stage.materials.size(); ++i)
            material_descs[i].base_color = stage.materials[i].base_color;

        material_descs_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "material_descs_buffer",
            .data = material_descs.data(),
            .data_size = material_descs.size() * sizeof(MaterialDesc),
        });

        // Prepare mesh descriptors
        mesh_descs.reserve(stage.meshes.size());
        uint32_t vertex_count = 0;
        uint32_t index_count = 0;
        for (const Mesh& mesh : stage.meshes) {
            mesh_descs.push_back(MeshDesc{
                .vertex_count = mesh.vertex_count(),
                .index_count = mesh.index_count(),
                .vertex_offset = vertex_count,
                .index_offset = index_count,
            });
            vertex_count += mesh.vertex_count();
            index_count += mesh.index_count();
        }

        // Preapre instance descriptors
        instance_descs.reserve(stage.instances.size());
        for (const auto& [mesh_id, material_id, transform_id] : stage.instances) {
            instance_descs.push_back({
                .mesh_id = mesh_id,
                .material_id = material_id,
                .transform_id = transform_id,
            });
        }

        // Create vertex and index buffers
        std::vector<Mesh::Vertex> vertices;
        std::vector<uint32_t> indices;
        vertices.reserve(vertex_count);
        indices.reserve(index_count);
        for (const Mesh& mesh : stage.meshes) {
            vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
            indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
        }
        vertex_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "vertex_buffer",
            .data = vertices.data(),
            .data_size = vertices.size() * sizeof(Mesh::Vertex),
        });
        index_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "index_buffer",
            .data = indices.data(),
            .data_size = indices.size() * sizeof(uint32_t),
        });

        mesh_descs_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "mesh_descs_buffer",
            .data = mesh_descs.data(),
            .data_size = mesh_descs.size() * sizeof(MeshDesc),
        });

        instance_descs_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "instance_descs_buffer",
            .data = instance_descs.data(),
            .data_size = instance_descs.size() * sizeof(InstanceDesc),
        });

        // Prepare transforms
        transforms.resize(stage.transforms.size());
        for (size_t i = 0; i < stage.transforms.size(); ++i)
            transforms[i] = stage.transforms[i].matrix;
        inverse_transpose_transforms.resize(transforms.size());
        for (size_t i = 0; i < transforms.size(); ++i)
            inverse_transpose_transforms[i] = transpose(inverse(transforms[i]));

        transforms_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "transforms_buffer",
            .data = transforms.data(),
            .data_size = transforms.size() * sizeof(float4x4),
        });

        inverse_transpose_transforms_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "inverse_transpose_transforms_buffer",
            .data = inverse_transpose_transforms.data(),
            .data_size = inverse_transpose_transforms.size() * sizeof(float4x4),
        });

        float3x4 identity = float3x4::identity();
        identity_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "identity_buffer",
            .data = &identity,
            .data_size = sizeof(identity),
        });

        // Build BLASes
        for (const MeshDesc& mesh_desc : mesh_descs)
            blases.push_back(build_blas(mesh_desc));

        // Build TLAS
        tlas = build_tlas();
    }

    ref<AccelerationStructure> build_blas(const MeshDesc& mesh_desc)
    {
        RayTracingGeometryDesc blas_geometry_desc;
        blas_geometry_desc.type = RayTracingGeometryType::triangles;
        blas_geometry_desc.flags = RayTracingGeometryFlags::opaque;
        blas_geometry_desc.triangles.transform3x4 = identity_buffer->device_address();
        blas_geometry_desc.triangles.index_format = Format::r32_uint;
        blas_geometry_desc.triangles.vertex_format = Format::rgb32_float;
        blas_geometry_desc.triangles.index_count = mesh_desc.index_count;
        blas_geometry_desc.triangles.vertex_count = mesh_desc.vertex_count;
        blas_geometry_desc.triangles.index_data = index_buffer->device_address() + mesh_desc.index_offset * 4;
        blas_geometry_desc.triangles.vertex_data = vertex_buffer->device_address() + mesh_desc.vertex_offset * 32;
        blas_geometry_desc.triangles.vertex_stride = 32;

        AccelerationStructureBuildInputs blas_build_inputs;
        blas_build_inputs.kind = AccelerationStructureKind::bottom_level;
        blas_build_inputs.flags = AccelerationStructureBuildFlags::none;
        blas_build_inputs.desc_count = 1;
        blas_build_inputs.geometry_descs = &blas_geometry_desc;

        AccelerationStructurePrebuildInfo blas_prebuild_info
            = device->get_acceleration_structure_prebuild_info(blas_build_inputs);

        ref<Buffer> blas_scratch_buffer = device->create_buffer({
            .size = blas_prebuild_info.scratch_data_size,
            .usage = ResourceUsage::unordered_access,
            .debug_name = "blas_scratch_buffer",
        });

        ref<Buffer> blas_buffer = device->create_buffer({
            .size = blas_prebuild_info.result_data_max_size,
            .usage = ResourceUsage::acceleration_structure,
            .debug_name = "blas_buffer",
        });

        ref<AccelerationStructure> blas = device->create_acceleration_structure({
            .kind = AccelerationStructureKind::bottom_level,
            .buffer = blas_buffer,
            .size = blas_buffer->size(),
        });

        ref<CommandBuffer> command_buffer = device->create_command_buffer();
        {
            RayTracingCommandEncoder encoder = command_buffer->encode_ray_tracing_commands();
            encoder.build_acceleration_structure({
                .inputs = blas_build_inputs,
                .dst = blas,
                .scratch_data = blas_scratch_buffer->device_address(),
            });
        }
        command_buffer->submit();

        return blas;
    }

    ref<AccelerationStructure> build_tlas()
    {
        std::vector<RayTracingInstanceDesc> rt_instance_descs;
        rt_instance_descs.reserve(stage.instances.size());
        for (size_t instance_id = 0; instance_id < instance_descs.size(); ++instance_id) {
            const InstanceDesc& instance_desc = instance_descs[instance_id];
            RayTracingInstanceDesc rt_instance_desc;
            rt_instance_desc.transform = float3x4(transforms[instance_desc.transform_id]);
            rt_instance_desc.instance_id = narrow_cast<uint32_t>(instance_id);
            rt_instance_desc.instance_mask = 0xFF;
            rt_instance_desc.instance_contribution_to_hit_group_index = 0;
            rt_instance_desc.set_flags(RayTracingInstanceFlags::none);
            rt_instance_desc.acceleration_structure = blases[instance_desc.mesh_id]->device_address();
            rt_instance_descs.push_back(rt_instance_desc);
        }

        ref<Buffer> rt_instance_buffer = device->create_buffer({
            .usage = ResourceUsage::shader_resource,
            .debug_name = "rt_instance_buffer",
            .data = rt_instance_descs.data(),
            .data_size = rt_instance_descs.size() * sizeof(RayTracingInstanceDesc),
        });

        AccelerationStructureBuildInputs tlas_build_inputs;
        tlas_build_inputs.kind = AccelerationStructureKind::top_level;
        tlas_build_inputs.flags = AccelerationStructureBuildFlags::none;
        tlas_build_inputs.desc_count = uint32_t(rt_instance_descs.size());
        tlas_build_inputs.instance_descs = rt_instance_buffer->device_address();

        AccelerationStructurePrebuildInfo tlas_prebuild_info
            = device->get_acceleration_structure_prebuild_info(tlas_build_inputs);

        ref<Buffer> tlas_scratch_buffer = device->create_buffer({
            .size = tlas_prebuild_info.scratch_data_size,
            .usage = ResourceUsage::unordered_access,
            .debug_name = "tlas_scratch_buffer",
        });

        ref<Buffer> tlas_buffer = device->create_buffer({
            .size = tlas_prebuild_info.result_data_max_size,
            .usage = ResourceUsage::acceleration_structure,
            .debug_name = "tlas_buffer",
        });

        ref<AccelerationStructure> tlas_ = device->create_acceleration_structure({
            .kind = AccelerationStructureKind::top_level,
            .buffer = tlas_buffer,
            .size = tlas_buffer->size(),
        });

        ref<CommandBuffer> command_buffer = device->create_command_buffer();
        {
            RayTracingCommandEncoder encoder = command_buffer->encode_ray_tracing_commands();
            encoder.build_acceleration_structure({
                .inputs = tlas_build_inputs,
                .dst = tlas_,
                .scratch_data = tlas_scratch_buffer->device_address(),
            });
        }
        command_buffer->submit();

        return tlas_;
    }

    void bind(ShaderCursor cursor) const
    {
        cursor["tlas"] = tlas;
        cursor["material_descs"] = material_descs_buffer;
        cursor["mesh_descs"] = mesh_descs_buffer;
        cursor["instance_descs"] = instance_descs_buffer;
        cursor["vertices"] = vertex_buffer;
        cursor["indices"] = index_buffer;
        cursor["transforms"] = transforms_buffer;
        cursor["inverse_transpose_transforms"] = inverse_transpose_transforms_buffer;
        camera.bind(cursor["camera"]);
    }
};

struct PathTracer {
    ref<Device> device;
    const Scene& scene;
    ref<ComputePipeline> pipeline;

    PathTracer(ref<Device> device, const Scene& scene)
        : device(device)
        , scene(scene)
    {
        ref<ShaderProgram> program = device->load_program("pathtracer.slang", {"main"});
        pipeline = device->create_compute_pipeline({.program = program});
    }

    void execute(ref<Texture> output, uint32_t frame)
    {
        ref<CommandBuffer> command_buffer = device->create_command_buffer();
        {
            ComputeCommandEncoder encoder = command_buffer->encode_compute_commands();
            ref<ShaderObject> shader_object = encoder.bind_pipeline(pipeline);
            ShaderCursor cursor = ShaderCursor(shader_object);
            cursor["g_output"] = output;
            cursor["g_frame"] = frame;
            scene.bind(cursor["g_scene"]);
            encoder.dispatch({output->width(), output->height(), 1});
        }
        command_buffer->submit();
    }
};

struct Accumulator {
    ref<Device> device;
    ref<ComputeKernel> kernel;
    ref<Texture> accumulator;

    Accumulator(ref<Device> device)
        : device(device)
    {
        ref<ShaderProgram> program = device->load_program("accumulator.slang", {"main"});
        kernel = device->create_compute_kernel({.program = program});
    }

    void execute(ref<Texture> input, ref<Texture> output, bool reset = false)
    {
        if (!accumulator || accumulator->width() != input->width() || accumulator->height() != input->height()) {
            accumulator = device->create_texture({
                .format = Format::rgba32_float,
                .width = input->width(),
                .height = input->height(),
                .mip_count = 1,
                .usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
                .debug_name = "accumulator",
            });
        }
        kernel->dispatch(
            uint3(input->width(), input->height(), 1),
            [&](ShaderCursor cursor)
            {
                ShaderCursor a = cursor["g_accumulator"];
                a["input"] = input;
                a["output"] = output;
                a["accumulator"] = accumulator;
                a["reset"] = reset;
            }
        );
    }
};

struct ToneMapper {
    ref<Device> device;
    ref<ComputeKernel> kernel;

    ToneMapper(ref<Device> device)
        : device(device)
    {
        ref<ShaderProgram> program = device->load_program("tone_mapper.slang", {"main"});
        kernel = device->create_compute_kernel({.program = program});
    }

    void execute(ref<Texture> input, ref<Texture> output)
    {
        kernel->dispatch(
            uint3(input->width(), input->height(), 1),
            [&](ShaderCursor cursor)
            {
                ShaderCursor t = cursor["g_tone_mapper"];
                t["input"] = input;
                t["output"] = output;
            }
        );
    }
};

struct App {
    ref<Window> window;
    ref<Device> device;
    ref<Swapchain> swapchain;
    ref<Texture> render_texture;
    ref<Texture> accum_texture;
    ref<Texture> output_texture;
    std::unique_ptr<Stage> stage;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<CameraController> camera_controller;
    std::unique_ptr<PathTracer> path_tracer;
    std::unique_ptr<Accumulator> accumulator;
    std::unique_ptr<ToneMapper> tone_mapper;

    App()
    {
        window = Window::create({
            .width = 1920,
            .height = 1080,
            .title = "PathTracer",
            .resizable = true,
        });
        device = Device::create({
            .enable_debug_layers = true,
            .compiler_options = {.include_paths = {EXAMPLE_DIR}},
        });
        swapchain = device->create_swapchain(
            {
                .format = Format::rgba8_unorm,
                .width = window->width(),
                .height = window->height(),
                .enable_vsync = false,
            },
            window
        );

        window->set_on_keyboard_event([this](const KeyboardEvent& event) { on_keyboard_event(event); });
        window->set_on_mouse_event([this](const MouseEvent& event) { on_mouse_event(event); });
        window->set_on_resize([this](uint32_t width, uint32_t height) { on_resize(width, height); });

        stage = Stage::demo();
        scene = std::make_unique<Scene>(device, *stage);

        camera_controller = std::make_unique<CameraController>(stage->camera);

        path_tracer = std::make_unique<PathTracer>(device, *scene);
        accumulator = std::make_unique<Accumulator>(device);
        tone_mapper = std::make_unique<ToneMapper>(device);
    }

    void on_keyboard_event(const KeyboardEvent& event)
    {
        if (event.type == KeyboardEventType::key_press) {
            if (event.key == KeyCode::escape) {
                window->close();
            } else if (event.key == KeyCode::f1) {
                if (output_texture)
                    utils::show_in_tev_async(output_texture);
            } else if (event.key == KeyCode::f2) {
                if (output_texture) {
                    ref<Bitmap> bitmap = output_texture->to_bitmap();
                    bitmap->convert(Bitmap::PixelFormat::rgb, Bitmap::ComponentType::uint8, true)
                        ->write_async("screenshot.png");
                }
            }
        }
        camera_controller->on_keyboard_event(event);
    }

    void on_mouse_event(const MouseEvent& event) { camera_controller->on_mouse_event(event); }

    void on_resize(uint32_t width, uint32_t height)
    {
        device->wait();
        swapchain->resize(width, height);
    }

    void main_loop()
    {
        uint32_t frame = 0;
        Timer timer;
        while (!window->should_close()) {
            float dt = float(timer.elapsed_s());
            timer.reset();

            window->process_events();

            if (camera_controller->update(dt))
                frame = 0;

            int index = swapchain->acquire_next_image();
            if (index < 0)
                continue;

            ref<Texture> image = swapchain->get_image(index);
            if (!output_texture || output_texture->width() != image->width()
                || output_texture->height() != image->height()) {
                output_texture = device->create_texture({
                    .format = Format::rgba8_unorm,
                    .width = image->width(),
                    .height = image->height(),
                    .mip_count = 1,
                    .usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
                    .debug_name = "output_texture",
                });
                render_texture = device->create_texture({
                    .format = Format::rgba32_float,
                    .width = image->width(),
                    .height = image->height(),
                    .mip_count = 1,
                    .usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
                    .debug_name = "render_texture",
                });
                accum_texture = device->create_texture({
                    .format = Format::rgba32_float,
                    .width = image->width(),
                    .height = image->height(),
                    .mip_count = 1,
                    .usage = ResourceUsage::shader_resource | ResourceUsage::unordered_access,
                    .debug_name = "accum_texture",
                });
            }

            stage->camera.width = image->width();
            stage->camera.height = image->height();
            stage->camera.recompute();

            path_tracer->execute(render_texture, frame);
            accumulator->execute(render_texture, accum_texture, frame == 0);
            tone_mapper->execute(accum_texture, output_texture);

            ref<CommandBuffer> command_buffer = device->create_command_buffer();
            command_buffer->copy_resource(image, output_texture);
            command_buffer->texture_barrier(image, ResourceState::present);
            command_buffer->submit();
            image.reset();

            swapchain->present();

            device->end_frame();

            frame++;
        }

        device->wait();
    }
};

int main()
{
    kali::static_init();

    {
        App app;
        app.main_loop();
    }

    kali::static_shutdown();
}
