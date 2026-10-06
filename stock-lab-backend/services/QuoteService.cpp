//
// Created by yuzhang on 2026/10/6.
//

#include "QuoteService.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>

#include "EmailWhitelist.h"

std::optional<dto::Quote> service::QuoteService::get_latest_quote(const std::string& symbol)
{
    std::optional<dto::Quote> quote = quote_cache_.get(symbol);
    if (quote.has_value())
    {
        return quote;
    }
    std::optional<dto::Quote> optional = do_query_latest_quote(symbol);
    if (optional.has_value())
    {
        quote_cache_.put(symbol, optional.value());
    }
    return optional;
}

std::string service::QuoteService::get_quote_url()
{
    return drogon::app().getCustomConfig()["finnhub"]["url"].asString();
}

std::string service::QuoteService::get_quote_path()
{
    return drogon::app().getCustomConfig()["finnhub"]["quote_path"].asString();
}

std::string service::QuoteService::get_quote_token()
{
    return drogon::app().getCustomConfig()["finnhub"]["token"].asString();
}

std::optional<dto::Quote> service::QuoteService::do_query_latest_quote(const std::string& symbol)
{
    std::string url = get_quote_url();
    auto client = drogon::HttpClient::newHttpClient(url);
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath(get_quote_path());
    req->setQueryParameter("symbol", symbol);
    req->setQueryParameter("token", get_quote_token());

    std::pair<drogon::ReqResult, drogon::HttpResponsePtr> resp_pair = client->sendRequest(req);
    drogon::ReqResult req_result = resp_pair.first;
    drogon::HttpResponsePtr resp = resp_pair.second;
    if (req_result != drogon::ReqResult::Ok || resp == nullptr)
    {
        LOG_ERROR << "failed to query latest quote, symbol=" << symbol << ", http_code=" << req_result;
        return std::nullopt;
    }
    std::string_view body = resp->body();
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());

    if (!reader->parse(body.data(), body.data() + body.size(), &root, &errs))
    {
        LOG_ERROR << "failed to query latest quote, symbol=" << symbol << ", error=" << errs;
        return std::nullopt;
    }
    return dto::from_json(root);
}
