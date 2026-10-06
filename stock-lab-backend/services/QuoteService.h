//
// Created by yuzhang on 2026/10/6.
//

#pragma once
#include <string>

#include "dots/Quote.h"
#include "utils/Math.h"
#include "utils/TtlCache.h"

namespace service
{
    class QuoteService
    {
    public:
        std::optional<dto::Quote> get_latest_quote(const std::string& symbol);
    private:
        util::TtlCache<std::string, dto::Quote> quote_cache_;
        static std::string get_quote_url();
        static std::string get_quote_path();
        static std::string get_quote_token();
        static std::optional<dto::Quote> do_query_latest_quote(const std::string& symbol);
    };
}

