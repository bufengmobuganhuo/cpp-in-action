//
// Created by yuzhang on 2026/10/5.
//

#include "AuthService.h"

#include <cstdint>
#include <random>

#include "dots/JsonResult.h"
#include "dots/loginRespDto.h"
#include "utils/JwtUtils.h"
#include "utils/TtlCache.h"

void service::AuthService::send_verify_code(const dto::LoginDto& login_dto, ResponseCallback callback)
{
    auto err = login_dto.validate();
    if (err.has_value())
    {
        const auto code = dto::result_code_info(dto::ResultCode::ParamError).code_;
        callback(dto::JsonResult::fail(code, err.value(), drogon::k400BadRequest));
        return;
    }

    auto callback_ptr = std::make_shared<ResponseCallback>(std::move(callback));
    const std::string email = login_dto.email_;

    email_whitelist_repository_.exists_by_email(
        email,
        [this, email, callback_ptr](bool exists)
        {
            if (!exists)
            {
                (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::EmailNotInvited));
                return;
            }
            if (verify_code_cache_.contains(email))
            {
                (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::VerifyCodeExceedsThreshold));
                return;
            }
            const int verify_code = generate_verify_code();
            mail_service_.send_verify_code_email(
                email,
                verify_code,
                [this, email, verify_code, callback_ptr](bool success)
                {
                    if (!success)
                    {
                        (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                        return;
                    }
                    verify_code_cache_.put(email, verify_code, std::chrono::minutes(5));
                    (*callback_ptr)(dto::JsonResult::ok());
                });

        },
        [callback_ptr](const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "failed to query email whitelist: " << e.base().what();
            (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
        });
}

void service::AuthService::verify_code(const dto::LoginDto& login_dto, ResponseCallback callback)
{
    const std::string email = login_dto.email_;
    const int verify_code = login_dto.verify_code_;
    const auto callback_ptr = std::make_shared<ResponseCallback>(callback);
    email_whitelist_repository_.exists_by_email(
        email,
        [this, email, verify_code, callback_ptr](bool exits)
        {
            if (!exits)
            {
                (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::EmailNotInvited));
                return;
            }
            std::optional<int> cached_verify_code = verify_code_cache_.get(email);
            if (!cached_verify_code.has_value())
            {
                (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::VerifyCodeExpired));
                return;
            }
            if (cached_verify_code.value() != verify_code)
            {
                (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::VerifyCodeMismatch));
                return;
            }
            int64_t user_id = register_if_necessary(email);
            const auto expiration = std::chrono::system_clock::now() + std::chrono::milliseconds(util::JwtUtil::k_expiration_time_mills);

            std::string token = jwt_util_.create(user_id, expiration);

            const int64_t expire_at = std::chrono::duration_cast<std::chrono::milliseconds>(
                expiration.time_since_epoch()
            ).count();

            dto::LoginRespDto login_resp_dto;
            login_resp_dto.token_ = token;
            login_resp_dto.expire_at_ = expire_at;
            (*callback_ptr)(dto::JsonResult::ok(login_resp_dto.to_json()));
        },
        [callback_ptr](const drogon::orm::DrogonDbException &e)
        {
            LOG_ERROR << "failed to verify code: " << e.base().what();
            (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
        }
    );
}

int service::AuthService::generate_verify_code()
{
    static thread_local std::mt19937 generator{std::random_device{}()};
    std::uniform_int_distribution<int> distribution(100000, 999999);
    return distribution(generator);
}

int64_t service::AuthService::register_if_necessary(const std::string& email)
{
    std::optional<drogon_model::stock_lab::User> optional = user_repository_.selectByEmail(email);
    if (optional.has_value())
    {
        return optional->getValueOfId();
    }

    const int64_t user_id = snowflake_.next_id();
    drogon_model::stock_lab::User user;
    user.setId(user_id);
    user.setEmail(email);
    user_repository_.insert(user);
    return user_id;
}
