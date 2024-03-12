// SPDX-License-Identifier: Apache-2.0

import test_link_time_type_binary_op;

extern struct BINARY_OP : IBinaryOp;

RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    result[0] = BINARY_OP::apply(1.0, 2.0);
}
