//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <cstdint>
#include <functional>

#include "BaseRepository.h"
#include "EmailWhitelist.h"
#include "ModelAliases.h"
#include "dots/TransactionPlanDto.h"

namespace repository
{
    class TransactionPlanRepository : public BaseRepository
    {
    public:
        using Plan = drogon_model::stock_lab::TransactionPlan;
        static void select_by_user_id(
            int64_t user_id,
            const std::vector<std::string>& statuses,
            int page_num,
            int page_size,
            std::function<void(std::vector<Plan>, size_t)> on_success,
            std::function<void(const drogon::orm::DrogonDbException&)> on_error
        );
        static void select_all(
            const std::vector<std::string>& statuses,
            std::function<void(std::vector<Plan>)> on_success,
            std::function<void(const drogon::orm::DrogonDbException&)> on_error
        );
        static void count_by_user_id(
            int64_t user_id,
            const std::vector<std::string>& statuses,
            std::function<void(size_t)> on_success,
            std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void update_transaction_plan_status(uint64_t id,
                                                   int64_t user_id,
                                                   const std::string& status,
                                                   std::function<void()> on_success,
                                                   std::function<void(const drogon::orm::DrogonDbException& e)>
                                                   on_error);
        static void insert(
            Plan plan,
            std::function<void()> on_success,
            std::function<void(const drogon::orm::DrogonDbException& e)> on_error);
    };
}
