#pragma once

#include <drogon/HttpController.h>

#include "BaseCtrl.h"
#include "services/TransactionRecordService.h"

using namespace drogon;

namespace controller
{
class TransactionRecordCtrl : public drogon::HttpController<TransactionRecordCtrl>, public BaseCtrl
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    // METHOD_ADD(TransactionRecordCtrl::get, "/{2}/{1}", Get); // path is /controller/TransactionRecordCtrl/{arg2}/{arg1}
    // METHOD_ADD(TransactionRecordCtrl::your_method_name, "/{1}/{2}/list", Get); // path is /controller/TransactionRecordCtrl/{arg1}/{arg2}/list
    // ADD_METHOD_TO(TransactionRecordCtrl::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
    ADD_METHOD_TO(TransactionRecordCtrl::add_transaction_record, "/api/transaction/record", Post, "filter::AuthFilter");
    ADD_METHOD_TO(TransactionRecordCtrl::get_latest_transaction_records, "/api/transaction/record/list", Get, "filter::AuthFilter");
    ADD_METHOD_TO(TransactionRecordCtrl::get_transaction_records_by_plan_id, "/api/transaction/record/list-by-plan", Get, "filter::AuthFilter");

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
    void add_transaction_record(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback,
                                const dto::TransactionRecordDto& dto);
    void get_latest_transaction_records(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    void get_transaction_records_by_plan_id(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
private:
    service::TransactionRecordService transaction_record_service_;
};
}
