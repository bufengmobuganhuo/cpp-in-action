//
// Created by yuzhang on 2026/10/5.
//

#pragma once
#include <string>
#include <chrono>
#include <cstdint>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/open-source-parsers-jsoncpp/traits.h>

namespace util
{
    class JwtUtil
    {
    public:
        using JwtTraits = jwt::traits::open_source_parsers_jsoncpp;

        static constexpr int64_t k_expiration_time_mills = 2592000000L;

        inline static const std::string secret_ = "stocklab-secret12587564";

        std::string create(int64_t user_id, std::chrono::system_clock::time_point expire_at) const
        {
            return jwt::create<JwtTraits>()
            .set_subject(std::to_string(user_id))
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(expire_at)
            .sign(jwt::algorithm::hs512{secret_});
        }

        std::string resolve_user_id(const std::string& token) const
        {
            auto decoded = jwt::decode<JwtTraits>(token);
            jwt::verify<JwtTraits>()
            .allow_algorithm(jwt::algorithm::hs512{secret_})
            .verify(decoded);
            return decoded.get_subject();
        }
    };
}
