// Test_fuzzer.cc
#include <stdint.h>
#include <stddef.h>
#include <string_view>
#include <harbour/forms.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    Request::encode(std::string_view(reinterpret_cast<const char *>(data), size));

    return 0;
}