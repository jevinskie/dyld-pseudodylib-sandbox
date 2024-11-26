add_executable(test-pseudodylib-simple-util test-pseudodylib-simple-util.cpp embed-test-pseudodylib-simple.S)

target_link_libraries(test-pseudodylib-simple-util PRIVATE test-pseudodylib-shared argparse fmt LIEF::LIEF)
target_compile_options(test-pseudodylib-simple-util PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable)

install(TARGETS test-pseudodylib-simple-util
    RUNTIME DESTINATION bin
)
