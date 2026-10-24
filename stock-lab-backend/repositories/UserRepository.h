//
// Created by yuzhang on 2026/10/5.
//

#pragma once
#include <functional>
#include <optional>
#include <string>

#include "BaseRepository.h"
#include "ModelAliases.h"

namespace repository
{
    class UserRepository : public BaseRepository
    {
    public:
        static void select_by_email(
            const std::string& email,
            std::function<void(std::optional<drogon_model::stock_lab::User>)> on_success,
            std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void select_by_id(
            int64_t user_id,
            std::function<void(drogon_model::stock_lab::User)> on_success,
            std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void insert(
            const drogon_model::stock_lab::User& user,
            std::function<void(drogon_model::stock_lab::User)> on_success,
            std::function<void(const drogon::orm::DrogonDbException&)> on_error);
    };
}
