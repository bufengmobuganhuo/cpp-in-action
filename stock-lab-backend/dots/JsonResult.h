//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <string>
#include <drogon/HttpResponse.h>

#include "PositionSnapshots.h"
#include "ResultCode.h"

namespace dto
{
    class JsonResult
    {
    public:
        static drogon::HttpResponsePtr ok(const Json::Value &data = Json::Value(Json::nullValue))
        {
            Json::Value root;
            const auto result_code_info = dto::result_code_info(dto::ResultCode::OK);
            root["code"] = result_code_info.code_;
            root["msg"] = result_code_info.msg_;
            root["data"] = data;

            auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
            resp->setStatusCode(drogon::k200OK);
            return resp;
        }

        static drogon::HttpResponsePtr fail(ResultCode result_code)
        {
            Json::Value root;
            const auto result_code_info = dto::result_code_info(result_code);
            root["code"] = result_code_info.code_;
            root["msg"] = result_code_info.msg_;

            auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
            resp->setStatusCode(drogon::k200OK);
            return resp;
        }

        static drogon::HttpResponsePtr fail(int code, std::string& msg, drogon::HttpStatusCode http_status_code)
        {
            Json::Value root;
            root["code"] = code;
            root["msg"] = msg;

            auto resp = drogon::HttpResponse::newHttpJsonResponse(root);
            resp->setStatusCode(http_status_code);
            return resp;
        }
    };
}
