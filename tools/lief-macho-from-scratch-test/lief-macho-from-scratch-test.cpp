#include <test-pseudodylib/common.hpp>

#include <cstdio>

class nested {
public:
    TEST_PSEUDODYLIB_EXPORT nested() = default;
    int aa;
};

struct dumpme {
    int a;
    int b;
    nested n;
};

int main(void) {
    nested n;
    // n.aa = 12;
    dumpme d;
    d.a = 1;
    d.b = 2;
    d.n = n;
    __builtin_dump_struct(&d, &printf);
    return 0;
}
