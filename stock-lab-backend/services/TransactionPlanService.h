//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <functional>
#include <drogon/HttpResponse.h>

#include "ContractService.h"
#include "dots/JsonResult.h"
#include "dots/TransactionPlanDto.h"
#include "enums/TransactionPlanStatus.h"
#include "repositories/TransactionPlanRepository.h"
#include "utils/Snowflake.h"

namespace service
{
    struct InvestFrequency
    {
        std::string cron_;
        std::string frequency_desc_;
    };

    class TransactionPlanService
    {
    public:
        using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;
        void add_transaction_plan(const dto::TransactionPlanDto& transaction_plan_dto, int64_t user_id,
                                  ResponseCallback callback);
        void get_transaction_plans(int64_t user_id, int page_num, int page_size, ResponseCallback callback);
        void update_transaction_plan_status(uint64_t id, int64_t user_id, enums::TransactionPlanStatus status,
                                            ResponseCallback callback);

    private:
        static constexpr int MAX_PLAN_COUNT = 10;
        void check(
            const dto::TransactionPlanDto& transaction_plan_dto,
            std::string& cron,
            int64_t user_id,
            std::function<void()> on_success,
            std::function<void(drogon::HttpResponsePtr)> on_error);
        void resolve_invest_requency(const dto::TransactionPlanDto& transaction_plan_dto,
                                     std::function<void(InvestFrequency)> on_success,
                                     std::function<void(drogon::HttpResponsePtr)> on_error);
        ContractService contract_service_;
        util::Snowflake snowflake_{1, 1};
    };
}
