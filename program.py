


# ShaderModule


# Program
#     - shader modules
#         - source files

"""
Program
=======

A gfx shader program is just a composite containing the global scope and entrypoints.

class IShaderProgram
{
    struct Desc
    {
        // TODO: Tess doesn't like this but doesn't know what to do about it
        // The linking style of this program.
        LinkingStyle linkingStyle = LinkingStyle::SingleProgram;

        // The global scope or a Slang composite component that represents the entire program.
        slang::IComponentType*  slangGlobalScope;

        // Number of separate entry point components in the `slangEntryPoints` array to link in.
        // If set to 0, then `slangGlobalScope` must contain Slang EntryPoint components.
        // If not 0, then `slangGlobalScope` must not contain any EntryPoint components.
        GfxCount entryPointCount = 0;

        // An array of Slang entry points. The size of the array must be `entryPointCount`.
        // Each element must define only 1 Slang EntryPoint.
        slang::IComponentType** slangEntryPoints = nullptr;
    };
};

ComputePipeline
===============

To create a compute pipeline we only need the program.

struct ComputePipelineStateDesc
{
    IShaderProgram*  program = nullptr;
    void* d3d12RootSignatureOverride = nullptr;
};

GraphicsPipeline
================

To create a graphics pipeline we need additional data such as the input layout, framebuffer layout, primitive type, depth stencil, rasterizer and blend.

struct GraphicsPipelineStateDesc
{
    IShaderProgram*      program = nullptr;

    IInputLayout*       inputLayout = nullptr;
    IFramebufferLayout* framebufferLayout = nullptr;
    PrimitiveType       primitiveType = PrimitiveType::Triangle;
    DepthStencilDesc    depthStencil;
    RasterizerDesc      rasterizer;
    BlendDesc           blend;
};

RaytracingPipeline
==================

struct RayTracingPipelineStateDesc
{
    IShaderProgram* program = nullptr;
    GfxCount hitGroupCount = 0;
    const HitGroupDesc* hitGroups = nullptr;
    int maxRecursion = 0;
    Size maxRayPayloadSize = 0;
    Size maxAttributeSizeInBytes = 8;
    RayTracingPipelineFlags::Enum flags = RayTracingPipelineFlags::None;
};
"""

# Loading shader modules

# Load module from a single file
shader_module = device.load_module(path="shader.slang")
# Load module from multiple files
shader_module = device.load_module(paths=["shader.slang", "shader2.slang"])
# Load module from source string
shader_module = device.load_module(source="...")
# Load module from submodules
# shader_module = device.load_module(submodules=[shader_module, shader_module2])

# During module load we can also specify compilation flags and defines
shader_module = device.load_module(path="shader.slang", defines={"DEFINE":1}, flags=SlangCompilerFlags.xxx)

# Creating programs

# Create program from single entrypoint
compute_program = Program(entry_point=shader_module.entry_point("main"))
# Create program from multiple entrypoints
graphics_program = Program(entry_points=[shader_module.entry_point("vertex"), shader_module.entry_point("fragment")])



# Loading programs directly
compute_program = device.load_program(path="shader.slang", entry_point="main")
graphics_program = device.load_program(paths=["vertex.slang", "fragment.slang"], entry_points=["vertex", "fragment"])



# Compute

# Load the program
program = device.create_program(path="compute.slang", defines={})
# or
program = Program(device=device, path="compute.slang", defines={})
# Short form to launch compute kernel
program.entrypoint("main").pipeline().dispatch(threads=[32,32], var1=1, var2=2, globals={"var1":1,"var2":2})
# Short form with syntactic sugar
program.main.dispatch(threads=[32,32], ...)

# Create pipeline explicitly
pipeline = ComputePipeline(entry_point=program.entrypoint("main"))

# Specializing entrypoint
program.entrypoint("main").specialize(constants={}, type_conformances={}).rename("main2")



# Graphics
program = device.create_program(path="graphics.slang", defines={})
pipeline = GraphicsPipeline(vertex=program.entrypoint("vertex"), fragment=program.entrypoint("fragment"), ...)
pipeline.draw(vertex_count=3, instance_count=1, first_vertex=0, first_instance=0)

# Raytracing
program = device.create_program(path="raytracing.slang", defines={})
pipeline = RaytracingPipeline(ray_generation=program.entrypoint("ray_generation"), miss=program.entrypoint("miss"), ...)
pipeline.dispatch(ray_count=1, width=1, height=1, depth=1)


raygen = program.entrypoint("raygen")
miss = program.entrypoint("miss")
hitgroup = program.hitgroup(name="name", closest_hit="closest_hit", any_hit="any_hit", intersection="intersection")

shader_table = ShaderTable(device, raygen_entrypoints=[raygen], miss_entrypoints=[miss], hitgroups=[hitgroup])
pipeline = RaytracingPipeline(shader_table=shader_table)
