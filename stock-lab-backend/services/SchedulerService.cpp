//
// Created by yuzhang on 2026/10/7.
//

#include "SchedulerService.h"

#include "enums/TransactionPlanStatus.h"
#include "utils/CronUtil.h"

void service::SchedulerService::scan()
{
    transaction_plan_repository_.select_all(
        TARGET_STATUS,
        [this](std::vector<drogon_model::stock_lab::TransactionPlan> rows)
        {
            for (const auto& item : rows)
            {
                if (informed_cache_.contains(item.getValueOfId()))
                {
                    continue;
                }
                auto now = std::chrono::system_clock::now();
                std::optional<std::chrono::system_clock::time_point> optional =
                    util::next_time_point(item.getValueOfCron(), now - std::chrono::minutes(1));
                if (!optional.has_value())
                {
                    continue;
                }
                if (now >= optional.value())
                {
                    execute_plan(item);
                }
            }
        },
        [](const drogon::orm::DrogonDbException &e)
        {
            LOG_ERROR << "failed to scan plan, e=" << e.base().what();
        }
        );
}

void service::SchedulerService::execute_plan(const drogon_model::stock_lab::TransactionPlan& plan)
{
    if (plan.getValueOfStatus() != enums::name(enums::ENABLED))
    {
        LOG_WARN << "transaction plan is not enabled, plan_id=" << plan.getValueOfId();
        return;
    }
    else if (plan.getValueOfStartDate() > trantor::Date::now())
    {
        LOG_WARN << "transaction plan is not started, plan_id=" << plan.getValueOfId();
        return;
    }

}
