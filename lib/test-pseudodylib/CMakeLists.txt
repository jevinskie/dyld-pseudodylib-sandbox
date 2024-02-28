set(TEST_PSEUDODYLIB_SRC
    test-pseudodylib.cpp
    utils.cpp
)

set(TEST_PSEUDODYLIB_HDR_STANDALONE
    common.hpp
    common-attrs.hpp
)

set(TEST_PSEUDODYLIB_PRIVATE_HDR_STANDALONE
    common-internal.hpp
)

set(TEST_PSEUDODYLIB_HDR)
foreach(HDR ${TEST_PSEUDODYLIB_HDR_STANDALONE})
    set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/test-pseudodylib/${HDR}")
    list(APPEND TEST_PSEUDODYLIB_HDR ${HDR})
endforeach()

foreach(SRC ${TEST_PSEUDODYLIB_SRC})
    get_filename_component(HDR_NAME ${SRC} NAME_WLE)
    get_filename_component(SRC_EXT ${SRC} EXT)
    if("${SRC_EXT}" STREQUAL ".c")
       set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/test-pseudodylib/${HDR_NAME}.h")
    elseif("${SRC_EXT}" STREQUAL ".cpp")
        set(HDR "${CMAKE_CURRENT_SOURCE_DIR}/../../include/test-pseudodylib/${HDR_NAME}.hpp")
    else()
        message(FATAL_ERROR "Unknown source extension '${SRC_EXT}' for source file '${SRC}'")
    endif()
    if(EXISTS ${HDR})
        list(APPEND TEST_PSEUDODYLIB_HDR ${HDR})
    endif()
endforeach()

set(TEST_PSEUDODYLIB_PUBLIC_LIBS
    icecream
)

set(TEST_PSEUDODYLIB_PRIVATE_LIBS
    fmt
)

# TEST_PSEUDODYLIB_HDR/TEST_PSEUDODYLIB_PRIVATE_HDR_STANDALONE added for Xcode project generation
add_library(test-pseudodylib STATIC ${TEST_PSEUDODYLIB_SRC} ${TEST_PSEUDODYLIB_HDR} ${TEST_PSEUDODYLIB_PRIVATE_HDR_STANDALONE})
add_library(test-pseudodylib-shared SHARED ${TEST_PSEUDODYLIB_SRC} ${TEST_PSEUDODYLIB_HDR} ${TEST_PSEUDODYLIB_PRIVATE_HDR_STANDALONE})
set_target_properties(test-pseudodylib test-pseudodylib-shared PROPERTIES PUBLIC_HEADER "${TEST_PSEUDODYLIB_HDR}")

target_link_libraries(test-pseudodylib
PUBLIC
    ${TEST_PSEUDODYLIB_PUBLIC_LIBS}
PRIVATE
    ${TEST_PSEUDODYLIB_PRIVATE_LIBS}
)
target_compile_options(test-pseudodylib PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(test-pseudodylib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test-pseudodylib-shared
PUBLIC
    ${TEST_PSEUDODYLIB_PUBLIC_LIBS}
PRIVATE
    ${TEST_PSEUDODYLIB_PRIVATE_LIBS}
)
target_compile_options(test-pseudodylib-shared PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(test-pseudodylib-shared
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)


install(TARGETS test-pseudodylib test-pseudodylib-shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/test-pseudodylib
)

# target_compile_options(test-pseudodylib PRIVATE -Xclang -fdump-record-layouts)
