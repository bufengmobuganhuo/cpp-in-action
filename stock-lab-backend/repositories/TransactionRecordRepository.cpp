//
// Created by yuzhang on 2026/10/6.
//

#include "TransactionRecordRepository.h"

#include <drogon/HttpAppFramework.h>

#include "ModelAliases.h"

void repository::TransactionRecordRepository::delete_by_symbol(const std::string& symbol, int64_t user_id,
                                                               std::function<void()> on_success, std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto db_client = drogon::app().getDbClient();
    drogon::orm::Mapper<drogon_model::stock_lab::TransactionRecord> mapper(db_client);
    mapper.deleteBy(
        drogon::orm::Criteria(
            drogon_model::stock_lab::TransactionRecord::Cols::_symbol,
            drogon::orm::CompareOperator::EQ,
            symbol
        ) &&
        drogon::orm::Criteria(
            drogon_model::stock_lab::TransactionRecord::Cols::_user_id,
            drogon::orm::CompareOperator::EQ,
            user_id
        ),
        [on_success = std::move(on_success)](size_t)
        {
            on_success();
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException &e)
        {
            on_error(e);
        }
    );
}
