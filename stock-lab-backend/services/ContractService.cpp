//
// Created by yuzhang on 2026/10/5.
//

#include "ContractService.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>

#include "dots/ContractSearchResp.h"
#include "dots/JsonResult.h"

void service::ContractService::search_contract(const std::string& keyword, ResponseCallback response_callback) const
{
    do_search_contract_async(keyword, std::move(response_callback));
}

std::optional<dto::Contract> service::ContractService::get_by_symbol(const std::string& symbol)
{
     std::optional<dto::ContractSearchResp> contract_search_resp = do_search_contract_sync(symbol);
    if (contract_search_resp.has_value())
    {
        for (const auto& contract : contract_search_resp.value().result)
        {
            if (contract.symbol == symbol)
            {
                return contract;
            }
        }
    }
    return std::nullopt;
}

std::string service::ContractService::get_contract_search_url()
{
    return drogon::app().getCustomConfig()["finnhub"]["url"].asString();
}

std::string service::ContractService::get_contract_search_path()
{
    return drogon::app().getCustomConfig()["finnhub"]["search_path"].asString();
}

std::string service::ContractService::get_contract_search_token()
{
    return drogon::app().getCustomConfig()["finnhub"]["token"].asString();
}

std::optional<dto::ContractSearchResp> service::ContractService::do_search_contract_sync(const std::string& keyword)
{
    drogon::HttpClientPtr client = drogon::HttpClient::newHttpClient(get_contract_search_url());
    drogon::HttpRequestPtr req = build_request(keyword);
    std::pair<drogon::ReqResult, drogon::HttpResponsePtr> resp_pair = client->sendRequest(req);
    drogon::ReqResult req_result = resp_pair.first;
    drogon::HttpResponsePtr resp = resp_pair.second;
    std::pair<bool, dto::ContractSearchResp> result_pair = do_process_resp(req_result, resp, keyword);
    if (result_pair.first)
    {
        return result_pair.second;
    }
    return std::nullopt;
}

void service::ContractService::do_search_contract_async(const std::string& keyword, ResponseCallback callback)
{
    drogon::HttpRequestPtr req = build_request(keyword);
    drogon::app().getLoop()->queueInLoop(
        [req, callback = std::move(callback), keyword]() mutable
        {
            drogon::HttpClientPtr client = async_contract_search_client();
            client->sendRequest(
                req,
                [callback = std::move(callback), keyword] (
                    drogon::ReqResult req_result,
                    const drogon::HttpResponsePtr &resp
                ) mutable
                {
                    std::pair<bool, dto::ContractSearchResp> result_pair = do_process_resp(req_result, resp, keyword);
                    if (result_pair.first)
                    {
                        callback(dto::JsonResult::ok(result_pair.second.to_json()));
                    }
                    else
                    {
                        callback(dto::JsonResult::fail(dto::ResultCode::InternalError));
                    }
                });
        });
}

drogon::HttpClientPtr service::ContractService::async_contract_search_client()
{
    static auto client = drogon::HttpClient::newHttpClient(
        get_contract_search_url(),
        drogon::app().getLoop()
    );
    return client;
}

drogon::HttpRequestPtr service::ContractService::build_request(const std::string& keyword)
{
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath(get_contract_search_path());
    req->setQueryParameter("q", keyword);
    req->setQueryParameter("token", get_contract_search_token());

    return req;
}

std::pair<bool, dto::ContractSearchResp> service::ContractService::do_process_resp(drogon::ReqResult req_result,
    const drogon::HttpResponsePtr& resp, const std::string& keyword)
{
    if (req_result != drogon::ReqResult::Ok || resp == nullptr)
    {
        LOG_ERROR << "failed to search contract, keyword=" << keyword << ", http_code=" << req_result;
        return {false, dto::ContractSearchResp{}
    };
    }
    std::string_view body = resp->body();
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());

    if (!reader->parse(body.data(), body.data() + body.size(), &root, &errs) ||
        !root["result"].isArray())
    {
        LOG_ERROR << "failed to parse contract search resp, keyword=" << keyword << ", error=" << errs;
        return {false, dto::ContractSearchResp{}};
    }
    dto::ContractSearchResp contract_search_resp = dto::ContractSearchResp::from_json(root);
    return {true, contract_search_resp};
}
