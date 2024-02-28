#include "test-pseudodylib/utils.hpp"
#include "common-internal.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace TestPeudodylib {

void posix_check(int retval, const std::string &msg) {
    const auto orig_errno = errno;
    if (TEST_PSEUDODYLIB_UNLIKELY(retval < 0)) {
        throw std::runtime_error(
            fmt::format("POSIX error: '{:s}' retval: {:d} errno: {:d} description: '{:s}'", msg,
                        retval, orig_errno, strerror(orig_errno)));
    }
}

} // namespace TestPeudodylib
