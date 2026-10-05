/**
 *
 *  AuthFilter.cc
 *
 */

#include "AuthFilter.h"

#include "dots/JsonResult.h"
#include "utils/JwtUtils.h"

namespace filters
{
    void AuthFilter::doFilter(const drogon::HttpRequestPtr& req, drogon::FilterCallback&& fcb, drogon::FilterChainCallback&& fccb)
    {
        if (req->method() == drogon::Options)
        {
            fccb();
            return;
        }

        std::string authorization = req->getHeader("Authorization");
        constexpr  std::string_view prefix = "Bearer ";
        if (authorization.rfind(prefix.data(), 0) != 0)
        {
            auto resp = dto::JsonResult::fail(dto::ResultCode::AuthError);
            resp->setStatusCode(drogon::k401Unauthorized);
            fcb(resp);
            return;
        }
        const std::string token = authorization.substr(prefix.size());

        try
        {
            util::JwtUtil jwt_util;
            const std::string user_id = jwt_util.resolve_user_id(token);

            req->attributes()->insert("userId", user_id);

            fccb();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR << "jwt validate failed: " << e.what();
            auto resp = dto::JsonResult::fail(dto::ResultCode::AuthError);
            resp->setStatusCode(drogon::k401Unauthorized);
            fcb(resp);
        }
    }
}
