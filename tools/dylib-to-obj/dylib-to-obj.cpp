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
template <> struct fmt::formatter<Header> : ostream_formatter {};
template <> struct fmt::formatter<SegmentCommand> : ostream_formatter {};
template <> struct fmt::formatter<Section> : ostream_formatter {};

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
    std::vector<std::unique_ptr<Binary>> out_thin_objs;
    for (const auto &in_dylib : *in_fat_dylib) {
        const auto in_hdr = in_dylib.header();
        const auto in_ft  = in_hdr.file_type();
        if (in_ft != FILE_TYPES::MH_DYLIB) {
            fmt::print(stderr, "filetype '{}' isn't MH_DYLIB in slice:\n{}\n", in_ft, in_dylib);
            return false;
        }
        // auto obj = Binary{};
        auto obj_hdr = Header{};
        obj_hdr.magic(in_hdr.magic());
        obj_hdr.cpu_type(in_hdr.cpu_type());
        obj_hdr.cpu_subtype(in_hdr.cpu_subtype());
        obj_hdr.nb_cmds(0);
        obj_hdr.sizeof_cmds(0);
        obj_hdr.reserved(0);
        obj_hdr.file_type(FILE_TYPES::MH_OBJECT);
        obj_hdr.flags(0);
        if (verbose) {
            fmt::print("new object header:\n{}\n", obj_hdr);
        }
        // obj.header() = obj_hdr;
        std::vector<SegmentCommand> obj_segs;
        std::vector<Section> obj_sects;
        for (const auto &in_seg : in_dylib.segments()) {
            auto obj_seg = SegmentCommand{};
            obj_seg.command(in_seg.command());
            obj_seg.name(in_seg.name() + "_PD");
            obj_seg.flags(in_seg.flags());
            obj_seg.init_protection(in_seg.init_protection());
            obj_seg.max_protection(in_seg.max_protection());
            for (const auto &in_sect : in_seg.sections()) {
                auto obj_sect = Section{};
                obj_sect.name(in_sect.name());
                obj_sect.address(in_sect.address());
                obj_sect.virtual_address(in_sect.virtual_address());
                obj_sect.alignment(in_sect.alignment());
                obj_sect.offset(in_sect.offset());
                obj_sect.size(in_sect.size());
                obj_sect.flags(in_sect.flags());
                const auto content = in_sect.content();
                obj_sect.content(std::vector<uint8_t>{content.begin(), content.end()});
                if (verbose) {
                    fmt::print("new section:\n{}\n", obj_sect);
                }
                obj_seg.add_section(obj_sect);
            }
            fmt::print("new segment:\n{}\n", obj_seg);
        }
        // out_thin_objs.emplace_back(std::move(obj));
    }

    // auto obj_fat = std::make_unique<FatBinary>(std::move(out_thin_objs));

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
