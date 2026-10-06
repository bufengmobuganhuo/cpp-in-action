//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <string>

#include "ModelAliases.h"
#include "dots/JsonResult.h"

namespace vo
{
    struct PositionSnapshotDetailVo
    {
        std::string id_;
        std::string symbol_;
        std::string description_;

        std::string diluted_cost_;
        std::string position_quantity_;
        std::string net_investment_amount_;
        std::string commission_;
        std::string ratio_;
        std::string latest_price_;
        std::string total_profit_;
        std::string ref_return_rate_;
        std::string true_return_rate_;

        std::string created_at_;
        std::string updated_at_;

        static PositionSnapshotDetailVo from_model(
            const drogon_model::stock_lab::PositionSnapshot& position_snapshot)
        {
            PositionSnapshotDetailVo vo;
            vo.id_ = std::to_string(position_snapshot.getValueOfId());
            vo.symbol_ = position_snapshot.getValueOfSymbol();
            vo.description_ = position_snapshot.getValueOfDescription();
            vo.position_quantity_ = position_snapshot.getValueOfPositionQuantity();
            vo.commission_ = position_snapshot.getValueOfCommission();

            if (position_snapshot.getCreatedAt())
            {
                vo.created_at_ = position_snapshot.getCreatedAt()->toDbStringLocal();
            }
            if (position_snapshot.getUpdatedAt())
            {
                vo.updated_at_ = position_snapshot.getUpdatedAt()->toDbStringLocal();
            }

            return vo;
        }

        Json::Value to_json() const
        {
            Json::Value root;
            root["id"] = id_;
            root["symbol"] = symbol_;
            root["description"] = description_;
            root["dilutedCost"] = diluted_cost_;
            root["positionQuantity"] = position_quantity_;
            root["netInvestmentAmount"] = net_investment_amount_;
            root["commission"] = commission_;
            root["ratio"] = ratio_;
            root["latestPrice"] = latest_price_;
            root["totalProfit"] = total_profit_;
            root["refReturnRate"] = ref_return_rate_;
            root["trueReturnRate"] = true_return_rate_;
            root["createdAt"] = created_at_;
            root["updatedAt"] = updated_at_;
            return root;
        }
    };
}
