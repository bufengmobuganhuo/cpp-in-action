//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <drogon/HttpResponse.h>
#include <functional>

#include "EmailWhitelist.h"
#include "QuoteService.h"
#include "repositories/PositionSnapshotRepository.h"
#include "repositories/TransactionRecordRepository.h"
#include "utils/Math.h"
#include "utils/TtlCache.h"

namespace service
{
    class PositionSnapshotService
    {
    public:
        using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;
        void get_position_snapshots(int64_t user_id, ResponseCallback);
        void reset_position_snaphosts(const std::string& symbol, int64_t user_id, ResponseCallback);
    private:
        respository::PositionSnapshotRepository position_snapshot_repository_;
        repository::TransactionRecordRepository transaction_record_repository_;
        QuoteService quote_service_;
    };
}

