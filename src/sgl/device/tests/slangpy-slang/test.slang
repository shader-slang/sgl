import "slangpy";
import "example.slang";

typealias Context = ContextND<0>;

typealias _t_a = ValueType<float>;
static const int _m_a = 0;

typealias _t_b = ValueType<float>;
static const int _m_b = 0;

typealias _t__result = RWValueRef<float>;
static const int _m__result = 0;

struct CallData {
    uint3 _thread_count;
    _t_a a;
    _t_b b;
    _t__result _result;
};

ParameterBlock<CallData> call_data;

void _trampoline(Context context, CallData data) {
    float _result;
    float a;
    float b;

    data.a.load(context.map(_m_a), a);
    data.b.load(context.map(_m_b), b);

    _result = add(a, b);
    data._result.store(context.map(_m__result), _result);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    if (any(dispatchThreadID >= call_data._thread_count)) {
        return;
    }

    Context context = { dispatchThreadID };
    _trampoline(context, call_data);
}