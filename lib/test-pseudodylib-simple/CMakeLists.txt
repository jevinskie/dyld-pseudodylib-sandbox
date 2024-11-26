set(TEST_PSEUDODYLIB_SIMPLE_SRC
    test-pseudodylib-simple.c
)

set(TEST_PSEUDODYLIB_SIMPLE_HDR_STANDALONE
)

set(TEST_PSEUDODYLIB_SIMPLE_PRIVATE_HDR_STANDALONE
)

set(TEST_PSEUDODYLIB_SIMPLE_HDR)
foreach(HDR ${TEST_PSEUDODYLIB_SIMPLE_HDR_STANDALONE})
    set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/test-pseudodylib-simple/${HDR}")
    list(APPEND TEST_PSEUDODYLIB_SIMPLE_HDR ${HDR})
endforeach()

foreach(SRC ${TEST_PSEUDODYLIB_SIMPLE_SRC})
    get_filename_component(HDR_NAME ${SRC} NAME_WLE)
    get_filename_component(SRC_EXT ${SRC} EXT)
    if("${SRC_EXT}" STREQUAL ".c")
       set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/test-pseudodylib-simple/${HDR_NAME}.h")
    elseif("${SRC_EXT}" STREQUAL ".cpp")
        set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/test-pseudodylib-simple/${HDR_NAME}.hpp")
    else()
        message(FATAL_ERROR "Unknown source extension '${SRC_EXT}' for source file '${SRC}'")
    endif()
    if(EXISTS ${HDR})
        list(APPEND TEST_PSEUDODYLIB_SIMPLE_HDR ${HDR})
    endif()
endforeach()

set(TEST_PSEUDODYLIB_SIMPLE_PUBLIC_LIBS
    # icecream
)

set(TEST_PSEUDODYLIB_SIMPLE_PRIVATE_LIBS
    # fmt
)

# TEST_PSEUDODYLIB_SIMPLE_HDR/TEST_PSEUDODYLIB_SIMPLE_PRIVATE_HDR_STANDALONE added for Xcode project generation
add_library(test-pseudodylib-simple STATIC ${TEST_PSEUDODYLIB_SIMPLE_SRC} ${TEST_PSEUDODYLIB_SIMPLE_HDR} ${TEST_PSEUDODYLIB_SIMPLE_PRIVATE_HDR_STANDALONE})
add_library(test-pseudodylib-simple-shared SHARED ${TEST_PSEUDODYLIB_SIMPLE_SRC} ${TEST_PSEUDODYLIB_SIMPLE_HDR} ${TEST_PSEUDODYLIB_SIMPLE_PRIVATE_HDR_STANDALONE})
set_target_properties(test-pseudodylib-simple test-pseudodylib-simple-shared PROPERTIES PUBLIC_HEADER "${TEST_PSEUDODYLIB_SIMPLE_HDR}")

target_link_libraries(test-pseudodylib-simple
PUBLIC
    ${TEST_PSEUDODYLIB_SIMPLE_PUBLIC_LIBS}
PRIVATE
    ${TEST_PSEUDODYLIB_SIMPLE_PRIVATE_LIBS}
)
target_compile_options(test-pseudodylib-simple PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(test-pseudodylib-simple
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test-pseudodylib-simple-shared
PUBLIC
    ${TEST_PSEUDODYLIB_SIMPLE_PUBLIC_LIBS}
PRIVATE
    ${TEST_PSEUDODYLIB_SIMPLE_PRIVATE_LIBS}
)
target_compile_options(test-pseudodylib-simple-shared PRIVATE -Wall -Wextra -Wpedantic)
target_link_options(test-pseudodylib-simple-shared PRIVATE -Wl,-install_name,libtest-pseudodylib-simple.dylib)

target_include_directories(test-pseudodylib-simple-shared
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)


install(TARGETS test-pseudodylib-simple test-pseudodylib-simple-shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/test-pseudodylib-simple
)

# target_compile_options(test-pseudodylib-simple PRIVATE -Xclang -fdump-record-layouts)
