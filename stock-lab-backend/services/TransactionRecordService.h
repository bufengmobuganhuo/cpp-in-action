//
// Created by yuzhang on 2026/10/7.
//

#pragma once
#include <functional>
#include <optional>
#include <drogon/HttpResponse.h>

#include "ContractService.h"
#include "dots/TransactionRecordDto.h"
#include "ModelAliases.h"
#include "vo/TransactionStatisticsVo.h"

namespace service
{
    class TransactionRecordService
    {
    public:
        using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;
        void add_transaction_record(
            const dto::TransactionRecordDto& transaction_record_dto,
            int64_t user_id,
            ResponseCallback callback);
        void get_transaction_records(
            const std::string& filled_since,
            const std::string& filled_to,
            int64_t user_id,
            std::optional<int64_t> plan_id,
            int64_t last_id,
            int page_size,
            ResponseCallback callback);
        void get_invest_statistics(int64_t plan_id, int64_t user_id,
                                   std::function<void(vo::TransactionStatisticsVo)> callback);

    private:
        void check(int64_t user_id, const dto::TransactionRecordDto& record_dto,
                   std::function<void(bool, std::optional<std::string> msg)> check_result);
        bool update_position_snapshot_for_sell(
            drogon_model::stock_lab::PositionSnapshot& snapshot,
            const dto::TransactionRecordDto& record_dto,
            int64_t user_id);
        std::optional<drogon_model::stock_lab::PositionSnapshot> build_or_update_position_snapshot_for_buy(
            const std::optional<drogon_model::stock_lab::PositionSnapshot>& snapshot,
            const dto::TransactionRecordDto& record_dto,
            int64_t user_id);
        ContractService contract_service_;
    };
}
