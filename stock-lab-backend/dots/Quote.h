//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <json/value.h>

#include "utils/Math.h"

namespace dto
{
    struct Quote
    {
        util::Decimal latest_price_;
    };

    static Quote from_json(Json::Value root)
    {
        Quote quote;
        quote.latest_price_ = root.get("c", "").asFloat();
        return quote;
    }
}
