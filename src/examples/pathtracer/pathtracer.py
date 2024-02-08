import kali
import numpy as np
from pathlib import Path
from dataclasses import dataclass
import struct

EXAMPLE_DIR = Path(__file__).parent


class Camera:
    def __init__(self):
        self.width = 100
        self.height = 100
        self.aspect_ratio = 1.0
        self.position = kali.float3(1, 1, 1)
        self.target = kali.float3(0, 0, 0)
        self.up = kali.float3(0, 1, 0)
        self.fov = 70.0
        self.recompute()

    def recompute(self):
        self.aspect_ratio = float(self.width) / float(self.height)

        self.fwd = kali.math.normalize(self.target - self.position)
        self.right = kali.math.normalize(kali.math.cross(self.fwd, self.up))
        self.up = kali.math.normalize(kali.math.cross(self.right, self.fwd))

        fov = kali.math.radians(self.fov)

        self.image_u = self.right * kali.math.tan(fov * 0.5) * self.aspect_ratio
        self.image_v = self.up * kali.math.tan(fov * 0.5)
        self.image_w = self.fwd

    def bind(self, cursor: kali.ShaderCursor):
        cursor["position"] = self.position
        cursor["image_u"] = self.image_u
        cursor["image_v"] = self.image_v
        cursor["image_w"] = self.image_w


class CameraController:
    MOVE_KEYS = {
        kali.KeyCode.a: kali.float3(-1, 0, 0),
        kali.KeyCode.d: kali.float3(1, 0, 0),
        kali.KeyCode.e: kali.float3(0, 1, 0),
        kali.KeyCode.q: kali.float3(0, -1, 0),
        kali.KeyCode.w: kali.float3(0, 0, 1),
        kali.KeyCode.s: kali.float3(0, 0, -1),
    }
    MOVE_SHIFT_FACTOR = 10.0

    def __init__(self, camera: Camera):
        self.camera = camera
        self.mouse_down = False
        self.mouse_pos = kali.float2()
        self.key_state = {k: False for k in CameraController.MOVE_KEYS.keys()}
        self.shift_down = False

        self.move_delta = kali.float3()
        self.rotate_delta = kali.float2()

        self.move_speed = 1.0
        self.rotate_speed = 0.002

    def update(self, dt: float):
        changed = False
        position = self.camera.position
        fwd = self.camera.fwd
        up = self.camera.up
        right = self.camera.right

        # Move
        if kali.math.length(self.move_delta) > 0:
            offset = right * self.move_delta.x
            offset += up * self.move_delta.y
            offset += fwd * self.move_delta.z
            factor = CameraController.MOVE_SHIFT_FACTOR if self.shift_down else 1.0
            offset *= self.move_speed * factor * dt
            position += offset
            changed = True

        # Rotate
        if kali.math.length(self.rotate_delta) > 0:
            yaw = kali.math.atan2(fwd.z, fwd.x)
            pitch = kali.math.asin(fwd.y)
            yaw += self.rotate_speed * self.rotate_delta.x
            pitch -= self.rotate_speed * self.rotate_delta.y
            fwd = kali.float3(
                kali.math.cos(yaw) * kali.math.cos(pitch),
                kali.math.sin(pitch),
                kali.math.sin(yaw) * kali.math.cos(pitch),
            )
            self.rotate_delta = kali.float2()
            changed = True

        if changed:
            self.camera.position = position
            self.camera.target = position + fwd
            self.camera.up = kali.float3(0, 1, 0)
            self.camera.recompute()

        return changed

    def on_keyboard_event(self, event: kali.KeyboardEvent):
        if event.is_key_press() or event.is_key_release():
            down = event.is_key_press()
            if event.key in CameraController.MOVE_KEYS:
                self.key_state[event.key] = down
            elif event.key == kali.KeyCode.left_shift:
                self.shift_down = down
        self.move_delta = kali.float3()
        for key, state in self.key_state.items():
            if state:
                self.move_delta += CameraController.MOVE_KEYS[key]

    def on_mouse_event(self, event: kali.MouseEvent):
        self.rotate_delta = kali.float2()
        if event.is_button_down() and event.button == kali.MouseButton.left:
            self.mouse_down = True
        if event.is_button_up() and event.button == kali.MouseButton.left:
            self.mouse_down = False
        if event.is_move():
            mouse_delta = event.pos - self.mouse_pos
            if self.mouse_down:
                self.rotate_delta = mouse_delta
            self.mouse_pos = event.pos


