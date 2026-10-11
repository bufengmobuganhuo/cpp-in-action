//
// Created by yuzhang on 2026/10/7.
//

#pragma once

#include <string>
#include <json/json.h>

namespace vo
{
    struct TransactionStatisticsVo
    {
        std::string symbol_;
        std::string total_invest_times_;
        std::string total_invest_amount_;
        std::string total_invest_qty_;
        std::string total_invest_commission_;
        std::string next_target_value_;

        static std::string strip_trailing_zeros(std::string value)
        {
            const auto dot_pos = value.find('.');
            if (dot_pos == std::string::npos)
            {
                return value;
            }

            while (!value.empty() && value.back() == '0')
            {
                value.pop_back();
            }
            if (!value.empty() && value.back() == '.')
            {
                value.pop_back();
            }
            return value.empty() ? "0" : value;
        }

        Json::Value to_json() const
        {
            Json::Value root;
            root["totalInvestTimes"] = total_invest_times_;
            root["totalInvestAmount"] = strip_trailing_zeros(total_invest_amount_);
            root["totalInvestQty"] = strip_trailing_zeros(total_invest_qty_);
            root["totalInvestCommission"] = strip_trailing_zeros(total_invest_commission_);
            root["nextTargetValue"] = strip_trailing_zeros(next_target_value_);
            return root;
        }
    };
}
