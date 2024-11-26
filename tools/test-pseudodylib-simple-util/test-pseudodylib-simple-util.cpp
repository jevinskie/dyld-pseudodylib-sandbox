#include "fmt/base.h"
#include "test-pseudodylib-simple/test-pseudodylib-simple.h"
#include <cstddef>
#include <mach/vm_types.h>

#undef NDEBUG
#include <cassert>

#include <LIEF/MachO.hpp>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <mach/mach.h>
#include <mach/vm_prot.h>
#include <pthread.h>
#include <sys/mman.h>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace LIEF::MachO;

extern "C" {
extern void *test_pseudodylib_simple_begin;
extern void *test_pseudodylib_simple_begin_inner;
extern void *test_pseudodylib_simple_end_inner;
extern void *test_pseudodylib_simple_end;

// Symbol flags type for symbols defined via the pseudo-dylibs APIs.
typedef uint64_t _dyld_pseudodylib_symbol_flags;

// Flag values for _dyld_pseudodylib_symbol_flags.
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_NONE     0
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_FOUND    1
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_WEAK_DEF 2
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_CALLABLE 4

typedef void (*_dyld_pseudodylib_dispose_error_message)(char *err_msg);
typedef char *(*_dyld_pseudodylib_initialize)(void *pd_ctx, const void *mh);
typedef char *(*_dyld_pseudodylib_deinitialize)(void *pd_ctx, const void *mh);
typedef char *(*_dyld_pseudodylib_lookup_symbols)(void *pd_ctx, const void *mh, const char *names[], size_t num_names,
                                                  void *addrs[], _dyld_pseudodylib_symbol_flags *flags);
typedef int (*_dyld_pseudodylib_lookup_address)(void *pd_ctx, const void *mh, const void *addr, struct dl_info *dl);
typedef char *(*_dyld_pseudodylib_find_unwind_sections)(void *pd_ctx, const void *mh, const void *addr, bool *found,
                                                        struct dyld_unwind_sections *info);

// Versioned struct to hold pseudo-dylib callbacks.
// See _dyld_pseudodylib_callbacks_v1.
struct _dyld_pseudodylib_callbacks {
    uintptr_t version;
};

// Callbacks to implement pseudo-dylib behavior.
//
// dispose_error_message will be called to destroy error messages returned by the other callbacks.
// initialize will be called by dlopen to run initializers in the pseudo-dylib.
// deinitialize will be called by dlclose to run deinitializers.
// lookup_symbols will be called to find the address of symbols defined by the pseudo-dylib (e.g. by
// dlsym). lookup_address will be called by dladdr to find information about the given address.
// find_unwind_sections will be called by _dyld_find_unwind_sections.
struct _dyld_pseudodylib_callbacks_v1 {
    uintptr_t version; // == 1
    _dyld_pseudodylib_dispose_error_message dispose_error_message;
    _dyld_pseudodylib_initialize initialize;
    _dyld_pseudodylib_deinitialize deinitialize;
    _dyld_pseudodylib_lookup_symbols lookup_symbols;
    _dyld_pseudodylib_lookup_address lookup_address;
    _dyld_pseudodylib_find_unwind_sections find_unwind_sections;
};

typedef struct _dyld_pseudodylib_callbacks_opaque *_dyld_pseudodylib_callbacks_handle;
typedef struct _dyld_pseudodylib_opaque *_dyld_pseudodylib_handle;

// pseudo-dylib registration SPIs.
//
// These APIs can be used to register "pseudo-dylibs" which present as dylibs when accessed via the
// dlfcn.h functions (dlopen, dlclose, dladdr, dlsym), but are backed by a set of callbacks rather
// than a full mach-o image.
//
// _dyld_pseudodylib_register_callbacks is used to register a set of callbacks that can be shared
// between multiple pseudo-dylibs. On success, _dyld_pseudodylib_register_callbacks will return a
// handle that can be used in calls to register pseudo-dylib instances (see
// _dyld_pseudodylib_register below). On failure it will return null. Registered callbacks should be
// deregistered by calling _dyld_pseudodylib_deregister_callbacks once all pseudo-dylibs using the
// callbacks have been deregistered.
//
// _dyld_pseudodylib_register is used to register an instance of a pseudo-dylib. This can be thought
// of as equivalent to creating a dylib on disk: the pseudo-dylib is not yet open, but can be found
// via its install-name by dlopen. Registration takes the address range that the pseudo-dylib can
// occupy, and this range must start with a valid mach header and load commands containing, at
// minimum, an LC_VERSION_MIN and LC_ID_DYLIB command identifying the pseudo-dylib's install name.
// The callbacks argument identifies the set of callbacks to use for this pseudo-dylib instance, and
// the opaque context pointer will be passed to each of these callbacks. Once a pseduo-dylib is no
// longer needed it should be deregistered by calling _dyld_pseudodylib_deregister (equivalent to
// "rm'ing" a dylib on disk).
//
// Exists in Mac OS X 14.0 and later
// Exists in iOS 17.0 and later
extern _dyld_pseudodylib_callbacks_handle
_dyld_pseudodylib_register_callbacks(const struct _dyld_pseudodylib_callbacks *callbacks);
extern void _dyld_pseudodylib_deregister_callbacks(_dyld_pseudodylib_callbacks_handle callbacks_handle);
extern _dyld_pseudodylib_handle
_dyld_pseudodylib_register(void *addr, size_t size, _dyld_pseudodylib_callbacks_handle callbacks_handle, void *context);
extern void _dyld_pseudodylib_deregister(_dyld_pseudodylib_handle pd_handle);
}