class Material:
    def __init__(self, base_color=kali.float3(0.5)):
        self.base_color = base_color


class Mesh:
    def __init__(self, vertices, indices):
        assert vertices.ndim == 2 and vertices.dtype == np.float32
        assert indices.ndim == 2 and indices.dtype == np.uint32
        self.vertices = vertices
        self.indices = indices

    @property
    def vertex_count(self):
        return self.vertices.shape[0]

    @property
    def triangle_count(self):
        return self.indices.shape[0]

    @property
    def index_count(self):
        return self.triangle_count * 3

    @classmethod
    def create_quad(cls, size=kali.float2(1)):
        vertices = np.array(
            [
                # position, normal, uv
                [-0.5, 0, -0.5, 0, 1, 0, 0, 0],
                [+0.5, 0, -0.5, 0, 1, 0, 1, 0],
                [-0.5, 0, +0.5, 0, 1, 0, 0, 1],
                [+0.5, 0, +0.5, 0, 1, 0, 1, 1],
            ],
            dtype=np.float32,
        )
        vertices[:, (0, 2)] *= [size[0], size[1]]
        indices = np.array(
            [
                [2, 1, 0],
                [1, 2, 3],
            ],
            dtype=np.uint32,
        )
        return Mesh(vertices, indices)

    @classmethod
    def create_cube(cls, size=kali.float3(1)):
        vertices = np.array(
            [
                # position, normal, uv
                # left
                [-0.5, -0.5, -0.5, 0, -1, 0, 0.0, 0.0],
                [-0.5, -0.5, +0.5, 0, -1, 0, 1.0, 0.0],
                [+0.5, -0.5, +0.5, 0, -1, 0, 1.0, 1.0],
                [+0.5, -0.5, -0.5, 0, -1, 0, 0.0, 1.0],
                # right
                [-0.5, +0.5, +0.5, 0, +1, 0, 0.0, 0.0],
                [-0.5, +0.5, -0.5, 0, +1, 0, 1.0, 0.0],
                [+0.5, +0.5, -0.5, 0, +1, 0, 1.0, 1.0],
                [+0.5, +0.5, +0.5, 0, +1, 0, 0.0, 1.0],
                # back
                [-0.5, +0.5, -0.5, 0, 0, -1, 0.0, 0.0],
                [-0.5, -0.5, -0.5, 0, 0, -1, 1.0, 0.0],
                [+0.5, -0.5, -0.5, 0, 0, -1, 1.0, 1.0],
                [+0.5, +0.5, -0.5, 0, 0, -1, 0.0, 1.0],
                # front
                [+0.5, +0.5, +0.5, 0, 0, +1, 0.0, 0.0],
                [+0.5, -0.5, +0.5, 0, 0, +1, 1.0, 0.0],
                [-0.5, -0.5, +0.5, 0, 0, +1, 1.0, 1.0],
                [-0.5, +0.5, +0.5, 0, 0, +1, 0.0, 1.0],
                # bottom
                [-0.5, +0.5, +0.5, -1, 0, 0, 0.0, 0.0],
                [-0.5, -0.5, +0.5, -1, 0, 0, 1.0, 0.0],
                [-0.5, -0.5, -0.5, -1, 0, 0, 1.0, 1.0],
                [-0.5, +0.5, -0.5, -1, 0, 0, 0.0, 1.0],
                # top
                [+0.5, +0.5, -0.5, +1, 0, 0, 0.0, 0.0],
                [+0.5, -0.5, -0.5, +1, 0, 0, 1.0, 0.0],
                [+0.5, -0.5, +0.5, +1, 0, 0, 1.0, 1.0],
                [+0.5, +0.5, +0.5, +1, 0, 0, 0.0, 1.0],
            ],
            dtype=np.float32,
        )
        vertices[:, 0:3] *= [size[0], size[1], size[2]]

        indices = np.array(
            [
                [0, 2, 1],
                [0, 3, 2],
                [4, 6, 5],
                [4, 7, 6],
                [8, 10, 9],
                [8, 11, 10],
                [12, 14, 13],
                [12, 15, 14],
                [16, 18, 17],
                [16, 19, 18],
                [20, 22, 21],
                [20, 23, 22],
            ],
            dtype=np.uint32,
        )

        return Mesh(vertices, indices)


