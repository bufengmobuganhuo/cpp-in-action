//
// Created by yuzhang on 2026/10/7.
//

#pragma once
#include "ModelAliases.h"
#include "enums/TransactionPlanStatus.h"
#include "repositories/TransactionPlanRepository.h"
#include "utils/TtlCache.h"

namespace service
{
    class SchedulerService
    {
    public:
        const std::vector<std::string> TARGET_STATUS = std::vector{enums::name(enums::ENABLED)};
        void scan();
    private:
        repository::TransactionPlanRepository transaction_plan_repository_;
        util::TtlCache<uint64_t, drogon_model::stock_lab::TransactionPlan> informed_cache_;
        void execute_plan(const drogon_model::stock_lab::TransactionPlan& plan);
    };
}

