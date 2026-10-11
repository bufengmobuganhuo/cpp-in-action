//
// Created by yuzhang on 2026/10/11.
//

#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace enums
{
    enum OrderAction
    {
        BUY,
        SELL
    };

    inline constexpr std::optional<OrderAction> parse_order_action(const std::string_view name)
    {
        if (name == "BUY")
        {
            return BUY;
        }
        if (name == "SELL")
        {
            return SELL;
        }
        return std::nullopt;
    }

    inline constexpr std::string_view name(const OrderAction action)
    {
        switch (action)
        {
        case BUY:
            return "BUY";
        case SELL:
        default:
            return "SELL";
        }
    }
}
