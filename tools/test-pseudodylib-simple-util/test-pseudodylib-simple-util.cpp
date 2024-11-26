#include "test-pseudodylib-simple/test-pseudodylib-simple.h"
#include <cstring>
#include <sys/_types/_uintptr_t.h>

#undef NDEBUG
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <dlfcn.h>
#include <sys/mman.h>

#include <vector>

#include <LIEF/MachO.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace LIEF::MachO;

extern "C" {
extern void *test_pseudodylib_simple_begin;
extern void *test_pseudodylib_simple_end;

// Symbol flags type for symbols defined via the pseudo-dylibs APIs.
typedef uint64_t _dyld_pseudodylib_symbol_flags;

// Flag values for _dyld_pseudodylib_symbol_flags.
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_NONE 0
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_FOUND 1
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_WEAK_DEF 2
#define DYLD_PSEUDODYLIB_SYMBOL_FLAGS_CALLABLE 4

typedef void (*_dyld_pseudodylib_dispose_error_message)(char *err_msg);
typedef char *(*_dyld_pseudodylib_initialize)(void *pd_ctx, const void *mh);
typedef char *(*_dyld_pseudodylib_deinitialize)(void *pd_ctx, const void *mh);
typedef char *(*_dyld_pseudodylib_lookup_symbols)(void *pd_ctx, const void *mh, const char *names[],
                                                  size_t num_names, void *addrs[],
                                                  _dyld_pseudodylib_symbol_flags flags[]);
typedef int (*_dyld_pseudodylib_lookup_address)(void *pd_ctx, const void *mh, const void *addr,
                                                struct dl_info *dl);
typedef char *(*_dyld_pseudodylib_find_unwind_sections)(void *pd_ctx, const void *mh,
                                                        const void *addr, bool *found,
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
extern void
_dyld_pseudodylib_deregister_callbacks(_dyld_pseudodylib_callbacks_handle callbacks_handle);
extern _dyld_pseudodylib_handle
_dyld_pseudodylib_register(void *addr, size_t size,
                           _dyld_pseudodylib_callbacks_handle callbacks_handle, void *context);
extern void _dyld_pseudodylib_deregister(_dyld_pseudodylib_handle pd_handle);
}

template <> struct fmt::formatter<Binary> : fmt::ostream_formatter {};
template <> struct fmt::formatter<FatBinary> : fmt::ostream_formatter {};

void my_dyld_pseudodylib_dispose_error_message(char *err_msg) {
    assert(!__PRETTY_FUNCTION__);
}
char *my_dyld_pseudodylib_initialize(void *pd_ctx, const void *mh) {
    assert(!__PRETTY_FUNCTION__);
}
char *my_dyld_pseudodylib_deinitialize(void *pd_ctx, const void *mh) {
    assert(!__PRETTY_FUNCTION__);
}
char *my_dyld_pseudodylib_lookup_symbols(void *pd_ctx, const void *mh, const char *names[],
                                         size_t num_names, void *addrs[],
                                         _dyld_pseudodylib_symbol_flags flags[]) {
    assert(!__PRETTY_FUNCTION__);
}
int my_dyld_pseudodylib_lookup_address(void *pd_ctx, const void *mh, const void *addr,
                                       struct dl_info *dl) {
    assert(!__PRETTY_FUNCTION__);
}
char *my_dyld_pseudodylib_find_unwind_sections(void *pd_ctx, const void *mh, const void *addr,
                                               bool *found, struct dyld_unwind_sections *info) {
    assert(!__PRETTY_FUNCTION__);
}

struct pseudo_ctx {
    int dummy;
};

int main(void) {
    fmt::print("test_pseudodylib_simple_begin: {}\n", fmt::ptr(&test_pseudodylib_simple_begin));
    fmt::print("test_pseudodylib_simple_end: {}\n", fmt::ptr(&test_pseudodylib_simple_end));
    const size_t pd_size = (uintptr_t)&test_pseudodylib_simple_end - (uintptr_t)&test_pseudodylib_simple_begin;
    const auto mprotect_res = mprotect(reinterpret_cast<void *>(&test_pseudodylib_simple_begin), pd_size, PROT_READ | MAP_JIT);
    if (mprotect_res) {
        const auto dle = dlerror();
        fmt::print("mprotect failed: {:d} '{:s}' '{:s}'\n", mprotect_res, strerror(errno), dle ? dle : "");
        return 2;
    }
    std::vector<uint8_t> macho_buf(
        reinterpret_cast<const uint8_t *>(&test_pseudodylib_simple_begin),
        reinterpret_cast<const uint8_t *>(&test_pseudodylib_simple_end));
    const auto bin = Parser::parse(macho_buf);
    if (!bin) {
        fmt::print("couldn't open pseudodylib macho\n");
        return 1;
    }
    fmt::print("bin:\n{}\n", *bin);
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
    _dyld_pseudodylib_handle pd_handle = _dyld_pseudodylib_register(
        &test_pseudodylib_simple_begin,
        (uintptr_t)&test_pseudodylib_simple_end - (uintptr_t)&test_pseudodylib_simple_begin,
        pd_cb_handle, reinterpret_cast<void *>(&ctx));
    fmt::print("pd_handle: {}\n", fmt::ptr(pd_handle));
    void *handle = dlopen("libtest-pseudodylib-simple.dylib", RTLD_GLOBAL);
    fmt::print("handle: {}\n", fmt::ptr(handle));
    _dyld_pseudodylib_deregister(pd_handle);
    return 0;
}
