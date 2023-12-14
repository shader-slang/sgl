#pragma once

/**
 * CUDA runtime defines vector types in the global namespace. Some of these
 * types clash with the vector types in kali, which live in the kali::math
 * and kali namespace. To avoid this clash, we rename the CUDA types here.
 * kali code should includle this header instead of <cuda_runtime.h>.
 */

#define int1 cuda_int1
#define int2 cuda_int2
#define int3 cuda_int3
#define int4 cuda_int4
#define uint1 cuda_uint1
#define uint2 cuda_uint2
#define uint3 cuda_uint3
#define uint4 cuda_uint4
#define float1 cuda_float1
#define float2 cuda_float2
#define float3 cuda_float3
#define float4 cuda_float4

#include <cuda_runtime.h>

#undef int1
#undef int2
#undef int3
#undef int4
#undef uint1
#undef uint2
#undef uint3
#undef uint4
#undef float1
#undef float2
#undef float3
#undef float4
