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

// shader.h

struct SlangSessionDesc;
class SlangSession;

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
class ComputeKernel;

// pipeline.h

struct ComputePipelineStateDesc;
class ComputePipelineState;
struct GraphicsPipelineStateDesc;
class GraphicsPipelineState;
struct RayTracingPipelineStateDesc;
class RayTracingPipelineState;

// query.h

struct QueryPoolDesc;
class QueryPool;

// raytracing.h

class AccelerationStructure;
struct ShaderTableDesc;
class ShaderTable;

// command.h

struct CommandQueueDesc;
class CommandQueue;
class CommandBuffer;
class CommandStream;
class ComputePassEncoder;
class RenderPassEncoder;
class RayTracingPassEncoder;

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

} // namespace kali
