#include "test-pseudodylib-simple/test-pseudodylib-simple.h"

#include <stdio.h>

__attribute__((constructor, visibility("default"), used)) void ctor(void) {
    printf("pseudo ctor\n");
}

__attribute__((visibility("default"), used)) int fact(int n) {
    printf("fact(%d) called\n", n);
    return 41;
}

__attribute__((destructor, visibility("default"), used)) void dtor(void) {
    printf("pseudo dtor\n");
    return;
}
