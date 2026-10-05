//
// Created by yuzhang on 2026/10/5.
//

#pragma once
#include <string>

namespace dto
{
    struct LoginRespDto
    {
        std::string token_;
        int64_t expire_at_{0};

        Json::Value to_json() const
        {
            Json::Value root;
            root["token"] = token_;
            root["expireAt"] = Json::Int64(expire_at_);
            return root;
        }
    };
}
