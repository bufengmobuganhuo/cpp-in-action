//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <optional>
#include <regex>
#include <string>
#include <drogon/HttpRequest.h>

namespace dto
{
    struct TransactionPlanDto
    {
        std::string symbol_;
        std::string amount_;
        std::string frequency_;
        std::optional<int> week_day_;
        std::optional<int> week_order_;
        std::string remind_time_;
        std::string start_date_;
        bool enabled_{false};

        std::optional<std::string> validate() const
        {
            if (!amount_.empty())
            {
                static const std::regex amount_regex(R"(^-?\d{1,10}(\.\d{1,5})?$)");
                if (!std::regex_match(amount_, amount_regex))
                {
                    return "定投金额不合法";
                }
            }

            return std::nullopt;
        }
    };
}

namespace drogon
{
    template<>
    inline dto::TransactionPlanDto fromRequest(const HttpRequest& req)
    {
        std::shared_ptr<Json::Value> json = req.getJsonObject();
        dto::TransactionPlanDto transaction_plan_dto;
        if (json)
        {
            if (json->isMember("symbol") && !(*json)["symbol"].isNull())
            {
                transaction_plan_dto.symbol_ = (*json)["symbol"].asString();
            }
            if (json->isMember("amount") && !(*json)["amount"].isNull())
            {
                transaction_plan_dto.amount_ = (*json)["amount"].asString();
            }
            if (json->isMember("frequency") && !(*json)["frequency"].isNull())
            {
                transaction_plan_dto.frequency_ = (*json)["frequency"].asString();
            }
            if (json->isMember("weekDay") && (*json)["weekDay"].isInt())
            {
                transaction_plan_dto.week_day_ = (*json)["weekDay"].asInt();
            }
            if (json->isMember("weekOrder") && (*json)["weekOrder"].isInt())
            {
                transaction_plan_dto.week_order_ = (*json)["weekOrder"].asInt();
            }
            if (json->isMember("remindTime") && !(*json)["remindTime"].isNull())
            {
                transaction_plan_dto.remind_time_ = (*json)["remindTime"].asString();
            }
            if (json->isMember("startDate") && !(*json)["startDate"].isNull())
            {
                transaction_plan_dto.start_date_ = (*json)["startDate"].asString();
            }
            if (json->isMember("enabled") && (*json)["enabled"].isBool())
            {
                transaction_plan_dto.enabled_ = (*json)["enabled"].asBool();
            }
        }
        return transaction_plan_dto;
    }
}
