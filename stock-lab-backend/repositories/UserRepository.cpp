//
// Created by yuzhang on 2026/10/5.
//

#include "UserRepository.h"

#include <drogon/HttpAppFramework.h>

void repository::UserRepository::select_by_email(
    const std::string& email,
    std::function<void(std::optional<drogon_model::stock_lab::User>)> on_success,
    std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<drogon_model::stock_lab::User>();
    mapper.findBy(
        drogon::orm::Criteria(
            drogon_model::stock_lab::User::Cols::_email,
            drogon::orm::CompareOperator::EQ,
            email
        ),
        [on_success = std::move(on_success)](std::vector<drogon_model::stock_lab::User> users) mutable
        {
            if (users.empty())
            {
                on_success(std::nullopt);
                return;
            }
            on_success(std::move(users.front()));
        },
        std::move(on_error)
    );
}

void repository::UserRepository::select_by_id(
    int64_t user_id,
    std::function<void(drogon_model::stock_lab::User)> on_success,
    std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<drogon_model::stock_lab::User>();
    mapper.findOne(
        drogon::orm::Criteria(
            drogon_model::stock_lab::User::Cols::_id,
            drogon::orm::CompareOperator::EQ,
            user_id
        ),
        [on_success = std::move(on_success)](drogon_model::stock_lab::User user)
        {
            on_success(user);
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        }
    );
}

void repository::UserRepository::insert(
    const drogon_model::stock_lab::User& user,
    std::function<void(drogon_model::stock_lab::User)> on_success,
    std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<drogon_model::stock_lab::User>();
    mapper.insert(user, std::move(on_success), std::move(on_error));
}
