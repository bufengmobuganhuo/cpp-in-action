//
// Created by yuzhang on 2026/10/6.
//

#include "PositionSnapshotRepository.h"

#include <drogon/HttpAppFramework.h>

#include "dots/LoginDto.h"

void respository::PositionSnapshotRepository::select_by_user_id(int64_t user_id,
                                                                const std::string& position_snapshot_status,
                                                                std::function<void(
                                                                    std::vector<
                                                                        drogon_model::stock_lab::PositionSnapshot>)>
                                                                on_success,
                                                                std::function<void(const drogon::orm::DrogonDbException &)> on_error)
{
    auto db_client = drogon::app().getDbClient();
    drogon::orm::Mapper<drogon_model::stock_lab::PositionSnapshot> mapper(db_client);
    mapper.findBy(
        drogon::orm::Criteria(
            drogon_model::stock_lab::PositionSnapshot::Cols::_user_id,
            drogon::orm::CompareOperator::EQ,
            user_id
        ) &&
        drogon::orm::Criteria(
            drogon_model::stock_lab::PositionSnapshot::Cols::_status,
            drogon::orm::CompareOperator::EQ,
            position_snapshot_status
        ),
        [on_success = std::move(on_success)](
        std::vector<drogon_model::stock_lab::PositionSnapshot> rows)
        {
            on_success(rows);
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        }
    );
}

void respository::PositionSnapshotRepository::delete_by_symbol(const std::string& symbol, int64_t user_id,
                                                               std::function<void()> on_success,
                                                               std::function<void(const drogon::orm::DrogonDbException &)> on_error)
{
    auto db_client = drogon::app().getDbClient();
    drogon::orm::Mapper<drogon_model::stock_lab::PositionSnapshot> mapper(db_client);
    mapper.deleteBy(
        drogon::orm::Criteria(
            drogon_model::stock_lab::PositionSnapshot::Cols::_user_id,
            drogon::orm::CompareOperator::EQ,
            user_id
        ) &&
        drogon::orm::Criteria(
            drogon_model::stock_lab::PositionSnapshot::Cols::_symbol,
            drogon::orm::CompareOperator::EQ,
            symbol
        ),
        [on_success = std::move(on_success)] (size_t)
        {
            on_success();
        },
        [on_error = std::move(on_error)] (const drogon::orm::DrogonDbException &e)
        {
            on_error(e);
        }
    );
}
