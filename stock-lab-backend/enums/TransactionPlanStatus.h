//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <array>
#include <string_view>
#include <optional>
#include <string>

namespace enums
{
    enum TransactionPlanStatus
    {
        ENABLED,
        PAUSED,
        DELETED,
        DISABLED
    };

    inline constexpr std::optional<TransactionPlanStatus> parse(const std::string& name)
    {
        if (name == "ENABLED")
        {
            return ENABLED;
        }
        if (name == "PAUSED")
        {
            return PAUSED;
        }
        if (name == "DELETED")
        {
            return DELETED;
        }
        if (name == "DISABLED")
        {
            return DISABLED;
        }
        return std::nullopt;
    }

    inline constexpr std::string_view name(TransactionPlanStatus status)
    {
        switch (status)
        {
        case ENABLED:
            return "ENABLED";
        case PAUSED:
            return  "PAUSED";
        case DELETED:
            return "DELETED";
        default:
        case DISABLED:
            return "DISABLED";
        }
    }

    inline constexpr std::array<std::string_view, 3> DISPLAYABLE_STATUS{
        "ENABLED", "PAUSED", "DISABLED"
    };

    inline constexpr std::array<std::string_view, 3> DISABLED_STATUS{
        "DELETED", "DISABLED", "PAUSED"
    };
}
