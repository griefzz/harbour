set(OPENSSL_USE_STATIC_LIBS TRUE)

if(WIN32 AND NOT HARBOUR_SKIP_AUTOMATE_VCPKG)
    include(cmake/vcpkg.cmake)
    vcpkg_bootstrap()
    set(VCPKG_TARGET_TRIPLET "x64-windows-static")
    vcpkg_install_packages(openssl:x64-windows-static)
endif()

find_package(OpenSSL REQUIRED QUIET)
target_link_libraries(harbour INTERFACE OpenSSL::SSL OpenSSL::Crypto)
