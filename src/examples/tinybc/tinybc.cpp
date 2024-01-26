#include "kali/kali.h"
#include "kali/core/platform.h"
#include "kali/core/bitmap.h"
#include "kali/core/timer.h"
#include "kali/device/device.h"
#include "kali/device/shader.h"
#include "kali/device/command.h"
#include "kali/device/shader_cursor.h"
#include "kali/device/shader_object.h"
#include "kali/device/kernel.h"
#include "kali/device/query.h"
#include "kali/device/agility_sdk.h"
#include "kali/utils/tev.h"

#include <argparse/argparse.hpp>

KALI_EXPORT_AGILITY_SDK

static const std::filesystem::path EXAMPLE_DIR(KALI_EXAMPLE_DIR);

using namespace kali;

int main(int argc, const char* argv[])
{
    argparse::ArgumentParser args("tinybc");
    args.add_description("Slang-based BC7 - mode 6 compressor");
    args.add_argument("input_path").required().help("Path to the input texture.");
    args.add_argument("-o", "--output_path").help("Optional path to save the decoded BC7 texture.");
    args.add_argument("-s", "--opt_steps").default_value(100).help("Number of optimization (gradient descene) steps.");
    args.add_argument("-b", "--benchmark")
        .default_value(false)
        .implicit_value(true)
        .help("Run in benchmark mode to measure processing time.");
    args.add_argument("-t", "--tev").default_value(false).implicit_value(true).help("Show images in tev image viewer.");
    args.add_argument("-v", "--verbose").default_value(false).implicit_value(true).help("Enable verbose logging.");

    try {
        // args.parse_args(argc, argv);
        KALI_UNUSED(argc, argv);
        args.parse_args(
            {"tinybc", "C:/projects/kali/monalisa.jpg", "-o", "C:/projects/kali/monalisa_bc7.jpg", "-b", "-v"}
        );
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    std::string input_path = args.get("input_path");
    std::optional<std::string> output_path = args.present("output_path");
    int opt_steps = args.get<int>("opt_steps");
    bool benchmark = args.get<bool>("benchmark");
    bool tev = args.get<bool>("tev");
    bool verbose = args.get<bool>("verbose");

    kali::static_init();

    ref<Bitmap> input;

    try {
        input = make_ref<Bitmap>(input_path)->convert(Bitmap::PixelFormat::rgba, Bitmap::ComponentType::float32, false);
    } catch (const std::exception& e) {
        fmt::println("Failed to load input image from {}:\n{}", input_path, e.what());
        return 1;
    }

    uint32_t w = input->width();
    uint32_t h = input->height();

    ref<Device> device = Device::create({.enable_debug_layers = verbose});

    // Create input texture
    ref<Texture> input_tex = device->create_texture({
        .type = TextureType::texture_2d,
        .format = Format::rgba32_float,
        .width = w,
        .height = h,
        .mip_count = 1,
        .usage = ResourceUsage::shader_resource,
    });
    // TODO use convenience function (or init data) when available
    {
        ref<CommandBuffer> command_buffer = device->create_command_buffer();
        command_buffer->upload_texture_data(input_tex, 0, input->data());
        command_buffer->submit();
    }

    // Show input texture in tev
    if (tev)
        utils::show_in_tev_async(input_tex, "tinybc-input");

    // Create decoded texture
    ref<Texture> decoded_tex = device->create_texture({
        .type = TextureType::texture_2d,
        .format = Format::rgba32_float,
        .width = w,
        .height = h,
        .mip_count = 1,
        .usage = ResourceUsage::unordered_access,
    });

    ref<ComputeKernel> encoder = device
                                     ->load_module(
                                         EXAMPLE_DIR / "tinybc.slang",
                                         DefineList({
                                             {"CONFIG_USE_ADAM", "true"},
                                             {"CONFIG_OPT_STEPS", std::to_string(opt_steps)},
                                         })
                                     )
                                     ->create_compute_kernel("main");

    uint32_t num_iters = benchmark ? 1000 : 1;

    Timer t;

    // Setup query pool to measure GPU time
    ref<QueryPool> queries = device->create_query_pool({.type = QueryType::timestamp, .count = num_iters * 2});

    // Compress!
    ref<CommandBuffer> command_buffer = device->create_command_buffer();
    for (uint32_t i = 0; i < num_iters; ++i) {
        command_buffer->write_timestamp(queries, i * 2);
        encoder->dispatch(
            uint3(w, h, 1),
            [&](ShaderCursor cursor)
            {
                cursor["input_tex"] = input_tex;
                cursor["decoded_tex"] = decoded_tex;
                cursor["lr"] = 0.1f;
                cursor["adam_beta_1"] = 0.9f;
                cursor["adam_beta_2"] = 0.999f;
            },
            command_buffer
        );
        command_buffer->write_timestamp(queries, i * 2 + 1);
    }
    command_buffer->submit();

    device->wait();

    double total_cpu_time_sec = t.elapsed_s();

    std::vector<double> times = queries->get_timestamp_results(0, num_iters * 2);
    double comp_time_sec = 0.0;
    for (uint32_t i = 0; i < num_iters; ++i)
        comp_time_sec += (times[i * 2 + 1] - times[i * 2]);
    comp_time_sec /= num_iters;

    // Calculate and print performance metrics
    if (benchmark) {
        double textures_per_sec = 1.0 / comp_time_sec;
        double giga_texels_per_sec = w * h * textures_per_sec / 1e9;
        fmt::println("Benchmark:");
        fmt::println("- Number of optimization steps: {}", opt_steps);
        fmt::println("- Compression time: {:.4g} ms", comp_time_sec * 1e3);
        fmt::println("- Compression throughput: {:.4g} GTexels/s", giga_texels_per_sec);
        fmt::println("- Total CPU time: {:.4g} s", total_cpu_time_sec);
    }

    // Calculate and print PSNR
    ref<Bitmap> decoded = decoded_tex->to_bitmap();
    double mse = 0.0;
    const float* input_data = input->data_as<float>();
    const float* decoded_data = decoded->data_as<float>();
    for (uint32_t i = 0; i < w * h * 4; ++i)
        mse += (input_data[i] - decoded_data[i]) * (input_data[i] - decoded_data[i]);
    mse /= w * h * 4;
    double psnr = 20.0 * std::log10(1.0 / std::sqrt(mse));
    fmt::println("PSNR: {:.4g}", psnr);

    // Show decoded texture in tev
    if (tev)
        utils::show_in_tev_async(decoded_tex, "tinybc-decoded");

    // Output decoded texture
    if (output_path) {
        decoded_tex->to_bitmap()
            ->convert(Bitmap::PixelFormat::rgb, Bitmap::ComponentType::uint8, true)
            ->write_async(*output_path);
    }

    kali::static_shutdown();

    return 0;
}
