#include "command.h"

#include "kali/rhi/resource.h"
#include "kali/rhi/formats.h"
#include "kali/rhi/helpers.h"

#include "kali/core/error.h"
#include "kali/core/maths.h"
#include "kali/core/type_utils.h"
#include "kali/math/vector_math.h"

namespace kali {

// defined in resource.cpp
gfx::ResourceState get_gfx_resource_state(ResourceState resource_state);

bool CommandList::buffer_barrier(const Buffer* buffer, ResourceState new_state)
{
    KALI_ASSERT(buffer);

    bool recorded = false;
    ResourceState current_state = buffer->get_global_state();
    if (current_state != new_state) {
        auto encoder = get_gfx_resource_command_encoder();
        gfx::IBufferResource* gfx_buffer_resource = buffer->get_gfx_buffer_resource();
        encoder->bufferBarrier(
            1,
            &gfx_buffer_resource,
            get_gfx_resource_state(current_state),
            get_gfx_resource_state(new_state)
        );
        buffer->set_global_state(new_state);
        mark_pending();
        recorded = true;
    }

    return recorded;
}

bool CommandList::texture_barrier(const Texture* texture, ResourceState new_state)
{
    KALI_ASSERT(texture);

    bool recorded = false;
    ResourceState current_state = texture->get_global_state();
    if (current_state != new_state) {
        auto encoder = get_gfx_resource_command_encoder();
        gfx::ITextureResource* gfx_texture_resource = texture->get_gfx_texture_resource();
        encoder->textureBarrier(
            1,
            &gfx_texture_resource,
            get_gfx_resource_state(current_state),
            get_gfx_resource_state(new_state)
        );
        texture->set_global_state(new_state);
        mark_pending();
        recorded = true;
    }

    return recorded;
}

void CommandList::uav_barrier(const Resource* resource)
{
    KALI_ASSERT(resource);

    auto encoder = get_gfx_resource_command_encoder();
    if (resource->get_type() == ResourceType::buffer) {
        gfx::IBufferResource* buffer = static_cast<gfx::IBufferResource*>(resource->get_gfx_resource());
        encoder->bufferBarrier(1, &buffer, gfx::ResourceState::UnorderedAccess, gfx::ResourceState::UnorderedAccess);
    } else {
        gfx::ITextureResource* texture = static_cast<gfx::ITextureResource*>(resource->get_gfx_resource());
        encoder->textureBarrier(1, &texture, gfx::ResourceState::UnorderedAccess, gfx::ResourceState::UnorderedAccess);
    }
    mark_pending();
}

#if 0
void CommandList::copy_resource(const Resource* dst, const Resource* src)
{
    // Copy from texture to texture or from buffer to buffer.
    KALI_ASSERT(dst->get_type() == src->get_type());

    resource_barrier(dst, ResourceState::copy_destination);
    resource_barrier(src, ResourceState::copy_source);

    auto encoder = get_gfx_resource_command_encoder();

    if (dst->get_type() == ResourceType::buffer) {
        const Buffer* dst_buffer = static_cast<const Buffer*>(dst);
        const Buffer* src_buffer = static_cast<const Buffer*>(src);

        KALI_ASSERT(src_buffer->get_size() <= dst_buffer->get_size());

        encoder->copyBuffer(
            dst_buffer->get_gfx_buffer_resource(),
            dst_buffer->get_gpu_address_offset(),
            src_buffer->get_gfx_buffer_resource(),
            src_buffer->get_gpu_address_offset(),
            src_buffer->get_size()
        );
    } else {
        const Texture* dst_texture = static_cast<const Texture*>(dst);
        const Texture* src_texture = static_cast<const Texture*>(src);
        gfx::SubresourceRange sr_range = {};

        encoder->copyTexture(
            dst_texture->get_gfx_texture_resource(),
            gfx::ResourceState::CopyDestination,
            sr_range,
            gfx::ITextureResource::Offset3D(0, 0, 0),
            src_texture->get_gfx_texture_resource(),
            gfx::ResourceState::CopySource,
            sr_range,
            gfx::ITextureResource::Offset3D(0, 0, 0),
            gfx::ITextureResource::Extents{0, 0, 0}
        );
    }

    mark_pending();
}
#endif

void CommandList::copy_buffer_region(
    const Buffer* dst,
    DeviceOffset dst_offset,
    const Buffer* src,
    DeviceOffset src_offset,
    DeviceSize size
)
{
    // TODO(@skallweit): check for overlapping copies within same buffer?
    KALI_ASSERT(dst);
    KALI_ASSERT(src);
    KALI_ASSERT_LE(dst_offset + size, dst->get_size());
    KALI_ASSERT_LE(src_offset + size, src->get_size());

    buffer_barrier(dst, ResourceState::copy_destination);
    buffer_barrier(src, ResourceState::copy_source);

    auto encoder = get_gfx_resource_command_encoder();
    encoder->copyBuffer(dst->get_gfx_buffer_resource(), dst_offset, src->get_gfx_buffer_resource(), src_offset, size);
    mark_pending();
}

#if 0
void CommandList::copy_texture_region(
    const Texture* dst,
    uint32_t dst_sub_index,
    const Texture* src,
    uint32_t src_sub_index,
    const uint3& dstOffset,
    const uint3& srcOffset,
    const uint3& size
)
{
    texture_barrier(dst, ResourceState::copy_destination);
    texture_barrier(src, ResourceState::copy_source);

    gfx::SubresourceRange dstSubresource = {};
    dstSubresource.baseArrayLayer = dst->getSubresourceArraySlice(dstSubresourceIdx);
    dstSubresource.layerCount = 1;
    dstSubresource.mipLevel = dst->getSubresourceMipLevel(dstSubresourceIdx);
    dstSubresource.mipLevelCount = 1;

    gfx::SubresourceRange srcSubresource = {};
    srcSubresource.baseArrayLayer = src->getSubresourceArraySlice(srcSubresourceIdx);
    srcSubresource.layerCount = 1;
    srcSubresource.mipLevel = src->getSubresourceMipLevel(srcSubresourceIdx);
    srcSubresource.mipLevelCount = 1;

    gfx::ITextureResource::Extents copySize = {(int)size.x, (int)size.y, (int)size.z};

    if (size.x == glm::uint(-1)) {
        copySize.width = src->getWidth(srcSubresource.mipLevel) - srcOffset.x;
        copySize.height = src->getHeight(srcSubresource.mipLevel) - srcOffset.y;
        copySize.depth = src->getDepth(srcSubresource.mipLevel) - srcOffset.z;
    }

    auto resourceEncoder = getLowLevelData()->getResourceCommandEncoder();
    resourceEncoder->copyTexture(
        dst->getGfxTextureResource(),
        gfx::ResourceState::CopyDestination,
        dstSubresource,
        gfx::ITextureResource::Offset3D(dstOffset.x, dstOffset.y, dstOffset.z),
        src->getGfxTextureResource(),
        gfx::ResourceState::CopySource,
        srcSubresource,
        gfx::ITextureResource::Offset3D(srcOffset.x, srcOffset.y, srcOffset.z),
        copySize
    );
    mCommandsPending = true;
}
#endif

void CommandList::upload_buffer_data(const Buffer* buffer, size_t offset, size_t size, const void* data)
{
    KALI_ASSERT(buffer);
    KALI_ASSERT_LE(offset + size, buffer->get_size());
    KALI_ASSERT(data);

    buffer_barrier(buffer, ResourceState::copy_destination);

    auto encoder = get_gfx_resource_command_encoder();
    encoder->uploadBufferData(buffer->get_gfx_buffer_resource(), offset, size, const_cast<void*>(data));
    mark_pending();
}

void CommandList::upload_texture_data(const Texture* texture, const void* data)
{
    KALI_ASSERT(texture);
    KALI_ASSERT(data);

    upload_texture_subresource_data(texture, 0, texture->get_subresource_count(), data);
}

void CommandList::upload_texture_subresource_data(
    const Texture* texture,
    uint32_t subresource_index,
    uint32_t subresource_count,
    const void* data,
    uint3 offset,
    uint3 size
)
{
    KALI_ASSERT(texture);
    KALI_ASSERT_LT(subresource_index, texture->get_subresource_count());
    KALI_ASSERT_LE(subresource_index + subresource_count, texture->get_subresource_count());
    KALI_ASSERT_GE(subresource_count, 1);
    KALI_ASSERT(data);

    bool copy_region = (any(offset != uint3{0})) || any((size != uint3{0xffffffff}));
    KALI_ASSERT(subresource_count == 1 || (copy_region == false));

    texture_barrier(texture, ResourceState::copy_destination);

    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(data);
    auto encoder = get_gfx_resource_command_encoder();
    gfx::ITextureResource::Offset3D gfx_offset = {
        narrow_cast<gfx::GfxIndex>(offset.x),
        narrow_cast<gfx::GfxIndex>(offset.y),
        narrow_cast<gfx::GfxIndex>(offset.z),
    };
    gfx::ITextureResource::Extents gfx_size = {
        narrow_cast<gfx::GfxCount>(size.x),
        narrow_cast<gfx::GfxCount>(size.y),
        narrow_cast<gfx::GfxCount>(size.z),
    };
    gfx::FormatInfo format_info = {};
    SLANG_CALL(gfx::gfxGetFormatInfo(get_gfx_format(texture->get_format()), &format_info));
    for (uint32_t i = subresource_index; i < subresource_index + subresource_count; i++) {
        gfx::SubresourceRange sr_range = {};
        // sr_range.baseArrayLayer = narrow_cast<gfx::GfxIndex>(pTexture->getSubresourceArraySlice(i));
        // sr_range.mipLevel = narrow_cast<gfx::GfxIndex>(pTexture->getSubresourceMipLevel(i));
        sr_range.layerCount = 1;
        sr_range.mipLevelCount = 1;
        if (!copy_region) {
            gfx_size.width
                = align_to(format_info.blockWidth, narrow_cast<gfx::GfxCount>(texture->get_width(sr_range.mipLevel)));
            gfx_size.height
                = align_to(format_info.blockHeight, narrow_cast<gfx::GfxCount>(texture->get_height(sr_range.mipLevel)));
            gfx_size.depth = narrow_cast<gfx::GfxCount>(texture->get_depth(sr_range.mipLevel));
        }
        gfx::ITextureResource::SubresourceData sr_data = {};
        sr_data.data = data_ptr;
        sr_data.strideY = narrow_cast<int64_t>(gfx_size.width) / format_info.blockWidth * format_info.blockSizeInBytes;
        sr_data.strideZ = sr_data.strideY * (gfx_size.height / format_info.blockHeight);
        data_ptr += sr_data.strideZ * gfx_size.depth;
        encoder->uploadTextureData(texture->get_gfx_texture_resource(), sr_range, gfx_offset, gfx_size, &sr_data, 1);
    }

    mark_pending();
}


void CommandList::mark_pending() { }

gfx::IResourceCommandEncoder* CommandList::get_gfx_resource_command_encoder()
{
    return nullptr;
}

} // namespace kali
