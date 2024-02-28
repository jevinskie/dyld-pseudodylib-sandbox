#include "test-pseudodylib/test-pseudodylib.hpp"
#include "common-internal.hpp"
#include "test-pseudodylib/utils.hpp"

#undef NDEBUG
#include <cstdio>

#include <icecream/icecream.hpp>

namespace TestPeudodylib {

uint64_t Factorial::fact(uint8_t n) {
    if (n > 20) {
        fmt::print(stderr, "Sorry, uint64_t cant hold {:d}!, returning UINT64_MAX\n",
                   static_cast<uint8_t>(n));
        return UINT64_MAX;
    }
    uint64_t res = 1;
    while (n > 1)
        res *= n--;
    return res;
}

} // namespace TestPeudodylib
