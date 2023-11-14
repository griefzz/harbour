#pragma once
#include <string>

namespace base64 {
    static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

    bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string encode(const std::string &input) {
        std::string encoded;
        unsigned char input3[3];
        unsigned char output4[4];
        size_t i = 0;

        for (auto &ch: input) {
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

    std::string decode(const std::string &input) {
        std::string decoded;
        size_t in_len = input.size();
        int i         = 0;
        int j         = 0;
        int in_       = 0;
        unsigned char input4[4], output3[3];

        while (in_len-- && (input[in_] != '=') && is_base64(input[in_])) {
            input4[i++] = input[in_];
            in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++) {
                    input4[i] = static_cast<unsigned char>(base64_chars.find(input4[i]));
                }

                output3[0] = (input4[0] << 2) + ((input4[1] & 0x30) >> 4);
                output3[1] = ((input4[1] & 0x0F) << 4) + ((input4[2] & 0x3C) >> 2);
                output3[2] = ((input4[2] & 0x03) << 6) + input4[3];

                for (i = 0; i < 3; i++) {
                    decoded += static_cast<char>(output3[i]);
                }

                i = 0;
            }
        }

        if (i > 0) {
            for (j = i; j < 4; j++) {
                input4[j] = 0;
            }

            for (j = 0; j < 4; j++) {
                input4[j] = static_cast<unsigned char>(base64_chars.find(input4[j]));
            }

            output3[0] = (input4[0] << 2) + ((input4[1] & 0x30) >> 4);
            output3[1] = ((input4[1] & 0x0F) << 4) + ((input4[2] & 0x3C) >> 2);
            output3[2] = ((input4[2] & 0x03) << 6) + input4[3];

            for (j = 0; j < i - 1; j++) {
                decoded += static_cast<char>(output3[j]);
            }
        }

        return decoded;
    }
};// namespace base64