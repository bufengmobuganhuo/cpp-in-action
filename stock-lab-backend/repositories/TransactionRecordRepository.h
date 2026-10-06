//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <drogon/orm/Exception.h>

namespace repository
{
    class TransactionRecordRepository
    {
    public:
        void delete_by_symbol(const std::string& symbol, int64_t user_id,
                              std::function<void()> on_success,
                              std::function<void(const drogon::orm::DrogonDbException &)> on_error);
    };
}

