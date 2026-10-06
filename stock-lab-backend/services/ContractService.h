//
// Created by yuzhang on 2026/10/5.
//

#pragma once
#include <string>
#include <drogon/HttpClient.h>
#include <functional>
#include <optional>
#include <utility>

#include "dots/Contract.h"
#include "dots/ContractSearchResp.h"

namespace service
{
    class ContractService
    {
    public:
        using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;
        std::optional<dto::Contract> get_by_symbol(const std::string& symbol);
        void search_contract(const std::string& keyword, ResponseCallback response_callback) const;

    private:
        static  std::string get_contract_search_url();
        static  std::string get_contract_search_path();
        static std::string get_contract_search_token();
        static  std::pair<drogon::HttpClientPtr, drogon::HttpRequestPtr> build_request_obj(const std::string& keyword);
        static  std::pair<bool, dto::ContractSearchResp> do_process_resp(drogon::ReqResult req_result,
                                                                 const drogon::HttpResponsePtr& resp,
                                                                 const std::string& keyword);
        static std::optional<dto::ContractSearchResp> do_search_contract_sync(const std::string& keyword);
        static void do_search_contract_async(const std::string& keyword, ResponseCallback callback);
    };
}
