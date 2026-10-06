//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <iomanip>
#include <sstream>
#include <boost/cstdint.hpp>

namespace util
{
    using Decimal = boost::multiprecision::cpp_dec_float_50;

    static std::string format_decimal(const Decimal& value, int scale = 5)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(scale) << value;
        return oss.str();
    }

}
