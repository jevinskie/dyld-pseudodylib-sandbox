    .section __PDYLIB,__macho,regular
    .global _test_pseudodylib_simple_begin
    .p2align 14
_test_pseudodylib_simple_begin:
    .incbin "lib/test-pseudodylib-simple/libtest-pseudodylib-simple-shared.dylib"
    .global _test_pseudodylib_simple_end
    .p2align 14
_test_pseudodylib_simple_end:
