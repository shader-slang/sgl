// SPDX-License-Identifier: Apache-2.0

import printing;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    uint u = 123;
    int i = -123;
    float f = 1.23;
    double d = 2.34;

    g_print_output.set_current_tid(tid);

    print("Hello world!");
    print("tid = {}", tid);
    print("u = {}", u);
    print("i = {}", i);
    print("f = {}", f);
    print("d = {}", d);
    print("float3 = {}", float3(1, 2, 3));
    print("a = {}, b = {}, c = {}", 1, 2, 3);
    print("a = {}, b = {}, c = {}", bool4(true), double4(123.456), float4x4(1.0));
}
