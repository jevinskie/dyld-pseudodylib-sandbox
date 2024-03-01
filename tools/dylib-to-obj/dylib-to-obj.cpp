#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <string_view>
#include <type_traits>

#include <LIEF/MachO.hpp>
#include <LIEF/logging.hpp>
#include <argparse/argparse.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/std.h>

namespace fs = std::filesystem;
using namespace std::string_literals;
using namespace LIEF::MachO;

template <> struct fmt::formatter<Binary> : ostream_formatter {};

template <typename T>
concept ToCharStarFreeFunc = requires(T v) {
    { to_string(v) } -> std::same_as<const char *>;
};

template <ToCharStarFreeFunc T> struct fmt::formatter<T> : formatter<string_view> {
    auto format(T v, format_context &ctx) const {
        return formatter<string_view>::format(std::string_view{to_string(v)}, ctx);
    }
};

static bool dylib_to_obj(const fs::path &in_dylib_path, const fs::path &out_obj_path,
                         const bool verbose = false) {
    if (verbose) {
        fmt::print("verbose mode enabled\n");
        fmt::print("verbose mode enabled\nin_dylib_path: {}\nout_obj_path: {}\b", in_dylib_path,
                   out_obj_path);
        LIEF::logging::set_level(LIEF::logging::LOGGING_LEVEL::LOG_TRACE);
    }

    const auto in_fat_dylib = Parser::parse(in_dylib_path.string());
    if (!in_fat_dylib) {
        fmt::print(stderr, "failed to parse '{}'!\n", in_dylib_path);
        return false;
    }
    std::vector<LIEF::MachO::Binary> out_thin_objs;
    for (const auto &in_dylib : *in_fat_dylib) {
        const auto in_ft = in_dylib.header().file_type();
        if (in_ft != FILE_TYPES::MH_DYLIB) {
            fmt::print(stderr, "filetype '{}' isn't MH_DYLIB in slice {}\n", in_ft, in_dylib);
            return false;
        }
    }

    return true;
}

int main(int argc, const char *argv[]) {
    argparse::ArgumentParser parser(getprogname());
    parser.add_argument("-i", "--in-dylib").required().help("input dylib file path");
    parser.add_argument("-o", "--out-obj").required().help("output object file path");
    parser.add_argument("-V", "--verbose")
        .default_value(false)
        .implicit_value(true)
        .help("verbose mode");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {}\n", err.what());
        return -1;
    }

    const auto verbose       = parser.get<bool>("--verbose");
    const auto in_dylib_path = fs::path{parser.get("--in-dylib")};
    const auto out_obj_path  = fs::path{parser.get("--out-obj")};

    const auto res = dylib_to_obj(in_dylib_path, out_obj_path, verbose);

    return res ? 0 : 1;
}
