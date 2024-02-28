#pragma once

#include "common.hpp"

#include <stdexcept>

namespace TestPeudodylib {

void posix_check(int retval, const std::string &msg);

}
