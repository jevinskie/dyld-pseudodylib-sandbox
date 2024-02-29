add_executable(dylib-to-obj dylib-to-obj.cpp)

target_link_libraries(dylib-to-obj PRIVATE argparse fmt LIEF::LIEF icecream)
target_compile_options(dylib-to-obj PRIVATE -Wall -Wextra -Wpedantic)

install(TARGETS dylib-to-obj
    RUNTIME DESTINATION bin
)
