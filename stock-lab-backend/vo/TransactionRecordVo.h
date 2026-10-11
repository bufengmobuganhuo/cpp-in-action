//
// Created by yuzhang on 2026/10/10.
//

#pragma once

#include <string>
#include <json/json.h>

#include "ModelAliases.h"
#include "vo/TransactionPlanVo.h"

namespace vo
{
    struct TransactionRecordVo
    {
        std::string id_;
        std::string symbol_;
        std::string filled_datetime_;
        std::string filled_qty_;
        std::string filled_price_;
        std::string action_;
        std::string filled_amount_;
        std::string commission_;
        std::string plan_id_;
        std::string remark_;

        static TransactionRecordVo from_model(const drogon_model::stock_lab::TransactionRecord& record)
        {
            TransactionRecordVo vo;
            vo.id_ = std::to_string(record.getValueOfId());
            vo.symbol_ = record.getValueOfSymbol();
            vo.action_ = record.getValueOfAction();
            vo.filled_qty_ = TransactionPlanVo::strip_trailing_zeros(record.getValueOfFilledQty());
            vo.filled_price_ = TransactionPlanVo::strip_trailing_zeros(record.getValueOfFilledPrice());
            vo.filled_amount_ = TransactionPlanVo::strip_trailing_zeros(record.getValueOfFilledAmount());
            vo.commission_ = TransactionPlanVo::strip_trailing_zeros(record.getValueOfCommission());
            vo.plan_id_ = std::to_string(record.getValueOfPlanId());

            if (record.getFilledDatetime())
            {
                vo.filled_datetime_ = record.getFilledDatetime()->toDbStringLocal();
            }
            if (record.getRemark())
            {
                vo.remark_ = record.getValueOfRemark();
            }

            return vo;
        }

        Json::Value to_json() const
        {
            Json::Value root;
            root["id"] = id_;
            root["symbol"] = symbol_;
            root["filledDateTime"] = filled_datetime_;
            root["filledQty"] = filled_qty_;
            root["filledPrice"] = filled_price_;
            root["action"] = action_;
            root["filledAmount"] = filled_amount_;
            root["commission"] = commission_;
            root["planId"] = plan_id_;
            root["remark"] = remark_;
            return root;
        }
    };
}
