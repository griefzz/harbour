macro(hb_add_fuzz target)
    add_executable(fuzz_${target} ${target}.cpp)
    target_link_libraries(fuzz_${target} PUBLIC harbour)
    target_compile_features(fuzz_${target} PRIVATE cxx_std_20)

    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        target_compile_options(fuzz_${target} PRIVATE -fsanitize=fuzzer,undefined,address)
        target_link_options(fuzz_${target} PRIVATE -coverage -fsanitize=fuzzer,undefined,address)
    else()
        message(WARNING "Fuzz testing is not supported on this compiler!")
    endif()
endmacro()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    message(STATUS "Harbour fuzz testing only works on linux at the moment")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    hb_add_fuzz(requests)
    hb_add_fuzz(cookies)
    hb_add_fuzz(formdata)
endif()
