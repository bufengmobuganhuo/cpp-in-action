//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <drogon/HttpResponse.h>

#include "MailService.h"
#include "dots/LoginDto.h"
#include "repositories/EmailWhitelistRepository.h"
#include "repositories/UserRepository.h"
#include "utils/JwtUtils.h"
#include "utils/Snowflake.h"
#include "utils/TtlCache.h"

namespace service
{
    class AuthService
    {
    private:
        util::TtlCache<std::string, int> verify_code_cache_{std::chrono::seconds(60)};
        repository::EmailWhitelistRepository email_whitelist_repository_;
        repository::UserRepository user_repository_;
        service::MailService mail_service_;
        util::Snowflake snowflake_{1, 1};
        util::JwtUtil jwt_util_;
        int generate_verify_code();
        int64_t register_if_necessary(const std::string& email);
    public:
        using ResponseCallback = std::function<void(const drogon::HttpResponsePtr &)>;
        void send_verify_code(const dto::LoginDto &login_dto, ResponseCallback callback);
        void verify_code(const dto::LoginDto &login_dto, ResponseCallback callback);
    };
}
