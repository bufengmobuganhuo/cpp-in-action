//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <regex>
#include <string>
#include <drogon/HttpRequest.h>

namespace dto
{
    struct LoginDto
    {
        std::string email_;
        int verify_code_{0};

        std::optional<std::string> validate() const
        {
            if (email_.empty())
            {
                return "邮箱不能为空";
            }
            static const std::regex email_regex(
            R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
            if (!std::regex_match(email_, email_regex))
            {
                return "邮箱格式不合法";
            }

            return std::nullopt;
        }
    };
}

namespace drogon
{
    template<>
    inline dto::LoginDto fromRequest(const HttpRequest& req)
    {
        std::shared_ptr<Json::Value> json = req.getJsonObject();
        dto::LoginDto login_dto;
        if (json)
        {
            login_dto.email_ = (*json)["email"].asString();
            login_dto.verify_code_ = (*json)["verifyCode"].asInt();
        }
        return login_dto;
    }
}