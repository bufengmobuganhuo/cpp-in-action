//
// Created by yuzhang on 2026/10/7.
//

#include "SchedulerService.h"

#include <random>

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
        [](const drogon::orm::DrogonDbException& e)
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
    if (plan.getValueOfStartDate() > trantor::Date::now())
    {
        LOG_WARN << "transaction plan is not started, plan_id=" << plan.getValueOfId();
        return;
    }
    auto plan_ptr = std::make_shared<drogon_model::stock_lab::TransactionPlan>(plan);
    user_repository_.select_by_id(
        plan.getValueOfUserId(),
        [this, plan_ptr](drogon_model::stock_lab::User user)
        {
            transaction_record_service_.get_invest_statistics_vo(
                (*plan_ptr).getValueOfId(),
                (*plan_ptr).getValueOfUserId(),
                [this, plan_ptr, user = std::move(user)](vo::TransactionStatisticsVo vo)
                {
                    TransactionPlanReminderEmail email;
                    email.symbol_ = (*plan_ptr).getValueOfSymbol();
                    email.amount_ = (*plan_ptr).getValueOfAmount();
                    email.frequency_desc_ = (*plan_ptr).getValueOfFrequencyDesc();
                    email.total_invest_times_ = vo.total_invest_times_;
                    email.total_invest_amount_ = vo.total_invest_amount_;
                    email.target_total_value_ = vo.next_target_value_;
                    mail_service_.send_transaction_plan_remind_email(
                        user.getValueOfEmail(),
                        email,
                        [plan_ptr, this](bool success)
                        {
                            if (!success)
                            {
                                LOG_ERROR << "fail to send_transaction_plan_remind_email, plan_id=" << (*plan_ptr).getValueOfId();
                            }
                            else
                            {
                                informed_cache_.put((*plan_ptr).getValueOfId(), *plan_ptr);
                            }
                        }
                    );
                },
                [plan_ptr](const drogon::orm::DrogonDbException& e)
                {
                    LOG_ERROR << "failed to get invest statistics, plan_id="
                        << (*plan_ptr).getValueOfId() << ", e=" << e.base().what();
                }
            );
        },
        [plan_ptr](const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "failed to select user, user_id=" << (*plan_ptr).getValueOfUserId() << "e=" << e.base().what();
        }
    );
}
