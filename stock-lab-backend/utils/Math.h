//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <boost/cstdint.hpp>
#include <trantor/utils/Logger.h>

namespace util
{
    using Decimal = boost::multiprecision::cpp_dec_float_50;

    static std::string format_decimal(const Decimal& value, int scale = 5)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(scale) << value;
        return oss.str();
    }

    static std::optional<int> parse_int(const std::string& value)
    {
        if (value.empty())
        {
            return std::nullopt;
        }

        try
        {
            return std::stoi(value);
        }
        catch (...)
        {
            LOG_ERROR << "failed to parse_int64, value=" << value;
            return std::nullopt;
        }
    }

    static std::optional<int64_t> parse_int64(const std::string& value)
    {
        if (value.empty())
        {
            return std::nullopt;
        }

        try
        {
            return std::stoll(value);
        }
        catch (...)
        {
            LOG_ERROR << "failed to parse_int64, value=" << value;
            return std::nullopt;
        }
    }
}
