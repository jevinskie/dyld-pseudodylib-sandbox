add_executable(lief-macho-from-scratch-test lief-macho-from-scratch-test.cpp)

target_link_libraries(lief-macho-from-scratch-test PRIVATE argparse fmt LIEF::LIEF icecream test-pseudodylib)
target_compile_options(lief-macho-from-scratch-test PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(lief-macho-from-scratch-test PRIVATE -fuse-ld=lld -save-temps -v -Wl,-v,-save-temps,-save-temps-asm)
target_compile_options(lief-macho-from-scratch-test PRIVATE -save-temps)


install(TARGETS lief-macho-from-scratch-test
    RUNTIME DESTINATION bin
)
