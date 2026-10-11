//
// Created by yuzhang on 2026/10/11.
//

#pragma once

#include <cstddef>
#include <string>

namespace util
{
    inline std::size_t utf8_length(const std::string& text)
    {
        std::size_t length = 0;
        for (const unsigned char ch : text)
        {
            if ((ch & 0xC0) != 0x80)
            {
                ++length;
            }
        }
        return length;
    }
}
