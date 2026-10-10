//
// Created by yuzhang on 2026/10/8.
//

#pragma once
#include <optional>
#include <string>

namespace enums
{
    enum TransactionPlanFrequency
    {
        DAILY,
        WEEKLY,
        MONTHLY
    };

    inline constexpr std::optional<TransactionPlanFrequency> parse_frequency(const std::string& name)
    {
        if (name == "DAILY")
        {
            return DAILY;
        }
        if (name == "WEEKLY")
        {
            return WEEKLY;
        }
        if (name == "MONTHLY")
        {
            return MONTHLY;
        }
        return std::nullopt;
    }
};
