// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "kali/core/object.h"

namespace kali {

// device.h

struct DeviceDesc;
class Device;

// swapchain.h

struct SwapchainDesc;
class Swapchain;

// resource.h

class Resource;

struct BufferDesc;
struct StructuredBufferDesc;
struct TypedBufferDesc;
class Buffer;

struct TextureDesc;
class Texture;

struct ResourceViewDesc;
class ResourceView;

// sampler.h

struct SamplerDesc;
class Sampler;

// fence.h

struct FenceDesc;
class Fence;

// framebuffer.h

struct FramebufferAttachmentDesc;
struct FramebufferDesc;
class Framebuffer;

// shader.h

struct SlangSessionDesc;
class SlangSession;

class DefineList;
class SlangModule;
class SlangComponentType;
class SlangGlobalScope;
class SlangEntryPoint;

class ShaderProgram;

// reflection.h

class TypeReflection;
class TypeLayoutReflection;
class VariableReflection;
class VariableLayoutReflection;
class ProgramLayout;
class EntryPointLayout;

// kernel.h

class Kernel;
struct ComputeKernelDesc;
class ComputeKernel;

// pipeline.h

class Pipeline;
struct ComputePipelineDesc;
class ComputePipeline;
struct GraphicsPipelineDesc;
class GraphicsPipeline;
struct RayTracingPipelineDesc;
class RayTracingPipeline;

// query.h

struct QueryPoolDesc;
class QueryPool;

// raytracing.h

struct AccelerationStructureDesc;
class AccelerationStructure;
struct ShaderTableDesc;
class ShaderTable;

// command.h

struct CommandQueueDesc;
class CommandQueue;
class CommandBuffer;
class ComputeCommandEncoder;
class RenderCommandEncoder;
class RayTracingCommandEncoder;

// shader_cursor.h

class ShaderCursor;
class ShaderObject;
class TransientShaderObject;
class MutableShaderObject;

// input_layout.h

struct InputLayoutDesc;
class InputLayout;

// memory_heap.h

struct MemoryHeapDesc;
class MemoryHeap;

// cuda_interop.h

namespace cuda {
    struct TensorView;
    class InteropBuffer;
} // namespace cuda

// cuda_utils.h

namespace cuda {
    class Device;
    class ExternalMemory;
    class ExternalSemaphore;
} // namespace cuda

} // namespace kali
