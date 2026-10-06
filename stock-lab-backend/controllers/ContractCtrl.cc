#include "ContractCtrl.h"

using namespace controller;

void ContractCtrl::queryContracts(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string&& keyword)
{
    contract_service_.search_contract(keyword, std::move(callback));
}
