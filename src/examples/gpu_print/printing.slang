#ifndef PRINT_ENABLED
#define PRINT_ENABLED 0
#endif

/// Printable primitive types.
enum class PrintableType : uint {
    bool_,
    int_,
    uint_,
    float16,
    float32,
    float64,
};

/// Interface to print output.
interface IPrintOutput {
    /// Write a message and reserve space for writing the arguments.
    /// \param fmt The format string.
    /// \param arg_count The number of arguments.
    /// \param total_data_count The total number of data elements in all arguments.
    /// \param[out] offset Offset into the output buffer. This needs to be passed to \c write_arg calls.
    /// \return True if the message was written successfully, false if there was not enough space.
    bool write_msg(String fmt, uint arg_count, uint total_data_count, out uint offset);

    /// Write a single printable argument.
    /// \param[in,out] offset Offset into the output buffer.
    void write_arg(inout uint offset, IPrintable arg);
};

/// Interface for printable types.
interface IPrintable {
    /// The type of the elements.
    [ForceInline]
    static PrintableType printable_element_type();

    /// The number elements (i.e. scalars have one, vectors and matrices have multiple).
    [ForceInline]
    static uint printable_element_count();

    /// The total number of data elements (each element is a \c uint).
    [ForceInline]
    static uint printable_data_count();

    /// Get a printable data element.
    /// \param index The index of the data element in range [0..printable_data_count()).
    [ForceInline]
    uint get_printable_data(uint index);
};

// clang-format off
extension bool : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return PrintableType::bool_; }
    [ForceInline]
    static uint printable_element_count() { return 1; }
    [ForceInline]
    static uint printable_data_count() { return 1; }
    [ForceInline]
    uint get_printable_data(uint index) { return asuint(this); }
};

extension int : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return PrintableType::int_; }
    [ForceInline]
    static uint printable_element_count() { return 1; }
    [ForceInline]
    static uint printable_data_count() { return 1; }
    [ForceInline]
    uint get_printable_data(uint index) { return asuint(this); }
};

extension uint : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return PrintableType::uint_; }
    [ForceInline]
    static uint printable_element_count() { return 1; }
    [ForceInline]
    static uint printable_data_count() { return 1; }
    [ForceInline]
    uint get_printable_data(uint index) { return this; }
};

extension float16_t : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return PrintableType::float16; }
    [ForceInline]
    static uint printable_element_count() { return 1; }
    [ForceInline]
    static uint printable_data_count() { return 1; }
    [ForceInline]
    uint get_printable_data(uint index) { return f32tof16(this); }
}

extension float : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return PrintableType::float32; }
    [ForceInline]
    static uint printable_element_count() { return 1; }
    [ForceInline]
    static uint printable_data_count() { return 1; }
    [ForceInline]
    uint get_printable_data(uint index) { return asuint(this); }
}

extension double : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return PrintableType::float64; }
    [ForceInline]
    static uint printable_element_count() { return 1; }
    [ForceInline]
    static uint printable_data_count() { return 2; }
    [ForceInline]
    uint get_printable_data(uint index) { uint2 data; asuint(this, data.x, data.y); return data[index]; }
}

__generic<T : IPrintable, let N : int>
extension vector<T, N> : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return T::printable_element_type(); }
    [ForceInline]
    static uint printable_element_count() { return N * T::printable_element_count(); }
    [ForceInline]
    static uint printable_data_count() { return N * T::printable_data_count(); }
    [ForceInline]
    uint get_printable_data(uint index) { return this[index / T::printable_data_count()].get_printable_data(index % T::printable_data_count()); }
};

__generic<T : IPrintable, let R : int, let C : int, let L : int>
extension matrix<T, R, C, L> : IPrintable {
    [ForceInline]
    static PrintableType printable_element_type() { return T::printable_element_type(); }
    [ForceInline]
    static uint printable_element_count() { return R * C * T::printable_element_count(); }
    [ForceInline]
    static uint printable_data_count() { return R * C * T::printable_data_count(); }
    [ForceInline]
    uint get_printable_data(uint index) {
        uint element_index = index / T::printable_data_count();
        uint row = element_index / C;
        uint col = element_index % C;
        return this[row][col].get_printable_data(index % T::printable_data_count());
    }
};
// clang-format on

[ForceInline]
void print(IPrintOutput output, String fmt)
{
    uint offset;
    output.write_msg(fmt, 0, 0, offset);
}

