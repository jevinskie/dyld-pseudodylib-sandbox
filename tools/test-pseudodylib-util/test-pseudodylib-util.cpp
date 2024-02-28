#include "test-pseudodylib/test-pseudodylib.hpp"

#undef NDEBUG
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <locale>

#include <argparse/argparse.hpp>
#include <fmt/format.h>

namespace fs = std::filesystem;

using namespace TestPeudodylib;

int main(int argc, const char *argv[]) {
    std::locale::global(std::locale("en_US.UTF-8"));
    argparse::ArgumentParser parser(getprogname());
    parser.add_argument("n").help("value to take factorial of").scan<'u', uint8_t>();

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        fmt::print(stderr, "Error parsing arguments: {:s}\n", err.what());
        return -1;
    }

    const auto n      = parser.get<uint8_t>("n");
    const auto fact_n = Factorial::fact(n);
    if (fact_n == UINT64_MAX) {
        fmt::print("fact({:d}) can't be computed within 64 bits :'<\n", static_cast<uint8_t>(n));
        return -2;
    }
    fmt::print("fact({:d}) = {:d}\n", static_cast<uint8_t>(n), fact_n);

    return 0;
}
