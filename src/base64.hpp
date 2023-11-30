#pragma once
#include <ranges>
#include <string>
#include <array>

struct Base64 {
    static auto encode(const std::string &input) noexcept -> std::string {
        const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";
        std::string encoded;
        std::array<unsigned char, 3> input3{};
        std::array<unsigned char, 4> output4{};
        size_t i = 0;

        for (const auto &ch: input) {
            input3[i++] = static_cast<unsigned char>(ch);

            if (i == 3) {
                output4[0] = (input3[0] & 0xFC) >> 2;
                output4[1] = ((input3[0] & 0x03) << 4) + ((input3[1] & 0xF0) >> 4);
                output4[2] = ((input3[1] & 0x0F) << 2) + ((input3[2] & 0xC0) >> 6);
                output4[3] = input3[2] & 0x3F;

                for (i = 0; i < 4; i++) {
                    encoded += base64_chars[output4[i]];
                }

                i = 0;
            }
        }

        if (i > 0) {
            for (size_t j = i; j < 3; j++) {
                input3[j] = 0;
            }

            output4[0] = (input3[0] & 0xFC) >> 2;
            output4[1] = ((input3[0] & 0x03) << 4) + ((input3[1] & 0xF0) >> 4);
            output4[2] = ((input3[1] & 0x0F) << 2) + ((input3[2] & 0xC0) >> 6);
            output4[3] = input3[2] & 0x3F;

            for (size_t j = 0; j < i + 1; j++) {
                encoded += base64_chars[output4[j]];
            }

            while (i++ < 3) {
                encoded += '=';
            }
        }

        return encoded;
    }
};// namespace base64