template <> struct fmt::formatter<Binary> : fmt::ostream_formatter {};
template <> struct fmt::formatter<FatBinary> : fmt::ostream_formatter {};
template <> struct fmt::formatter<ChainedBindingInfo> : fmt::ostream_formatter {};

void my_dyld_pseudodylib_dispose_error_message(char *err_msg) {
    fmt::print("dispose_error_message: msg: '{:s}'\n", err_msg);
}

char *my_dyld_pseudodylib_initialize(void *pd_ctx, const void *mh) {
    fmt::print("my_dyld_pseudodylib_initialize(pd_ctx = {}, mh = {}) called\n", fmt::ptr(pd_ctx), fmt::ptr(mh));
    return nullptr;
}

char *my_dyld_pseudodylib_deinitialize(void *pd_ctx, const void *mh) {
    fmt::print("my_dyld_pseudodylib_deinitialize(pd_ctx = {}, mh = {}) called\n", fmt::ptr(pd_ctx), fmt::ptr(mh));
    return nullptr;
}

std::string get_names_str(const char *names[], size_t num_names) {
    std::string res = "names{ ";
    for (size_t i = 0; i < num_names; ++i) {
        if (names[i]) {
            res += fmt::format("'{:s}', ", names[i]);
        } else {
            res += "nullptr, ";
        }
    }
    res += " }";
    return res;
}

std::string get_addrs_str(void *addrs[], size_t num_names) {
    std::string res = "addrs{ ";
    for (size_t i = 0; i < num_names; ++i) {
        res += fmt::format("{}, ", fmt::ptr(addrs[i]));
    }
    res += " }";
    return res;
}

std::string get_flags_str(_dyld_pseudodylib_symbol_flags *flags) {
    std::string res = "flags{ ";
    if (flags) {
        if (*flags & DYLD_PSEUDODYLIB_SYMBOL_FLAGS_FOUND) {
            res += "DYLD_PSEUDODYLIB_SYMBOL_FLAGS_FOUND, ";
        }
        if (*flags & DYLD_PSEUDODYLIB_SYMBOL_FLAGS_WEAK_DEF) {
            res += "DYLD_PSEUDODYLIB_SYMBOL_FLAGS_WEAK_DEF, ";
        }
        if (*flags & DYLD_PSEUDODYLIB_SYMBOL_FLAGS_CALLABLE) {
            res += "DYLD_PSEUDODYLIB_SYMBOL_FLAGS_CALLABLE, ";
        }
        if (*flags == DYLD_PSEUDODYLIB_SYMBOL_FLAGS_NONE) {
            res += "DYLD_PSEUDODYLIB_SYMBOL_FLAGS_NONE";
        }
    } else {
        res += "nullptr";
    }
    res += " }";
    return res;
}

