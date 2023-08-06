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

Slang::ComPtr<slang::IModule> compileShaderModuleFromFile(slang::ISession* slangSession, char const* filePath)
{
    SlangCompileRequest* slangRequest = nullptr;
    slangSession->createCompileRequest(&slangRequest);

    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, filePath);
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, filePath);

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
    int gWindowWidth = 640;
    int gWindowHeight = 480;

    Slang::ComPtr<gfx::IDevice> gfx_device;

    Slang::ComPtr<slang::ISession> gSlangSession;
    Slang::ComPtr<slang::IModule> gSlangModule;
    Slang::ComPtr<gfx::IShaderProgram> gProgram;

    Slang::ComPtr<gfx::IPipelineState> gPipelineState;

    // Slang::Dictionary<int, std::string> gHashedStrings;

    // GPUPrinting gGPUPrinting;

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

        auto shaderProgram = gfx_device->createProgram(programDesc);

        return shaderProgram;
    }

    Slang::Result execute()
    {
        ref<Device> device = Device::create({.type = DeviceType::automatic, .enable_debug_layers = true});
        gfx_device = device->get_gfx_device();

        // gfx::IDevice::Desc deviceDesc = {};
        // deviceDesc.deviceType = gfx::DeviceType::DirectX12;
        // Slang::Result res = gfxCreateDevice(&deviceDesc, gfx_device.writeRef());
        // if (SLANG_FAILED(res))
        //     return res;

        gSlangSession = createSlangSession(gfx_device);
        gSlangModule = compileShaderModuleFromFile(
            gSlangSession,
            (get_project_directory() / "src/examples/simple_compute/compute.cs.slang").c_str()
        );

        gProgram = loadComputeProgram(gSlangModule, "main");
        if (!gProgram)
            return SLANG_FAIL;

        gfx::ComputePipelineStateDesc desc;
        desc.program = gProgram;
        auto pipelineState = gfx_device->createComputePipelineState(desc);
        if (!pipelineState)
            return SLANG_FAIL;

        gPipelineState = pipelineState;

        size_t buffer_size = 4 * 1024;

        ref<Buffer> buffer2
            = device->create_raw_buffer(buffer_size, ResourceUsage::unordered_access, MemoryType::device_local);
        auto gfx_buffer = buffer2->get_gfx_buffer_resource();

        // gfx::IBufferResource::Desc gfx_buffer_desc = {};
        // gfx_buffer_desc.type = gfx::IResource::Type::Buffer;
        // gfx_buffer_desc.sizeInBytes = buffer_size;
        // gfx_buffer_desc.elementSize = sizeof(float);
        // gfx_buffer_desc.defaultState = gfx::ResourceState::UnorderedAccess;
        // gfx_buffer_desc.allowedStates = gfx::ResourceStateSet(
        //     gfx::ResourceState::CopySource,
        //     gfx::ResourceState::CopyDestination,
        //     gfx::ResourceState::UnorderedAccess
        // );
        // gfx_buffer_desc.memoryType = gfx::MemoryType::DeviceLocal;
        // auto gfx_buffer = gfx_device->createBufferResource(gfx_buffer_desc);

        gfx::IResourceView::Desc bufferViewDesc = {};
        bufferViewDesc.type = gfx::IResourceView::Type::UnorderedAccess;
        bufferViewDesc.format = gfx::Format::Unknown;
        auto bufferView = gfx_device->createBufferView(gfx_buffer, nullptr, bufferViewDesc);

        gfx::ITransientResourceHeap::Desc transientResourceHeapDesc = {};
        transientResourceHeapDesc.constantBufferSize = 256;
        auto transientHeap = gfx_device->createTransientResourceHeap(transientResourceHeapDesc);

        // gfx::ICommandQueue::Desc queueDesc = {gfx::ICommandQueue::QueueType::Graphics};
        // auto queue = gfx_device->createCommandQueue(queueDesc);
        auto queue = device->get_gfx_queue();

        auto commandBuffer = transientHeap->createCommandBuffer();
        auto encoder = commandBuffer->encodeComputeCommands();
        auto rootShaderObject = encoder->bindPipeline(gPipelineState);
        auto cursor = gfx::ShaderCursor(rootShaderObject);
        cursor["g_buffer"].setResource(bufferView);
        encoder->dispatchCompute(1024 / 32, 1, 1);
        encoder->bufferBarrier(gfx_buffer, gfx::ResourceState::UnorderedAccess, gfx::ResourceState::CopySource);
        encoder->endEncoding();
        commandBuffer->close();
        queue->executeCommandBuffer(commandBuffer);

        queue->waitOnHost();

        Slang::ComPtr<ISlangBlob> blob;
        gfx_device->readBufferResource(gfx_buffer, 0, buffer_size, blob.writeRef());

        const uint32_t* data = (const uint32_t*)blob->getBufferPointer();
        for (int i = 0; i < 16; ++i) {
            log_info("data[{}] = {}", i, data[i]);
        }

        // gGPUPrinting.processGPUPrintCommands(blob->getBufferPointer(), printBufferSize);

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
