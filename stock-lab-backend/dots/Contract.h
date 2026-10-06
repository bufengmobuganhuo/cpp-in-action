//
// Created by yuzhang on 2026/10/5.
//

#pragma once
#include <string>
#include <json/value.h>

namespace dto
{
    struct Contract
    {
        std::string description;
        std::string type;
        std::string symbol;
        static Contract from_json(const Json::Value& root)
        {
            Contract contract;
            contract.description = root.get("description", "").asString();
            contract.type = root.get("type", "").asString();
            contract.symbol = root.get("symbol", "").asString();
            return contract;
        }

        Json::Value to_json() const
        {
            Json::Value root;
            root["description"] = description;
            root["type"] = type;
            root["symbol"] = symbol;
            return root;
        }
    };
}
