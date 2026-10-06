#pragma once

#include <drogon/HttpController.h>

#include "services/ContractService.h"

using namespace drogon;

namespace controller
{
class ContractCtrl : public drogon::HttpController<ContractCtrl>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    // METHOD_ADD(ContractCtrl::get, "/{2}/{1}", Get); // path is /controller/ContractCtrl/{arg2}/{arg1}
    // METHOD_ADD(ContractCtrl::your_method_name, "/{1}/{2}/list", Get); // path is /controller/ContractCtrl/{arg1}/{arg2}/list
    // ADD_METHOD_TO(ContractCtrl::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
    //ADD_METHOD_TO(ContractCtrl::queryContracts, "/api/contract/base-info?keyword={1}", Get, "filter::AuthFilter");
    ADD_METHOD_TO(ContractCtrl::queryContracts, "/api/contract/base-info?keyword={1}", Get);

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
    void queryContracts(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, std::string &&keyword);
private:
    service::ContractService contract_service_;
};
}