class Transform:
    def __init__(self):
        self.translation = kali.float3(0)
        self.scaling = kali.float3(1)
        self.rotation = kali.float3(0)
        self.matrix = kali.float4x4.identity()

    def update_matrix(self):
        T = kali.math.matrix_from_translation(self.translation)
        S = kali.math.matrix_from_scaling(self.scaling)
        R = kali.math.matrix_from_rotation_xyz(self.rotation)
        self.matrix = kali.math.mul(kali.math.mul(T, R), S)


class Stage:
    def __init__(self):
        self.camera = Camera()
        self.materials = []
        self.meshes = []
        self.transforms = []
        self.instances = []

    def add_material(self, material: Material):
        material_id = len(self.materials)
        self.materials.append(material)
        return material_id

    def add_mesh(self, mesh: Mesh):
        mesh_id = len(self.meshes)
        self.meshes.append(mesh)
        return mesh_id

    def add_transform(self, transform: Transform):
        transform_id = len(self.transforms)
        self.transforms.append(transform)
        return transform_id

    def add_instance(self, mesh_id: int, material_id: int, transform_id: int):
        instance_id = len(self.instances)
        self.instances.append((mesh_id, material_id, transform_id))
        return instance_id

    @classmethod
    def demo(cls):
        stage = Stage()
        stage.camera.target = kali.float3(0, 1, 0)
        stage.camera.position = kali.float3(2, 1, 2)

        floor_material = stage.add_material(Material(base_color=kali.float3(0.5)))
        floor_mesh = stage.add_mesh(Mesh.create_quad([5, 5]))
        floor_transform = stage.add_transform(Transform())
        stage.add_instance(floor_mesh, floor_material, floor_transform)

        cube_materials = []
        for _ in range(10):
            cube_materials.append(
                stage.add_material(
                    Material(base_color=np.random.rand(3).astype(np.float32))
                )
            )
        cube_mesh = stage.add_mesh(Mesh.create_cube([0.1, 0.1, 0.1]))

        for i in range(1000):
            transform = Transform()
            transform.translation = (np.random.rand(3) * 2 - 1).astype(np.float32)
            transform.translation[1] += 1
            transform.scaling = (np.random.rand(3) + 0.5).astype(np.float32)
            transform.rotation = (np.random.rand(3) * 10).astype(np.float32)
            transform.update_matrix()
            cube_transform = stage.add_transform(transform)
            stage.add_instance(
                cube_mesh, cube_materials[i % len(cube_materials)], cube_transform
            )

        return stage


