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

class ProgramLayout;
class EntryPointLayout;

// pipeline.h

struct ComputePipelineStateDesc;
class ComputePipelineState;
class ComputePipelineCache;
struct GraphicsPipelineStateDesc;
class GraphicsPipelineState;
class GraphicsPipelineCache;

// command_queue.h

struct CommandQueueDesc;
class CommandQueue;

// command_stream.h

struct CommandStreamDesc;
class CommandStream;

// shader_cursor.h

class ShaderCursor;
class ShaderObject;

} // namespace kali
