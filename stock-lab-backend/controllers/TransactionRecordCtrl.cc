#include "TransactionRecordCtrl.h"

#include "utils/Math.h"
#include <limits>

#include "dots/JsonResult.h"

using namespace controller;

// Add definition of your processing function here
void TransactionRecordCtrl::add_transaction_record(const HttpRequestPtr& req,
                                                   std::function<void(const HttpResponsePtr&)>&& callback,
                                                   const dto::TransactionRecordDto& dto)
{
    const std::optional<int64_t> user_id = get_user_id(req);
    if (!user_id.has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
        return;
    }
    transaction_record_service_.add_transaction_record(dto, user_id.value(), std::move(callback));
}

void TransactionRecordCtrl::get_invest_statistics(const HttpRequestPtr& req,
                                                  std::function<void(const HttpResponsePtr&)>&& callback,
                                                  int64_t plan_id)
{
    std::optional<int64_t> user_id = get_user_id(req);
    if (!user_id.has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
        return;
    }
    transaction_record_service_.get_invest_statistics(plan_id, user_id.value(), std::move(callback));
}

void TransactionRecordCtrl::get_latest_transaction_records(const HttpRequestPtr& req,
                                                           std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::optional<int64_t> user_id = get_user_id(req);
    if (!user_id.has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
        return;
    }
    const std::string start_date = req->getParameter("startDate");
    const std::string end_date = req->getParameter("endDate");
    std::optional<int64_t> last_id = util::parse_int64(req->getParameter("lastId"));
    if (!last_id.has_value())
    {
        LOG_ERROR << "failed to resolve last_id from req=" << req->getQuery();
        callback(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }
    std::optional<int> page_size = util::parse_int(req->getParameter("pageSize"));
    transaction_record_service_.get_transaction_records(start_date, end_date, user_id.value(),
                                                        std::nullopt, last_id.value(),
                                                        page_size.value_or(10), std::move(callback));
}

void TransactionRecordCtrl::get_transaction_records_by_plan_id(const HttpRequestPtr& req,
                                                               std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::optional<int64_t> user_id = get_user_id(req);
    if (!user_id.has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
        return;
    }
    const std::string start_date = req->getParameter("startDate");
    const std::string end_date = req->getParameter("endDate");
    std::optional<int64_t> plan_id = util::parse_int64(req->getParameter("planId"));
    if (!plan_id.has_value())
    {
        LOG_ERROR << "failed to resolve plan_id from req=" << req->getQuery();
        callback(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }
    std::optional<int64_t> last_id = util::parse_int64(req->getParameter("lastId"));
    if (!last_id.has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }
    std::optional<int> page_size = util::parse_int(req->getParameter("pageSize"));
    transaction_record_service_.get_transaction_records(start_date, end_date, user_id.value(),
                                                        plan_id, last_id.value(),
                                                        page_size.value_or(10), std::move(callback));
}
