//
// Created by yuzhang on 2026/10/6.
//

#include "TransactionRecordRepository.h"

#include <drogon/HttpAppFramework.h>

#include "ModelAliases.h"

void repository::TransactionRecordRepository::insert(const Record& record, std::function<void()> on_success,
    std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<Record>();
    mapper.insert(
        record,
        [on_success = std::move(on_success)](Record record)
        {
            on_success();
        },
        [on_error = std::move(on_error)] (const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        }
    );
}

void repository::TransactionRecordRepository::select_by_plan_id(int64_t user_id, int64_t plan_id, int page_num,
                                                                int page_size,
                                                                std::function<void(std::vector<Record>, size_t)>
                                                                on_success,
                                                                std::function<void(
                                                                    const drogon::orm::DrogonDbException&)> on_error)
{
    auto on_success_ptr = std::make_shared<std::function<void(std::vector<Record>, size_t)>>(on_success);
    auto on_error_ptr = std::make_shared<std::function<void(const drogon::orm::DrogonDbException&)>>(on_error);
    count_by_plan_id(user_id, plan_id,
                     [on_success_ptr, user_id, plan_id, on_error_ptr](size_t cnt)
                     {
                         drogon::orm::Mapper<Record> mapper = make_mapper<Record>();
                         mapper.findBy(
                             drogon::orm::Criteria(
                                 Record::Cols::_user_id,
                                 user_id
                             ) &&
                             drogon::orm::Criteria(
                                 Record::Cols::_plan_id,
                                 plan_id
                             ),
                             [on_success_ptr, cnt](std::vector<Record> records)
                             {
                                 (*on_success_ptr)(records, cnt);
                             },
                             [on_error_ptr](const drogon::orm::DrogonDbException& e)
                             {
                                 (*on_error_ptr)(e);
                             }
                         );
                     },
                     [on_error_ptr](const drogon::orm::DrogonDbException& e)
                     {
                         (*on_error_ptr)(e);
                     }
    );
}

void repository::TransactionRecordRepository::select_by_page(const std::string& filled_since,
                                                             const std::string& filled_to, int64_t user_id,
                                                             std::optional<int64_t> plan_id, int64_t last_id,
                                                             int page_size,
                                                             std::function<void(std::vector<Record>, size_t)>
                                                             on_success,
                                                             std::function<void(const drogon::orm::DrogonDbException&)>
                                                             on_error)
{
    drogon::orm::Criteria criteria =
        drogon::orm::Criteria(
            Record::Cols::_filled_datetime,
            drogon::orm::CompareOperator::GE,
            filled_since
        )
        &&
        drogon::orm::Criteria(
            Record::Cols::_filled_datetime,
            drogon::orm::CompareOperator::LE,
            filled_to
        )
        &&
        drogon::orm::Criteria(
            Record::Cols::_user_id,
            drogon::orm::CompareOperator::EQ,
            user_id
        )
        &&
        drogon::orm::Criteria(
            Record::Cols::_id,
            drogon::orm::CompareOperator::LT,
            last_id
        )
        &&
        drogon::orm::Criteria(
            Record::Cols::_deleted,
            drogon::orm::CompareOperator::EQ,
            0
        );
    if (plan_id.has_value())
    {
        criteria = criteria &&
            drogon::orm::Criteria(Record::Cols::_plan_id, drogon::orm::CompareOperator::EQ, plan_id.value());
    }
    auto mapper = make_mapper<Record>();
    auto on_success_ptr = std::make_shared<std::function<void(std::vector<Record>, size_t)>>(on_success);
    auto on_error_ptr = std::make_shared<std::function<void(const drogon::orm::DrogonDbException&)>>(on_error);
    mapper.orderBy(Record::Cols::_id, drogon::orm::SortOrder::DESC)
          .paginate(1, static_cast<size_t>(page_size))
          .findBy(criteria,
                 [on_success_ptr, criteria, on_error_ptr](std::vector<Record> rows)
                 {
                     auto mapper = make_mapper<Record>();
                     mapper.count(
                         criteria,
                         [on_success_ptr, rows = std::move(rows)](size_t cnt)
                         {
                             (*on_success_ptr)(rows, cnt);
                         },
                         [on_error_ptr](const drogon::orm::DrogonDbException& e)
                         {
                             (*on_error_ptr)(e);
                         }
                     );
                 },
                 [on_error_ptr](const drogon::orm::DrogonDbException& e)
                 {
                     (*on_error_ptr)(e);
                 }
          );
}

void repository::TransactionRecordRepository::delete_by_symbol(const std::string& symbol, int64_t user_id,
                                                               std::function<void()> on_success,
                                                               std::function<void
                                                                   (const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<drogon_model::stock_lab::TransactionRecord>();
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
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        }
    );
}

void repository::TransactionRecordRepository::count_by_plan_id(int64_t user_id, int64_t plan_id,
                                                               std::function<void(size_t)> on_success,
                                                               std::function<void
                                                                   (const drogon::orm::DrogonDbException&)> on_error)
{
    drogon::orm::Mapper<Record> mapper = make_mapper<Record>();
    mapper.count(
        drogon::orm::Criteria(
            Record::Cols::_user_id,
            user_id
        ) &&
        drogon::orm::Criteria(
            Record::Cols::_plan_id,
            plan_id
        ),
        [on_success = std::move(on_success)](size_t cnt)
        {
            on_success(cnt);
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        }
    );
}

void repository::TransactionRecordRepository::statistics_by_plan_id(int64_t user_id, int64_t plan_id,
                                                                    std::function<void(
                                                                        size_t, std::string, std::string,
                                                                        std::string)> on_success,
                                                                    std::function<void(
                                                                        const drogon::orm::DrogonDbException& e)>
                                                                    on_error)
{
    static const std::string sql =
        "SELECT "
        "COUNT(*) AS total_invest_times, "
        "COALESCE(SUM(filled_amount), 0) AS total_invest_amount, "
        "COALESCE(SUM(filled_qty), 0) AS total_invest_qty, "
        "COALESCE(SUM(commission), 0) AS total_invest_commission "
        "FROM transaction_records "
        "WHERE user_id = ? "
        "AND plan_id = ? "
        "AND deleted = 0";
    auto db_client_ptr = db_client();
    db_client_ptr->execSqlAsync(
        sql,
        [on_success = std::move(on_success)](const drogon::orm::Result& result)
        {
            if (result.empty())
            {
                on_success(0, "0", "0", "0");
                return;
            }
            const auto& row = result[0];
            on_success(
                row["total_invest_times"].as<size_t>(),
                row["total_invest_amount"].as<std::string>(),
                row["total_invest_qty"].as<std::string>(),
                row["total_invest_commission"].as<std::string>()
            );
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        },
        user_id,
        plan_id
    );
}
