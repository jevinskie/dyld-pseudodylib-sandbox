#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <filesystem>

#include <LIEF/MachO.hpp>
#include <LIEF/logging.hpp>
#include <argparse/argparse.hpp>
#include <fmt/format.h>
#include <fmt/std.h>

namespace fs = std::filesystem;

int main(int argc, const char *argv[]) {
    argparse::ArgumentParser parser(getprogname());
    parser.add_argument("-i", "--in-dylib").required().help("input dylib file path");
    parser.add_argument("-o", "--out-obj").required().help("output object file path");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {:s}\n", err.what());
        return -1;
    }

    const auto in_dylib_path = fs::path{parser.get("--in-dylib")};
    const auto out_obj_path  = fs::path{parser.get("--out-obj")};

    fmt::print("in dylib: {} out obj: {}\n", in_dylib_path, out_obj_path);

    return 0;
}