char *my_dyld_pseudodylib_lookup_symbols(void *pd_ctx, const void *mh, const char *names[], size_t num_names,
                                         void *addrs[], _dyld_pseudodylib_symbol_flags flags[]) {
    fmt::print("my_dyld_pseudodylib_lookup_symbols(pd_ctx = {}, mh = {}, names = {}, num_names = {}, addrs = {}, flags "
               "= {}) called with names: {:s} addrs: {:s} flags: {:s}\n",
               fmt::ptr(pd_ctx), fmt::ptr(mh), fmt::ptr(names), num_names, fmt::ptr(addrs), fmt::ptr(flags),
               get_names_str(names, num_names), get_addrs_str(addrs, num_names), get_flags_str(flags));

    fmt::print("my_dyld_pseudodylib_lookup_symbols(pd_ctx = {}, mh = {}, names = {}, num_names = {}, addrs = {}, flags "
               "= {}) returns with names: {:s} addrs: {:s} flags: {:s}\n",
               fmt::ptr(pd_ctx), fmt::ptr(mh), fmt::ptr(names), num_names, fmt::ptr(addrs), fmt::ptr(flags),
               get_names_str(names, num_names), get_addrs_str(addrs, num_names), get_flags_str(flags));
    return strdup("my_dyld_pseudodylib_lookup_symbols not implemented");
}

int my_dyld_pseudodylib_lookup_address(void *pd_ctx, const void *mh, const void *addr, struct dl_info *dl) {
    fmt::print("my_dyld_pseudodylib_deinitialize(pd_ctx = {}, mh = {}) called\n", fmt::ptr(pd_ctx), fmt::ptr(mh));
    return 0;
}

char *my_dyld_pseudodylib_find_unwind_sections(void *pd_ctx, const void *mh, const void *addr, bool *found,
                                               struct dyld_unwind_sections *info) {
    fmt::print("my_dyld_pseudodylib_find_unwind_sections(pd_ctx = {}, mh = {}, addr = {}, found = {} *found = {}, info "
               "= {}) called\n",
               fmt::ptr(pd_ctx), fmt::ptr(mh), fmt::ptr(addr), fmt::ptr(found),
               found ? (*found ? "true" : "false") : "n/a", fmt::ptr(info));
    return strdup("my_dyld_pseudodylib_find_unwind_sections not implemented");
}

struct pseudo_ctx {
    int dummy;
};

