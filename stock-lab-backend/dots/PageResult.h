//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <cstdint>
#include <vector>
#include <json/value.h>

namespace dto
{
    template<typename T>
    struct PageResult
    {
        int page_num_;
        int page_size_;
        int64_t total_;
        std::vector<T> data_;

        [[nodiscard]] Json::Value to_json() const
        {
            Json::Value root;
            root["pageNum"] = page_num_;
            root["pageSize"] = page_size_;
            root["total"] = Json::Int64(total_);

            Json::Value data(Json::arrayValue);
            for (const auto& item : data_)
            {
                data.append(item.to_json());
            }
            root["data"] = data;
            return root;
        }
    };
}
