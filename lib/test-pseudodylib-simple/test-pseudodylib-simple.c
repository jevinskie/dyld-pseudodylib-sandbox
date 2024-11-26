#include "test-pseudodylib-simple/test-pseudodylib-simple.h"

#if defined(PD_CTOR_DTOR) || defined(PD_EXTERN_SYMS)
#include <stdio.h>
#endif

#ifdef PD_CTOR_DTOR
__attribute__((constructor, visibility("default"), used)) void ctor(void) {
    printf("pseudo ctor\n");
}
#endif

#ifdef PD_EXTERN_SYMS
__attribute__((visibility("default"), used)) int fact(int n) {
    printf("fact(%d) called\n", n);
    return 41;
}
#endif

__attribute__((visibility("default"), used)) int dbl(int n) {
    return n * 2;
}

#ifdef PD_CTOR_DTOR
__attribute__((destructor, visibility("default"), used)) void dtor(void) {
    printf("pseudo dtor\n");
    return;
}
#endif
