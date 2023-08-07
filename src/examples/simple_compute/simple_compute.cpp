#include "kali/core/logger.h"
#include "kali/core/platform.h"
#include "kali/rhi/device.h"
#include "kali/rhi/program.h"
#include "kali/math/vector.h"

#include "shader-cursor.h"

using namespace kali;


Slang::ComPtr<slang::ISession> createSlangSession(gfx::IDevice* device)
{
    Slang::ComPtr<slang::ISession> slangSession = device->getSlangSession();
    return slangSession;
}

Slang::ComPtr<slang::IModule>
compileShaderModuleFromFile(slang::ISession* slangSession, const std::filesystem::path& path)
{
    SlangCompileRequest* slangRequest = nullptr;
    slangSession->createCompileRequest(&slangRequest);

    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, path.string().c_str());
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, path.string().c_str());

    const SlangResult compileRes = spCompile(slangRequest);
    if (auto diagnostics = spGetDiagnosticOutput(slangRequest)) {
        printf("%s", diagnostics);
    }

    if (SLANG_FAILED(compileRes)) {
        spDestroyCompileRequest(slangRequest);
        return Slang::ComPtr<slang::IModule>();
    }

    Slang::ComPtr<slang::IModule> slangModule;
    spCompileRequest_getModule(slangRequest, translationUnitIndex, slangModule.writeRef());
    return slangModule;
}

struct ExampleProgram {
    ref<Device> device;

    Slang::ComPtr<slang::ISession> gSlangSession;
    Slang::ComPtr<slang::IModule> gSlangModule;
    Slang::ComPtr<gfx::IShaderProgram> gProgram;

    Slang::ComPtr<gfx::IShaderProgram> loadComputeProgram(slang::IModule* slangModule, char const* entryPointName)
    {
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        slangModule->findEntryPointByName(entryPointName, entryPoint.writeRef());

        Slang::ComPtr<slang::IComponentType> linkedProgram;
        Slang::ComPtr<slang::IBlob> diagnostics;
        entryPoint->link(linkedProgram.writeRef(), diagnostics.writeRef());
        if (diagnostics) {
            printf("%s", (char const*)diagnostics->getBufferPointer());
        }

        // linkedProgram->getLayout();
        // gGPUPrinting.loadStrings(linkedProgram->getLayout());

        gfx::IShaderProgram::Desc programDesc = {};
        programDesc.slangGlobalScope = linkedProgram;

        auto shaderProgram = device->get_gfx_device()->createProgram(programDesc);

        return shaderProgram;
    }

    Slang::Result execute()
    {
        device = Device::create({.type = DeviceType::automatic, .enable_debug_layers = true});

        // device->create_program(ProgramDesc::create_compute(
        //     get_project_directory() / "src/examples/simple_compute/compute.cs.slang",
        //     "main"
        // ));

        gSlangSession = createSlangSession(device->get_gfx_device());
        gSlangModule = compileShaderModuleFromFile(
            gSlangSession,
            get_project_directory() / "src/examples/simple_compute/compute.cs.slang"
        );

        gProgram = loadComputeProgram(gSlangModule, "main");
        if (!gProgram)
            return SLANG_FAIL;

        gfx::ComputePipelineStateDesc desc;
        desc.program = gProgram;
        auto pipelineState = device->get_gfx_device()->createComputePipelineState(desc);
        if (!pipelineState)
            return SLANG_FAIL;

        ref<Buffer> buffer = device->create_structured_buffer<uint32_t>(1024, ResourceUsage::unordered_access, MemoryType::device_local);

        gfx::ITransientResourceHeap::Desc transientResourceHeapDesc = {};
        transientResourceHeapDesc.constantBufferSize = 256;
        auto transientHeap = device->get_gfx_device()->createTransientResourceHeap(transientResourceHeapDesc);

        auto queue = device->get_gfx_queue();

        auto commandBuffer = transientHeap->createCommandBuffer();
        auto encoder = commandBuffer->encodeComputeCommands();
        auto rootShaderObject = encoder->bindPipeline(pipelineState);
        auto cursor = gfx::ShaderCursor(rootShaderObject);
        cursor["g_buffer"].setResource(buffer->get_uav(0, 1024)->get_gfx_resource_view());
        encoder->dispatchCompute(1024 / 32, 1, 1);
        encoder->bufferBarrier(
            buffer->get_gfx_buffer_resource(),
            gfx::ResourceState::UnorderedAccess,
            gfx::ResourceState::CopySource
        );
        encoder->endEncoding();
        commandBuffer->close();
        queue->executeCommandBuffer(commandBuffer);

        queue->waitOnHost();

        std::vector<uint32_t> data = device->read_buffer<uint32_t>(buffer.get(), 0, 16);

        for (size_t i = 0; i < data.size(); ++i) {
            log_info("data[{}] = {}", i, data[i]);
        }

        return SLANG_OK;
    }
};

struct Test {
    int a = 2;
    bool is_valid(int b) const
    {
        KALI_ASSERT(a == 1);
        return a == b;
    }
};


int main()
{
    ExampleProgram program;
    program.execute();
    return 0;

#if 0
    std::vector<AdapterInfo> adapters = Device::enumerate_adapters();

    log_info("Found {} adapters", adapters.size());
    for (const auto& adapter : adapters)
        log_info("Adapter: {}", adapter.name);

    ref<Device> device = Device::create({.enable_debug_layers = true});

    device->create_typed_buffer<uint4>(1024);

    // device->create_program(ProgramDesc::create_compute());

    // create program (shader file, entry point)
    // create pipeline (program, root signature)
    // create buffer (size)

    // assign buffer to parameter block
    // dispatch compute

#if 0
    uint32_t N = 1024;
    ref<Buffer> a = device->create_buffer();
    ref<Buffer> b = device->create_buffer();
    ref<Buffer> c = device->create_buffer();

    ref<Program> program = device->create_program();
    ref<Pipeline> pipeline = device->create_pipeline({.program = program});

    ref<ShaderObject> shader_object = device->create_shader_object();

    auto compute_context = device->get_compute_context();


    auto vars = shader_object->get_entry_point_vars("main");
    vars["a"] = a;
    vars["b"] = b;
    vars["c"] = c;
    vars["size"] = N;

    compute_context->dispatch(pipeline, shader_object, {N, 1, 1},);

    compute_context->dispatch(pipeline, {{"a", a}, {"b", b}, {"c", c}, {"size", N},}, {N, 1, 1},);
#endif

#if 0
#python
    N = 1024
    a = device.create_buffer(1024)
    b = device.create_buffer(1024)
    c = device.create_buffer(1024)

    program = device.create_program(path="compute.cs.slang", "main")
    pipeline = device.create_pipeline(program=program)

    shader_object = device.create_shader_object()

    vars = shader_object.get_entry_point_vars("main")
    vars.a = a
    vars.b = b
    vars.c = c
    vars.size = N

    compute_context.dispatch(pipeline, entry_point_vars=shader_object, (N, 1, 1))

    compute_context.dispatch(pipeline, entry_point_vars={"a": a, "b": b, "c": c, "size": N} root_vars={}, (N, 1, 1))

#shorter version
    compute_kernel = device.create_compute_kernel("compute.cs.slang", "main")
    compute_kernel(ctx, (N, 1, 1), a=a, b=b, c=c, size=N)

#endif

    return 0;
#endif
}