class Scene:
    @dataclass
    class MaterialDesc:
        base_color: kali.float3

        def pack(self):
            return struct.pack(
                "fff", self.base_color[0], self.base_color[1], self.base_color[2]
            )

    @dataclass
    class MeshDesc:
        vertex_count: int
        index_count: int
        vertex_offset: int
        index_offset: int

        def pack(self):
            return struct.pack(
                "IIII",
                self.vertex_count,
                self.index_count,
                self.vertex_offset,
                self.index_offset,
            )

    @dataclass
    class InstanceDesc:
        mesh_id: int
        material_id: int
        transform_id: int

        def pack(self):
            return struct.pack("III", self.mesh_id, self.material_id, self.transform_id)

    def __init__(self, device: kali.Device, stage: Stage):
        self.device = device

        self.camera = stage.camera

        # Prepare material descriptors
        self.material_descs = [
            Scene.MaterialDesc(base_color=m.base_color) for m in stage.materials
        ]
        material_descs_data = np.frombuffer(
            b"".join(d.pack() for d in self.material_descs), dtype=np.uint8
        ).flatten()
        self.material_descs_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="material_descs_buffer",
            data=material_descs_data,
        )

        # Prepare mesh descriptors
        vertex_count = 0
        index_count = 0
        self.mesh_descs = []
        for mesh in stage.meshes:
            self.mesh_descs.append(
                Scene.MeshDesc(
                    vertex_count=mesh.vertex_count,
                    index_count=mesh.index_count,
                    vertex_offset=vertex_count,
                    index_offset=index_count,
                )
            )
            vertex_count += mesh.vertex_count
            index_count += mesh.index_count

        # Prepare instance descriptors
        self.instance_descs = []
        for mesh_id, material_id, transform_id in stage.instances:
            self.instance_descs.append(
                Scene.InstanceDesc(mesh_id, material_id, transform_id)
            )

        # Create vertex and index buffers
        vertices = np.concatenate([mesh.vertices for mesh in stage.meshes], axis=0)
        indices = np.concatenate([mesh.indices for mesh in stage.meshes], axis=0)
        assert vertices.shape[0] == vertex_count
        assert indices.shape[0] == index_count // 3

        self.vertex_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="vertex_buffer",
            data=vertices,
        )

        self.index_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="index_buffer",
            data=indices,
        )

        mesh_descs_data = np.frombuffer(
            b"".join(d.pack() for d in self.mesh_descs), dtype=np.uint8
        ).flatten()
        self.mesh_descs_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="mesh_descs_buffer",
            data=mesh_descs_data,
        )

        instance_descs_data = np.frombuffer(
            b"".join(d.pack() for d in self.instance_descs), dtype=np.uint8
        ).flatten()
        self.instance_descs_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="instance_descs_buffer",
            data=instance_descs_data,
        )

        # Prepare transforms
        self.transforms = [t.matrix for t in stage.transforms]
        self.inverse_transpose_transforms = [
            kali.math.transpose(kali.math.inverse(t)) for t in self.transforms
        ]
        self.transform_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="transform_buffer",
            data=np.stack([t.to_numpy() for t in self.transforms]),
        )
        self.inverse_transpose_transforms_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="inverse_transpose_transforms_buffer",
            data=np.stack([t.to_numpy() for t in self.inverse_transpose_transforms]),
        )

        self.identity_buffer = device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="identity_buffer",
            data=kali.float3x4.identity().to_numpy(),
        )

        # Build BLASes
        self.blases = [self.build_blas(mesh_desc) for mesh_desc in self.mesh_descs]

        # Build TLAS
        self.tlas = self.build_tlas()

    def build_blas(self, mesh_desc: MeshDesc):
        blas_geometry_desc = kali.RayTracingGeometryDesc()
        blas_geometry_desc.type = kali.RayTracingGeometryType.triangles
        blas_geometry_desc.flags = kali.RayTracingGeometryFlags.opaque
        blas_geometry_desc.triangles.transform3x4 = self.identity_buffer.device_address
        blas_geometry_desc.triangles.index_format = kali.Format.r32_uint
        blas_geometry_desc.triangles.vertex_format = kali.Format.rgb32_float
        blas_geometry_desc.triangles.index_count = mesh_desc.index_count
        blas_geometry_desc.triangles.vertex_count = mesh_desc.vertex_count
        blas_geometry_desc.triangles.index_data = (
            self.index_buffer.device_address + mesh_desc.index_offset * 4
        )
        blas_geometry_desc.triangles.vertex_data = (
            self.vertex_buffer.device_address + mesh_desc.vertex_offset * 32
        )
        blas_geometry_desc.triangles.vertex_stride = 32

        blas_build_inputs = kali.AccelerationStructureBuildInputs()
        blas_build_inputs.kind = kali.AccelerationStructureKind.bottom_level
        blas_build_inputs.flags = kali.AccelerationStructureBuildFlags.none
        blas_build_inputs.geometry_descs = [blas_geometry_desc]

        blas_prebuild_info = self.device.get_acceleration_structure_prebuild_info(
            blas_build_inputs
        )

        blas_scratch_buffer = self.device.create_buffer(
            size=blas_prebuild_info.scratch_data_size,
            usage=kali.ResourceUsage.unordered_access,
            debug_name="blas_scratch_buffer",
        )

        blas_buffer = self.device.create_buffer(
            size=blas_prebuild_info.result_data_max_size,
            usage=kali.ResourceUsage.acceleration_structure,
            debug_name="blas_buffer",
        )

        blas = self.device.create_acceleration_structure(
            kind=kali.AccelerationStructureKind.bottom_level,
            buffer=blas_buffer,
            size=blas_buffer.size,
        )

        command_buffer = self.device.create_command_buffer()
        with command_buffer.encode_ray_tracing_commands() as encoder:
            encoder.build_acceleration_structure(
                inputs=blas_build_inputs,
                dst=blas,
                scratch_data=blas_scratch_buffer.device_address,
            )
        command_buffer.submit()

        return blas

    def build_tlas(self):
        rt_instance_descs = []
        for instance_id, instance_desc in enumerate(self.instance_descs):
            rt_instance_desc = kali.RayTracingInstanceDesc()
            rt_instance_desc.transform = kali.float3x4(
                self.transforms[instance_desc.transform_id]
            )
            rt_instance_desc.instance_id = instance_id
            rt_instance_desc.instance_mask = 0xFF
            rt_instance_desc.instance_contribution_to_hit_group_index = 0
            rt_instance_desc.flags = kali.RayTracingInstanceFlags.none
            rt_instance_desc.acceleration_structure = self.blases[
                instance_desc.mesh_id
            ].device_address
            rt_instance_descs.append(rt_instance_desc)

        rt_instance_buffer = self.device.create_buffer(
            usage=kali.ResourceUsage.shader_resource,
            debug_name="rt_instance_buffer",
            data=np.stack([i.to_numpy() for i in rt_instance_descs]),
        )

        tlas_build_inputs = kali.AccelerationStructureBuildInputs()
        tlas_build_inputs.kind = kali.AccelerationStructureKind.top_level
        tlas_build_inputs.flags = kali.AccelerationStructureBuildFlags.none
        tlas_build_inputs.desc_count = len(rt_instance_descs)
        tlas_build_inputs.instance_descs = rt_instance_buffer.device_address

        tlas_prebuild_info = self.device.get_acceleration_structure_prebuild_info(
            tlas_build_inputs
        )

        tlas_scratch_buffer = self.device.create_buffer(
            size=tlas_prebuild_info.scratch_data_size,
            usage=kali.ResourceUsage.unordered_access,
            debug_name="tlas_scratch_buffer",
        )

        tlas_buffer = self.device.create_buffer(
            size=tlas_prebuild_info.result_data_max_size,
            usage=kali.ResourceUsage.acceleration_structure,
            debug_name="tlas_buffer",
        )

        tlas = self.device.create_acceleration_structure(
            kind=kali.AccelerationStructureKind.top_level,
            buffer=tlas_buffer,
            size=tlas_buffer.size,
        )

        command_buffer = self.device.create_command_buffer()
        with command_buffer.encode_ray_tracing_commands() as encoder:
            encoder.build_acceleration_structure(
                inputs=tlas_build_inputs,
                dst=tlas,
                scratch_data=tlas_scratch_buffer.device_address,
            )
        command_buffer.submit()

        return tlas

    def bind(self, cursor: kali.ShaderCursor):
        cursor["tlas"] = self.tlas
        cursor["material_descs"] = self.material_descs_buffer
        cursor["mesh_descs"] = self.mesh_descs_buffer
        cursor["instance_descs"] = self.instance_descs_buffer
        cursor["vertices"] = self.vertex_buffer
        cursor["indices"] = self.index_buffer
        cursor["transforms"] = self.transform_buffer
        cursor[
            "inverse_transpose_transforms"
        ] = self.inverse_transpose_transforms_buffer
        self.camera.bind(cursor["camera"])


