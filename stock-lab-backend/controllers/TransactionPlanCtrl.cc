#include "TransactionPlanCtrl.h"

#include "dots/JsonResult.h"

using namespace controller;

// Add definition of your processing function here
void TransactionPlanCtrl::add_transaction_plan(const HttpRequestPtr& req,
                                               std::function<void(const HttpResponsePtr&)>&& callback,
                                               const dto::TransactionPlanDto& dto)
{
    const auto user_id = get_user_id(req);
    if (user_id.has_value())
    {
        transaction_plan_service_.add_transaction_plan(dto, user_id.value(), std::move(callback));
    }
    else
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
    }
}

void TransactionPlanCtrl::get_transaction_plans(const HttpRequestPtr& req,
                                                std::function<void(const HttpResponsePtr&)>&& callback, int page_num,
                                                int page_size)
{
    const auto user_id = get_user_id(req);
    if (user_id.has_value())
    {
        transaction_plan_service_.get_transaction_plans(user_id.value(), page_num, page_size, std::move(callback));
    }
    else
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
    }
}

void TransactionPlanCtrl::update_transaction_plan_status(const HttpRequestPtr& req,
                                                         std::function<void(const HttpResponsePtr&)>&& callback,
                                                         const int64_t id, const std::string& status)
{
    std::optional<int64_t> user_id = get_user_id(req);
    if (!user_id.has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
        return;
    }
    std::optional<enums::TransactionPlanStatus> optional = enums::parse(status);
    if (!optional.has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }
    transaction_plan_service_.update_transaction_plan_status(id, user_id.value(), optional.value(), std::move(callback));
}
