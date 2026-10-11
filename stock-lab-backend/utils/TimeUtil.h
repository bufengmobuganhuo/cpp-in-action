//
// Created by yuzhang on 2026/10/11.
//

#pragma once

#include <ctime>
#include <optional>
#include <string>

#include <trantor/utils/Date.h>

namespace util
{
    inline std::optional<trantor::Date> parse_datetime(const std::string& value)
    {
        if (value.empty())
        {
            return std::nullopt;
        }

        std::tm tm{};
        tm.tm_isdst = -1;
        char* end = strptime(value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
        if (end == nullptr || *end != '\0')
        {
            return std::nullopt;
        }

        return trantor::Date(mktime(&tm) * 1000000);
    }
}