class PathTracer:
    def __init__(self, device: kali.Device, scene: Scene):
        self.device = device
        self.scene = scene

        self.program = self.device.load_module(
            EXAMPLE_DIR / "pathtracer.slang"
        ).create_program("main")

        self.pipeline = self.device.create_compute_pipeline(self.program)

    def execute(self, output: kali.Texture, frame: int):
        w = output.width
        h = output.height

        self.scene.camera.width = w
        self.scene.camera.height = h
        self.scene.camera.recompute()

        command_buffer = self.device.create_command_buffer()
        with command_buffer.encode_compute_commands() as encoder:
            shader_object = encoder.bind_pipeline(self.pipeline)
            cursor = kali.ShaderCursor(shader_object)
            cursor.g_output = output
            cursor.g_frame = frame
            self.scene.bind(cursor.g_scene)
            encoder.dispatch(thread_count=[w, h, 1])
        command_buffer.submit()


class Accumulator:
    def __init__(self, device: kali.Device):
        self.device = device
        self.kernel = self.device.load_module(
            EXAMPLE_DIR / "accumulator.slang"
        ).create_compute_kernel("main")
        self.accumulator: kali.Texture = None

    def execute(self, input: kali.Texture, output: kali.Texture, reset=False):
        if (
            self.accumulator == None
            or self.accumulator.width != input.width
            or self.accumulator.height != input.height
        ):
            self.accumulator = self.device.create_texture(
                format=kali.Format.rgba32_float,
                width=input.width,
                height=input.height,
                mip_count=1,
                usage=kali.ResourceUsage.shader_resource
                | kali.ResourceUsage.unordered_access,
                debug_name="accumulator",
            )
        self.kernel.dispatch(
            thread_count=[input.width, input.height, 1],
            vars={
                "g_accumulator": {
                    "input": input,
                    "output": output,
                    "accumulator": self.accumulator,
                    "reset": reset,
                }
            },
        )


