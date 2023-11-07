enable_testing()

macro(add_test_executable TARGET_NAME EXTRA_SOURCES)
    # Add the test executable and with servers sources
    add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp ${EXTRA_SOURCES})

    if(WIN32)
        target_link_libraries(${TARGET_NAME} wsock32 ws2_32)
    endif()

    # Set C++ standard
    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_23)

    # Add src directory
    target_include_directories(${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

    # Add test
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endmacro()

# Router test
add_test_executable(test_router ${PROJECT_SOURCE_DIR}/src/server.cpp)
