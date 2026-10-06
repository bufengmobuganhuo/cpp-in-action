//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <vector>
#include "Contract.h"

namespace dto
{
    struct ContractSearchResp
    {
        int count;
        std::vector<Contract> result;

        Json::Value to_json() const
        {
            Json::Value root;
            root["count"] = count;
            Json::Value arr(Json::arrayValue);
            for (const auto& contract : result)
            {
                arr.append(contract.to_json());
            }
            root["result"] = arr;
            return root;
        }

        static ContractSearchResp from_json(const Json::Value& root)
        {
            ContractSearchResp contract_search_resp;
            contract_search_resp.count = root.get("count", 0).asInt();
            Json::Value arr = root.get("result", Json::arrayValue);
            std::vector<Contract> contracts;
            for (const auto& item : arr)
            {
                Contract contract = Contract::from_json(item);
                contracts.push_back(contract);
            }
            contract_search_resp.result = contracts;
            return contract_search_resp;
        }
    };
}