int main(void) {
    const uint64_t pd_base = (uintptr_t)&test_pseudodylib_simple_begin;
    const size_t pd_size   = (uintptr_t)&test_pseudodylib_simple_end - (uintptr_t)&test_pseudodylib_simple_begin;
    fmt::print("test_pseudodylib_simple_begin: {}\n", fmt::ptr(&test_pseudodylib_simple_begin));
    fmt::print("test_pseudodylib_simple_begin_inner: {}\n", fmt::ptr(&test_pseudodylib_simple_begin_inner));
    fmt::print("test_pseudodylib_simple_end_inner: {}\n", fmt::ptr(&test_pseudodylib_simple_end_inner));
    fmt::print("test_pseudodylib_simple_end: {}\n", fmt::ptr(&test_pseudodylib_simple_end));
    fmt::print("pd_size: {:#x}\n", pd_size);

    std::vector<uint8_t> macho_buf(reinterpret_cast<const uint8_t *>(&test_pseudodylib_simple_begin),
                                   reinterpret_cast<const uint8_t *>(&test_pseudodylib_simple_end));
    const auto fatbin = Parser::parse(macho_buf);
    if (!fatbin) {
        fmt::print("couldn't open pseudodylib macho as fat\n");
        return 1;
    }
    // fmt::print("\n\n\nfatbin:\n{}\n", *fatbin);
    // fmt::print("\n\n\n\n");
    const auto bin = fatbin->at(0);
    if (!bin) {
        fmt::print("couldn't open pseudodylib macho as slim fat subset\n");
        return 1;
    }
    fmt::print("\n\n\nbin:\n{}\n\n\n\n", *bin);

#if 1
    fmt::print("pd_base: {:#x}\n", pd_base);
    const auto magic_orig_pre = *reinterpret_cast<uint64_t *>(pd_base);
    fmt::print("pre remap:  magic_orig: {:#018x}\n", magic_orig_pre);

    pthread_jit_write_protect_np(false);

    const vm_address_t src_addr{static_cast<vm_address_t>(pd_base)};
    // vm_address_t new_addr{src_addr};
    vm_address_t new_addr{0x300000000ULL};
    // vm_prot_t cur_prot{VM_PROT_READ | VM_PROT_WRITE};
    // vm_prot_t max_prot{VM_PROT_READ | VM_PROT_WRITE};
    // vm_prot_t cur_prot{VM_PROT_NONE};
    // vm_prot_t max_prot{VM_PROT_NONE};
    vm_prot_t cur_prot{VM_PROT_NO_CHANGE};
    vm_prot_t max_prot{VM_PROT_NO_CHANGE};
    const vm_address_t mask{0};
    const auto self          = mach_task_self();
    const boolean_t anywhere = false;
    fmt::print("before new_addr: {:#x} cur_prot: {:#x} max_prot: {:#x}\n", new_addr, cur_prot, max_prot);
    const auto kret = vm_remap(self, &new_addr, vm_size_t{pd_size}, mask, anywhere, self, src_addr, 0, &cur_prot,
                               &max_prot, VM_INHERIT_DEFAULT);
    fmt::print("kret: {:d} errstr: {:s}\n", kret, mach_error_string(kret));
    fmt::print("after new_addr: {:#x} cur_prot: {:#x} max_prot: {:#x}\n", new_addr, cur_prot, max_prot);

    for (const ChainedBindingInfo &bind : bin->dyld_chained_fixups()->bindings()) {
        fmt::print("handling: {}\n", bind);
        std::string segment_name;
        std::string libname;
        std::string symbol;
        bool is_ok = true;
        if (const auto *segment = bind.segment()) {
            segment_name = segment->name();
        }

        if (const auto *lib = bind.library()) {
            libname = lib->name();
        } else {
            is_ok = false;
        }

        if (const auto *sym = bind.symbol()) {
            symbol = sym->name();
        } else {
            is_ok = false;
        }

        if (!is_ok) {
            fmt::print("not ok for: {}\n", bind);
            continue;
        }

        void *lib_handle = dlopen(libname.c_str(), RTLD_LOCAL);
        if (!lib_handle) {
            fmt::print("dlopen('{:s}', RTLD_LOCAL) failed\n", libname);
            continue;
        }
        void *sym_ptr = dlsym(lib_handle, symbol.substr(1).c_str());
        if (sym_ptr) {
            fmt::print("{:s} sym_ptr: {} in lib {:s}\n", symbol, fmt::ptr(sym_ptr), libname);
            fmt::print("bind addr: {:#x} off: {:#x}\n", bind.address(), bind.offset());
            uintptr_t *sym_ptr_addr = reinterpret_cast<uintptr_t *>(bind.address() + new_addr);
            fmt::print("sym_ptr_addr: {}\n", fmt::ptr(sym_ptr_addr));
            fmt::print("sym_ptr_addr before: {:#018x}\n", *sym_ptr_addr);
            *sym_ptr_addr = reinterpret_cast<uintptr_t>(sym_ptr);
            fmt::print("sym_ptr_addr after: {:#018x}\n", *sym_ptr_addr);
        }

        dlclose(lib_handle);
    }

    // *reinterpret_cast<uint64_t *>(new_addr) = *reinterpret_cast<uint64_t *>(new_addr) + 1;

    pthread_jit_write_protect_np(true);

    // *reinterpret_cast<uint64_t *>(new_addr) = *reinterpret_cast<uint64_t *>(new_addr) + 1;

    const auto magic_orig = *reinterpret_cast<uint64_t *>(pd_base);
    fmt::print("post remap: magic_orig: {:#018x}\n", magic_orig);
    const auto magic_remap = *reinterpret_cast<uint64_t *>(new_addr);
    fmt::print("post remap: magic_remap: {:#018x}\n", magic_remap);
#endif

    _dyld_pseudodylib_callbacks_v1 cb{1,
                                      my_dyld_pseudodylib_dispose_error_message,
                                      my_dyld_pseudodylib_initialize,
                                      my_dyld_pseudodylib_deinitialize,
                                      my_dyld_pseudodylib_lookup_symbols,
                                      my_dyld_pseudodylib_lookup_address,
                                      my_dyld_pseudodylib_find_unwind_sections};
    fmt::print("&cb: {}\n", fmt::ptr(&cb));
    _dyld_pseudodylib_callbacks_handle pd_cb_handle =
        _dyld_pseudodylib_register_callbacks(reinterpret_cast<_dyld_pseudodylib_callbacks *>(&cb));
    fmt::print("pd_cb_handle: {}\n", fmt::ptr(pd_cb_handle));
    pseudo_ctx ctx{.dummy = 243};
    fmt::print("&ctx: {}\n", fmt::ptr(&ctx));
    _dyld_pseudodylib_handle pd_handle = _dyld_pseudodylib_register(reinterpret_cast<void *>(new_addr), pd_size,
                                                                    pd_cb_handle, reinterpret_cast<void *>(&ctx));
    fmt::print("pd_handle: {}\n", fmt::ptr(pd_handle));

    fflush(stdout);
    fmt::print("calling dlopen()...\n");
    fflush(stdout);
    void *handle = dlopen("libtest-pseudodylib-simple.dylib", RTLD_GLOBAL);
    fflush(stdout);
    fmt::print("calling dlopen()... done\n");
    fflush(stdout);
    fmt::print("handle: {}\n", fmt::ptr(handle));

    const auto dbl_fptr = reinterpret_cast<dbl_fptr_t>(dlsym(handle, "dbl"));
    fmt::print("dbl_fptr: {}\n", fmt::ptr(dbl_fptr));
    const int dbl_arg  = 42;
    const auto dbl_res = dbl_fptr(dbl_arg);
    fmt::print("dbl({:d}) => {:d}\n", dbl_arg, dbl_res);

#ifdef PD_EXTERN_SYMS
    const auto fact_fptr = reinterpret_cast<fact_fptr_t>(dlsym(handle, "fact"));
    fmt::print("fact_fptr: {}\n", fmt::ptr(fact_fptr));
    const int fact_arg  = 5;
    const auto fact_res = fact_fptr(fact_arg);
    fmt::print("fact({:d}) => {:d}\n", fact_arg, fact_res);
#endif

    fflush(stdout);
    fmt::print("calling dlclose(handle)...\n");
    fflush(stdout);
    dlclose(handle);
    fflush(stdout);
    fmt::print("calling dlclose(handle)... done\n");
    fflush(stdout);

    fflush(stdout);
    fmt::print("calling _dyld_pseudodylib_deregister(pd_handle)...\n");
    fflush(stdout);
    _dyld_pseudodylib_deregister(pd_handle);
    fflush(stdout);
    fmt::print("calling _dyld_pseudodylib_deregister(pd_handle)... done\n");
    fflush(stdout);

    fflush(stdout);
    fmt::print("calling _dyld_pseudodylib_deregister_callbacks(pd_cb_handle)...\n");
    fflush(stdout);
    _dyld_pseudodylib_deregister_callbacks(pd_cb_handle);
    fflush(stdout);
    fmt::print("calling _dyld_pseudodylib_deregister_callbacks(pd_cb_handle)... done\n");
    fflush(stdout);

    fflush(stdout);
    fmt::print("test done\n");
    fflush(stdout);

    return 0;
}
