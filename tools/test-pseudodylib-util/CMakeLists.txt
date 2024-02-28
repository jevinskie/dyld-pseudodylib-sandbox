add_executable(test-pseudodylib-util test-pseudodylib-util.cpp)

target_link_libraries(test-pseudodylib-util PRIVATE test-pseudodylib-shared argparse fmt)
target_compile_options(test-pseudodylib-util PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS test-pseudodylib-util
    RUNTIME DESTINATION bin
)
