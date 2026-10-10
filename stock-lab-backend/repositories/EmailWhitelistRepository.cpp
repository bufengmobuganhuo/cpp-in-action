//
// Created by yuzhang on 2026/10/5.
//
#include "EmailWhitelistRepository.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/orm/Mapper.h>

#include "EmailWhitelist.h"

void repository::EmailWhitelistRepository::exists_by_email(const std::string &email, ExistsCallback on_success, std::function<void(const drogon::orm::DrogonDbException &)> on_error)
{
    auto mapper = make_mapper<drogon_model::stock_lab::EmailWhitelist>();
    mapper.findBy(
        drogon::orm::Criteria(
            drogon_model::stock_lab::EmailWhitelist::Cols::_email,
            drogon::orm::CompareOperator::EQ,
            email
        ),
        [on_success = std::move(on_success)](
            const std::vector<drogon_model::stock_lab::EmailWhitelist>& rows) mutable
        {
            on_success(!rows.empty());
        },
        [on_error = std::move(on_error)](
            const drogon::orm::DrogonDbException &e) mutable
        {
            on_error(e);
        }
    );
}
