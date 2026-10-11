//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <drogon/orm/Exception.h>

#include "BaseRepository.h"
#include "ModelAliases.h"

namespace repository
{
    class TransactionRecordRepository : public BaseRepository
    {
    public:
        using Record = drogon_model::stock_lab::TransactionRecord;
        static void insert(const Record& record, std::function<void()> on_success,
                           std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void select_by_plan_id(int64_t user_id, int64_t plan_id, int page_num, int page_size,
                                      std::function<void(std::vector<Record>, size_t)> on_success,
                                      std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void select_by_page(const std::string& filled_since, const std::string& filled_to, int64_t user_id,
                                   std::optional<int64_t> plan_id, int64_t last_id, int page_size,
                                   std::function<void(std::vector<Record>, size_t)> on_success,
                                   std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void delete_by_symbol(const std::string& symbol, int64_t user_id,
                                     std::function<void()> on_success,
                                     std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void count_by_plan_id(int64_t user_id, int64_t plan_id, std::function<void(size_t)> on_success,
                                     std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void statistics_by_plan_id(int64_t user_id, int64_t plan_id,
                                          std::function<void(size_t, std::string, std::string, std::string)> on_success,
                                          std::function<void(const drogon::orm::DrogonDbException& e)> on_error);
    };
}
