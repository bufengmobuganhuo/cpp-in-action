//
// Created by yuzhang on 2026/10/5.
//

#pragma once
#include <optional>
#include <string>

#include "BaseRepository.h"
#include "ModelAliases.h"

namespace repository
{
    class UserRepository : public BaseRepository
    {
    public:
        static std::optional<drogon_model::stock_lab::User> select_by_email(const std::string& email);
        static void select_by_id(
            int64_t user_id,
            std::function<void(drogon_model::stock_lab::User)> on_success,
            std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void insert(drogon_model::stock_lab::User& user);
    };
}
