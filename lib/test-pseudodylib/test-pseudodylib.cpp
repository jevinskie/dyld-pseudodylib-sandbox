#include "test-pseudodylib/test-pseudodylib.hpp"
#include "common-internal.hpp"
#include "test-pseudodylib/utils.hpp"

#undef NDEBUG
#include <cstdio>

#include <icecream/icecream.hpp>

namespace TestPeudodylib {

uint64_t Factorial::fact(uint8_t n) {
    uint32_t nfast = n;
    if (TEST_PSEUDODYLIB_UNLIKELY(nfast > 20)) {
        // fmt::print(stderr, FMT_COMPILE("Sorry, uint64_t cant hold {:d}!, returning
        // UINT64_MAX\n"),
        //            nfast);
        fprintf(stderr, "Sorry, uint64_t can't hold %" PRIu32 "!, returning UINT64_MAX\n", nfast);
        return UINT64_MAX;
    }
    uint64_t res = 1;
    while (nfast > 1)
        res *= nfast--;
    return res;
}

} // namespace TestPeudodylib
