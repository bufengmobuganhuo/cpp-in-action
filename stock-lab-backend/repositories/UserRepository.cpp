//
// Created by yuzhang on 2026/10/5.
//

#include "UserRepository.h"

#include <drogon/HttpAppFramework.h>

std::optional<drogon_model::stock_lab::User> repository::UserRepository::selectByEmail(const std::string& email)
{
    auto mapper = make_mapper<drogon_model::stock_lab::User>();
    std::vector<drogon_model::stock_lab::User> vector = mapper.findBy(
        drogon::orm::Criteria(
                drogon_model::stock_lab::User::Cols::_email,
                drogon::orm::CompareOperator::EQ,
                email
        )
    );
    return vector.empty() ? std::nullopt : std::optional(vector[0]);
}

void repository::UserRepository::insert(drogon_model::stock_lab::User& user)
{
    auto mapper = make_mapper<drogon_model::stock_lab::User>();
    mapper.insert(user);
}
