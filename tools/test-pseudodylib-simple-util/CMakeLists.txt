add_executable(test-pseudodylib-simple-util test-pseudodylib-simple-util.cpp embed-test-pseudodylib-simple.S)

add_dependencies(test-pseudodylib-simple-util test-pseudodylib-shared)
target_link_libraries(test-pseudodylib-simple-util PRIVATE argparse fmt LIEF::LIEF)
target_include_directories(test-pseudodylib-simple-util PRIVATE $<TARGET_PROPERTY:test-pseudodylib-shared,INCLUDE_DIRECTORIES>)
target_compile_options(test-pseudodylib-simple-util PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable)
target_link_options(test-pseudodylib-simple-util PRIVATE -Wl,-segprot,__PDYLIB,rwx,rwx)

install(TARGETS test-pseudodylib-simple-util
    RUNTIME DESTINATION bin
)