class ToneMapper:
    def __init__(self, device: kali.Device):
        self.device = device
        self.kernel = self.device.load_module(
            EXAMPLE_DIR / "tone_mapper.slang"
        ).create_compute_kernel("main")

    def execute(self, input: kali.Texture, output: kali.Texture):
        self.kernel.dispatch(
            thread_count=[input.width, input.height, 1],
            vars={
                "g_tone_mapper": {
                    "input": input,
                    "output": output,
                }
            },
        )


class App:
    def __init__(self):
        self.window = kali.Window(
            width=1920, height=1080, title="PathTracer", resizable=True
        )
        self.device = kali.Device(enable_debug_layers=False)
        self.swapchain = self.device.create_swapchain(
            format=kali.Format.rgba8_unorm,
            width=self.window.width,
            height=self.window.height,
            window=self.window,
            enable_vsync=False,
        )

        self.render_texture: kali.Texture = None
        self.accum_texture: kali.Texture = None
        self.output_texture: kali.Texture = None

        self.window.on_keyboard_event = self.on_keyboard_event
        self.window.on_mouse_event = self.on_mouse_event
        self.window.on_resize = self.on_resize

        self.stage = Stage.demo()
        self.scene = Scene(self.device, self.stage)

        self.camera_controller = CameraController(self.stage.camera)

        self.path_tracer = PathTracer(self.device, self.scene)
        self.accumulator = Accumulator(self.device)
        self.tone_mapper = ToneMapper(self.device)

    def on_keyboard_event(self, event: kali.KeyboardEvent):
        if event.type == kali.KeyboardEventType.key_press:
            if event.key == kali.KeyCode.escape:
                self.window.close()
            elif event.key == kali.KeyCode.f1:
                if self.output_texture:
                    kali.utils.show_in_tev_async(self.output_texture)
            elif event.key == kali.KeyCode.f2:
                if self.output_texture:
                    bitmap = self.output_texture.to_bitmap()
                    bitmap.convert(
                        kali.Bitmap.PixelFormat.rgb,
                        kali.Bitmap.ComponentType.uint8,
                        srgb_gamma=True,
                    ).write_async("screenshot.png")

        self.camera_controller.on_keyboard_event(event)

    def on_mouse_event(self, event: kali.MouseEvent):
        self.camera_controller.on_mouse_event(event)

    def on_resize(self, width, height):
        self.device.wait()
        self.swapchain.resize(width, height)

    def main_loop(self):
        frame = 0
        timer = kali.Timer()
        while not self.window.should_close():
            dt = timer.elapsed_s()
            timer.reset()

            self.window.process_events()

            if self.camera_controller.update(dt):
                frame = 0

            index = self.swapchain.acquire_next_image()
            if index < 0:
                continue

            image = self.swapchain.get_image(index)
            if (
                self.output_texture == None
                or self.output_texture.width != image.width
                or self.output_texture.height != image.height
            ):
                self.output_texture = self.device.create_texture(
                    format=kali.Format.rgba8_unorm,
                    width=image.width,
                    height=image.height,
                    mip_count=1,
                    usage=kali.ResourceUsage.shader_resource
                    | kali.ResourceUsage.unordered_access,
                    debug_name="output_texture",
                )
                self.render_texture = self.device.create_texture(
                    format=kali.Format.rgba32_float,
                    width=image.width,
                    height=image.height,
                    mip_count=1,
                    usage=kali.ResourceUsage.shader_resource
                    | kali.ResourceUsage.unordered_access,
                    debug_name="render_texture",
                )
                self.accum_texture = self.device.create_texture(
                    format=kali.Format.rgba32_float,
                    width=image.width,
                    height=image.height,
                    mip_count=1,
                    usage=kali.ResourceUsage.shader_resource
                    | kali.ResourceUsage.unordered_access,
                    debug_name="accum_texture",
                )

            self.path_tracer.execute(self.render_texture, frame)
            self.accumulator.execute(
                self.render_texture, self.accum_texture, frame == 0
            )
            self.tone_mapper.execute(self.accum_texture, self.output_texture)

            command_buffer = self.device.create_command_buffer()
            command_buffer.copy_resource(dst=image, src=self.output_texture)
            command_buffer.texture_barrier(image, kali.ResourceState.present)
            command_buffer.submit()
            del image

            self.swapchain.present()

            self.device.end_frame()

            frame += 1


app = App()
app.main_loop()
