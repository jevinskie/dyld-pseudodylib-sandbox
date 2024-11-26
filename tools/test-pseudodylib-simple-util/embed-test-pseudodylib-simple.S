    .section __PDYLIB,__macho,regular
    .global _test_pseudodylib_simple_begin
_test_pseudodylib_simple_begin:
    .p2align 14
    .global _test_pseudodylib_simple_begin_inner
_test_pseudodylib_simple_begin_inner:
    .incbin "lib/test-pseudodylib-simple/libtest-pseudodylib-simple-shared.dylib"
    .global _test_pseudodylib_simple_end_inner
_test_pseudodylib_simple_end_inner:
    .p2align 14
    .global _test_pseudodylib_simple_end
_test_pseudodylib_simple_end:
