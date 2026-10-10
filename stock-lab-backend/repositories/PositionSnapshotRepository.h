//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <cstdint>
#include <functional>
#include <string>

#include "BaseRepository.h"
#include "ModelAliases.h"

namespace respository
{
    class PositionSnapshotRepository : public BaseRepository
    {
    public:
        static void select_by_user_id(int64_t user_id,
                                      const std::string& position_snapshot_status,
                                      std::function<void(std::vector<drogon_model::stock_lab::PositionSnapshot>)>
                                      on_success,
                                      std::function<void(const drogon::orm::DrogonDbException&)> on_error);
        static void delete_by_symbol(const std::string& symbol, int64_t user_id,
                                     std::function<void()> on_success,
                                     std::function<void(const drogon::orm::DrogonDbException&)> on_error);
    };
}
