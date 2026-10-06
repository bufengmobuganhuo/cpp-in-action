/**
 *
 *  AuthFilter.h
 *
 */

#pragma once

#include <drogon/HttpFilter.h>

namespace filter
{
    class AuthFilter : public drogon::HttpFilter<AuthFilter>
    {
    public:
        void doFilter(
            const drogon::HttpRequestPtr& req,
            drogon::FilterCallback&& fcb,
            drogon::FilterChainCallback&& fccb) override;
    };
}

