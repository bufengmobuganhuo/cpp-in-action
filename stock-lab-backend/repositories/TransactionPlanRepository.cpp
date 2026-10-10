//
// Created by yuzhang on 2026/10/6.
//

#include "TransactionPlanRepository.h"

#include <drogon/HttpAppFramework.h>

void repository::TransactionPlanRepository::select_by_user_id(int64_t user_id,
                                                              const std::vector<std::string>& statuses,
                                                              int page_num, int page_size,
                                                              std::function<void(std::vector<Plan>, size_t)> on_success,
                                                              std::function<void(const drogon::orm::DrogonDbException&)>
                                                              on_error)
{
    page_num = std::max(page_num, 1);
    page_size = std::clamp(page_size, 1, 100);
    auto on_success_ptr = std::make_shared<std::function<void(std::vector<Plan>, size_t)>>(on_success);
    auto on_error_ptr = std::make_shared<std::function<void(const drogon::orm::DrogonDbException&)>>(on_error);

    drogon::orm::Mapper<Plan> mapper = make_mapper<Plan>();

    drogon::orm::Criteria criteria = drogon::orm::Criteria(
        Plan::Cols::_user_id,
        drogon::orm::CompareOperator::EQ,
        user_id
    ) &&
    drogon::orm::Criteria(
        Plan::Cols::_status,
        drogon::orm::CompareOperator::In,
        statuses
    );

    mapper.orderBy(Plan::Cols::_created_at, drogon::orm::SortOrder::DESC)
          .paginate(static_cast<size_t>(page_num), static_cast<size_t>(page_size))
          .findBy(
              criteria,
              [on_success_ptr, on_error_ptr, user_id, statuses](std::vector<Plan> rows)
              {
                  count_by_user_id(
                      user_id,
                      statuses,
                      [on_success_ptr, rows = std::move(rows)](size_t cnt) mutable
                      {
                          (*on_success_ptr)(std::move(rows), cnt);
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

void repository::TransactionPlanRepository::select_all(
    const std::vector<std::string>& statuses,
    std::function<void(std::vector<Plan>)> on_success,
    std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<Plan>();
    mapper.findBy(
        drogon::orm::Criteria(
            Plan::Cols::_status,
            drogon::orm::CompareOperator::In,
            statuses
        ),
        [on_success = std::move(on_success)](std::vector<Plan> rows)
        {
            on_success(std::move(rows));
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        }
    );
}

void repository::TransactionPlanRepository::count_by_user_id(
    int64_t user_id,
    const std::vector<std::string>& statuses,
    std::function<void(size_t)> on_success,
    std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    drogon::orm::Mapper<Plan> mapper = make_mapper<Plan>();
    mapper.count(
        drogon::orm::Criteria(
            Plan::Cols::_user_id,
            drogon::orm::CompareOperator::EQ,
            user_id
        ) &&
        drogon::orm::Criteria(
            Plan::Cols::_status,
            drogon::orm::CompareOperator::In,
            statuses
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

void repository::TransactionPlanRepository::update_transaction_plan_status(uint64_t id, int64_t user_id, const std::string& status,
    std::function<void()> on_success, std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<Plan>();
    mapper.updateBy(
        std::vector{Plan::Cols::_status},
        [on_success = std::move(on_success)](size_t)
        {
            on_success();
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        },
        drogon::orm::Criteria(
            Plan::Cols::_id,
            drogon::orm::CompareOperator::EQ,
            id
        ) &&
        drogon::orm::Criteria(
            Plan::Cols::_user_id,
            drogon::orm::CompareOperator::EQ,
            user_id
        ),
        status
    );
}

void repository::TransactionPlanRepository::insert(
    Plan plan,
    std::function<void()> on_success,
    std::function<void(const drogon::orm::DrogonDbException&)> on_error)
{
    auto mapper = make_mapper<Plan>();
    mapper.insert(
        plan,
        [on_success = std::move(on_success)](Plan)
        {
            on_success();
        },
        [on_error = std::move(on_error)](const drogon::orm::DrogonDbException& e)
        {
            on_error(e);
        }
    );
}
