//
// Created by yuzhang on 2026/10/5.
//

#pragma once
#include <optional>
#include <string>

#include "ModelAliases.h"

namespace repository
{
    class UserRepository
    {
    public:
        std::optional<drogon_model::stock_lab::User> selectByEmail(const std::string& email);
        void insert(drogon_model::stock_lab::User &user);
    };
}
