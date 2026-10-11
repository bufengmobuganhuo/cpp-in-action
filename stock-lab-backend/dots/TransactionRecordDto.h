//
// Created by yuzhang on 2026/10/11.
//

#pragma once

#include <optional>
#include <regex>
#include <string>

#include <drogon/HttpRequest.h>

#include "ModelAliases.h"
#include "enums/OrderAction.h"
#include "utils/StringUtil.h"

namespace dto
{
    struct TransactionRecordDto
    {
        std::optional<int64_t> plan_id_;
        std::string symbol_;
        std::string action_;
        std::string filled_price_;
        std::string filled_qty_;
        std::string commission_{"0"};
        std::string filled_datetime_;
        std::string remark_;

        std::optional<std::string> validate() const
        {
            if (symbol_.empty())
            {
                return "股票代码不能为空";
            }
            if (!enums::parse_order_action(action_).has_value())
            {
                return "买卖方向不能为空不能为空";
            }
            if (!is_valid_decimal(filled_price_))
            {
                return "成交价格不合法";
            }
            if (!is_valid_decimal(filled_qty_))
            {
                return "成交数量不合法";
            }
            if (!is_valid_decimal(commission_))
            {
                return "佣金不合法";
            }
            if (!filled_datetime_.empty() && !is_valid_datetime(filled_datetime_))
            {
                return "成交时间不合法";
            }
            if (util::utf8_length(remark_) > 300)
            {
                return "备注长度不能超过300个字符";
            }
            return std::nullopt;
        }

        drogon_model::stock_lab::TransactionRecord to_model(
            const int64_t id,
            const int64_t user_id,
            const std::string& filled_amount,
            const trantor::Date& filled_datetime) const
        {
            drogon_model::stock_lab::TransactionRecord record;
            record.setId(id);
            record.setUserId(user_id);
            if (plan_id_.has_value())
            {
                record.setPlanId(plan_id_.value());
            }
            record.setSymbol(symbol_);
            record.setAction(action_);
            record.setFilledPrice(filled_price_);
            record.setFilledQty(filled_qty_);
            record.setFilledAmount(filled_amount);
            record.setCommission(commission_);
            record.setFilledDatetime(filled_datetime);
            if (!remark_.empty())
            {
                record.setRemark(remark_);
            }
            record.setDeleted(0);
            return record;
        }

    private:
        static bool is_valid_decimal(const std::string& value)
        {
            if (value.empty())
            {
                return false;
            }
            static const std::regex decimal_regex(R"(^-?\d{1,10}(\.\d{1,5})?$)");
            return std::regex_match(value, decimal_regex);
        }

        static bool is_valid_datetime(const std::string& value)
        {
            static const std::regex datetime_regex(
                R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)"
            );
            return std::regex_match(value, datetime_regex);
        }
    };
}

namespace drogon
{
    template <>
    inline dto::TransactionRecordDto fromRequest(const HttpRequest& req)
    {
        std::shared_ptr<Json::Value> json = req.getJsonObject();
        dto::TransactionRecordDto transaction_record_dto;
        if (json)
        {
            if (json->isMember("planId") && !(*json)["planId"].isNull())
            {
                transaction_record_dto.plan_id_ = std::stoll((*json)["planId"].asString());
            }
            if (json->isMember("symbol") && !(*json)["symbol"].isNull())
            {
                transaction_record_dto.symbol_ = (*json)["symbol"].asString();
            }
            if (json->isMember("action") && !(*json)["action"].isNull())
            {
                transaction_record_dto.action_ = (*json)["action"].asString();
            }
            if (json->isMember("filledPrice") && !(*json)["filledPrice"].isNull())
            {
                transaction_record_dto.filled_price_ = (*json)["filledPrice"].asString();
            }
            if (json->isMember("filledQty") && !(*json)["filledQty"].isNull())
            {
                transaction_record_dto.filled_qty_ = (*json)["filledQty"].asString();
            }
            if (json->isMember("commission") && !(*json)["commission"].isNull())
            {
                transaction_record_dto.commission_ = (*json)["commission"].asString();
            }
            if (json->isMember("filledDateTime") && !(*json)["filledDateTime"].isNull())
            {
                transaction_record_dto.filled_datetime_ = (*json)["filledDateTime"].asString();
            }
            if (json->isMember("remark") && !(*json)["remark"].isNull())
            {
                transaction_record_dto.remark_ = (*json)["remark"].asString();
            }
        }
        return transaction_record_dto;
    }
}
