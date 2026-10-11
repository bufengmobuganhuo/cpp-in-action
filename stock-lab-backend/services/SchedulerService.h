//
// Created by yuzhang on 2026/10/7.
//

#pragma once
#include "MailService.h"
#include "ModelAliases.h"
#include "TransactionRecordService.h"
#include "enums/TransactionPlanStatus.h"
#include "repositories/TransactionPlanRepository.h"
#include "repositories/UserRepository.h"
#include "utils/TtlCache.h"

namespace service
{
    class SchedulerService
    {
    public:
        const std::vector<std::string_view> TARGET_STATUS = std::vector{enums::name(enums::ENABLED)};
        void scan();
    private:
        repository::UserRepository user_repository_;
        repository::TransactionPlanRepository transaction_plan_repository_;
        MailService mail_service_;
        TransactionRecordService transaction_record_service_;
        util::TtlCache<uint64_t, drogon_model::stock_lab::TransactionPlan> informed_cache_;
        void execute_plan(const drogon_model::stock_lab::TransactionPlan& plan);
    };
}

