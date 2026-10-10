#pragma once

#include <drogon/HttpController.h>

#include "BaseCtrl.h"
#include "services/TransactionPlanService.h"

using namespace drogon;

namespace controller
{
class TransactionPlanCtrl : public drogon::HttpController<TransactionPlanCtrl>, public BaseCtrl
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    // METHOD_ADD(TransactionPlanCtrl::get, "/{2}/{1}", Get); // path is /controller/TransactionPlanCtrl/{arg2}/{arg1}
    // METHOD_ADD(TransactionPlanCtrl::your_method_name, "/{1}/{2}/list", Get); // path is /controller/TransactionPlanCtrl/{arg1}/{arg2}/list
    // ADD_METHOD_TO(TransactionPlanCtrl::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
    ADD_METHOD_TO(TransactionPlanCtrl::add_transaction_plan, "/api/transaction/plan", Post, "filter::AuthFilter");
    ADD_METHOD_TO(TransactionPlanCtrl::get_transaction_plans, "/api/transaction/plan/list?pageNum={1}&pageSize={2}", Get, "filter::AuthFilter");
    ADD_METHOD_TO(TransactionPlanCtrl::update_transaction_plan_status, "/api/transaction/plan/status?id={1}&status={2}", Put, "filter::AuthFilter");

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
    void add_transaction_plan(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, const dto::TransactionPlanDto& dto);
    void get_transaction_plans(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int page_num, int page_size);
    void update_transaction_plan_status(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback,
                                        int64_t id, const std::string& status);
private:
    service::TransactionPlanService transaction_plan_service_;
};
}
