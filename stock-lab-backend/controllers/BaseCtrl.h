//
// Created by yuzhang on 2026/10/8.
//

#pragma once
#include <optional>
#include <drogon/HttpRequest.h>

namespace controller
{
    class BaseCtrl
    {
    protected:
        static std::optional<int64_t> get_user_id(const drogon::HttpRequestPtr& req)
        {
            try
            {
                const auto user_id = req->attributes()->get<std::string>("userId");
                return std::stoll(user_id);
            }
            catch (const std::exception& e)
            {
                LOG_ERROR << "failed to resolve userId from request attributes, error=" << e.what();
                return std::nullopt;
            }
        }
    };
}
