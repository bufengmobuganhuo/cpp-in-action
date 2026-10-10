//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <string>
namespace enums
{
    enum PositionSnapshotStatus
    {
        /**
         * 持有仓位
         */
        HOLDING,

        /**
         * 已清仓
         */
        LIQUIDATED
    };

    inline constexpr std::string name(PositionSnapshotStatus status)
    {
        switch (status)
        {
        case HOLDING:
            return "HOLDING";
            default:
        case LIQUIDATED:
            return  "LIQUIDATED";
        }
    }
}