[ForceInline]
void print(IPrintOutput output, String fmt, IPrintable arg0)
{
    uint total_data_count = arg0.printable_data_count();
    uint offset;
    if (!output.write_msg(fmt, 1, total_data_count, offset))
        return;
    output.write_arg(offset, arg0);
}

[ForceInline]
void print(IPrintOutput output, String fmt, IPrintable arg0, IPrintable arg1)
{
    uint total_data_count = arg0.printable_data_count() + arg1.printable_data_count();
    uint offset;
    if (!output.write_msg(fmt, 2, total_data_count, offset))
        return;
    output.write_arg(offset, arg0);
    output.write_arg(offset, arg1);
}

[ForceInline]
void print(IPrintOutput output, String fmt, IPrintable arg0, IPrintable arg1, IPrintable arg2)
{
    uint total_data_count = arg0.printable_data_count() + arg1.printable_data_count() + arg2.printable_data_count();
    uint offset;
    if (!output.write_msg(fmt, 3, total_data_count, offset))
        return;
    output.write_arg(offset, arg0);
    output.write_arg(offset, arg1);
    output.write_arg(offset, arg2);
}

[ForceInline]
void print(IPrintOutput output, String fmt, IPrintable arg0, IPrintable arg1, IPrintable arg2, IPrintable arg3)
{
    uint total_data_count = arg0.printable_data_count() + arg1.printable_data_count() + arg2.printable_data_count()
        + arg3.printable_data_count();
    uint offset;
    if (!output.write_msg(fmt, 4, total_data_count, offset))
        return;
    output.write_arg(offset, arg0);
    output.write_arg(offset, arg1);
    output.write_arg(offset, arg2);
    output.write_arg(offset, arg3);
}


struct PrintOutput : IPrintOutput {

    static uint3 current_tid;

    RWByteAddressBuffer counter_buffer;
    RWStructuredBuffer<uint> output_buffer;

    uint3 selected_tid;
    uint output_buffer_capacity;


    static void set_current_tid(uint3 tid) { current_tid = tid; }

    [ForceInline]
    bool write_msg(String fmt, uint arg_count, uint total_data_count, out uint offset)
    {
        offset = 0;
        if (!all(selected_tid == uint3(0xffffffff)) && any(current_tid != selected_tid))
            return false;

        uint size = 3 + arg_count + total_data_count;
        counter_buffer.InterlockedAdd(0, size, offset);
        if (offset + size > output_buffer_capacity)
            return false;

        output_buffer[offset++] = size;
        output_buffer[offset++] = getStringHash(fmt);
        output_buffer[offset++] = arg_count;
        return true;
    }

    [ForceInline]
    void write_arg(inout uint offset, IPrintable arg)
    {
        output_buffer[offset++] = (uint(arg.printable_element_type()) << 24) | (arg.printable_element_count() << 16)
            | (1 + arg.printable_data_count());
        for (uint i = 0; i < arg.printable_data_count(); ++i)
            output_buffer[offset++] = arg.get_printable_data(i);
    }
};

ParameterBlock<PrintOutput> g_print_output;

#if PRINT_ENABLED

[ForceInline]
void print(String fmt)
{
    print(g_print_output, fmt);
}

[ForceInline]
void print(String fmt, IPrintable arg0)
{
    print(g_print_output, fmt, arg0);
}

[ForceInline]
void print(String fmt, IPrintable arg0, IPrintable arg1)
{
    print(g_print_output, fmt, arg0, arg1);
}

[ForceInline]
void print(String fmt, IPrintable arg0, IPrintable arg1, IPrintable arg2)
{
    print(g_print_output, fmt, arg0, arg1, arg2);
}

[ForceInline]
void print(String fmt, IPrintable arg0, IPrintable arg1, IPrintable arg2, IPrintable arg3)
{
    print(g_print_output, fmt, arg0, arg1, arg2, arg3);
}

#else // PRINT_ENABLED

void print(String fmt) { }
void print(String fmt, IPrintable arg0) { }
void print(String fmt, IPrintable arg0, IPrintable arg1) { }
void print(String fmt, IPrintable arg0, IPrintable arg1, IPrintable arg2) { }
void print(String fmt, IPrintable arg0, IPrintable arg1, IPrintable arg2, IPrintable arg3) { }

#endif // PRINT_ENABLED
