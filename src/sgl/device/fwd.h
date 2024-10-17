// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "sgl/core/object.h"

namespace sgl {

// device.h

struct DeviceDesc;
class Device;

// swapchain.h

struct SwapchainDesc;
class Swapchain;

// buffer_cursor.h
class BufferCursor;
class BufferElementCursor;

// resource.h

class Resource;

struct BufferDesc;
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

struct FramebufferLayoutTargetDesc;
struct FramebufferLayoutDesc;
class FramebufferLayout;
struct FramebufferDesc;
class Framebuffer;

// shader.h

struct SlangSessionDesc;
class SlangSession;

class SlangModule;
class SlangComponentType;
class SlangGlobalScope;
class SlangEntryPoint;

class ShaderProgram;

// reflection.h

class DeclReflection;
class DeclReflectionChildList;
class DeclReflectionIndexedChildList;
class TypeReflection;
class TypeReflectionFieldList;
class TypeLayoutReflection;
class TypeLayoutReflectionFieldList;
class FunctionReflection;
class FunctionReflectionParameterList;
class FunctionReflectionOverloadList;
class VariableReflection;
class VariableLayoutReflection;
class EntryPointLayout;
class EntryPointLayoutParameterList;
class ProgramLayout;
class ProgramLayoutParameterList;
class ProgramLayoutEntryPointList;

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

// blit.h

class Blitter;

// texture_loader.h

class TextureLoader;

// hot_reload.h
class HotReload;

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

} // namespace sgl
