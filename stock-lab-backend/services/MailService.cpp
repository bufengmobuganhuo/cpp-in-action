//
// Created by yuzhang on 2026/10/5.
//

#include "MailService.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <json/value.h>
#include <trantor/utils/Logger.h>

#include <ctime>
#include <fstream>
#include <sstream>

void service::MailService::send_verify_code_email(const std::string& email, int verify_code, Callback callback) const
{
    const std::string api_key = get_resend_api_key();
    const std::string from = get_mail_from();
    const std::string app_name = get_app_name();

    if (api_key.empty() || from.empty())
    {
        LOG_ERROR << "resend api key or mail from is empty";
        callback(false);
        return;
    }

    Json::Value body;
    body["from"] = from;
    body["to"].append(email);
    body["subject"] = "【" + app_name + "】邮箱验证码";

    const std::string html = build_verify_code_html(verify_code);
    if (html.empty())
    {
        LOG_ERROR << "verify code email html is empty";
        callback(false);
        return;
    }
    body["html"] = html;

    auto req = drogon::HttpRequest::newHttpJsonRequest(body);
    req->setMethod(drogon::Post);
    req->setPath("/emails");
    req->addHeader("Authorization", "Bearer " + api_key);
    req->addHeader("Content-Type", "application/json");

    auto client = drogon::HttpClient::newHttpClient("https://api.resend.com");

    client->sendRequest(
        req,
        [callback = std::move(callback)](
            drogon::ReqResult result,
            const drogon::HttpResponsePtr &resp
        ) mutable
        {
            if (result != drogon::ReqResult::Ok || resp == nullptr)
            {
                LOG_ERROR << "failed to call resend, result=" << static_cast<int>(result);
                callback(false);
                return;
            }
            const auto status = resp->getStatusCode();
            if (status < drogon::k200OK || status >= drogon::k300MultipleChoices)
            {
                LOG_ERROR << "resend returned non-2xx, status=" << status << ", body=" << resp->getBody();
                callback(false);
                return;
            }

            LOG_INFO << "send verify code email by resend";
            callback(true);
        }
    );
}

std::string service::MailService::get_resend_api_key() const
{
    return drogon::app().getCustomConfig()["resend"]["api_key"].asString();
}

std::string service::MailService::get_mail_from() const
{
    return drogon::app().getCustomConfig()["mail"]["from"].asString();
}

std::string service::MailService::get_app_name() const
{
    auto app_name = drogon::app().getCustomConfig()["app_name"].asString();
    return app_name.empty() ? "Stock Lab" : app_name;
}

std::string service::MailService::get_verify_code_template_path() const
{
    auto path = drogon::app().getCustomConfig()["mail"]["verify_code_template"].asString();
    return path.empty() ? "../templates/email/code.html" : path;
}

std::string service::MailService::build_verify_code_html(int verify_code) const
{
    std::string html = load_template(get_verify_code_template_path());
    if (html.empty())
    {
        return {};
    }

    replace_all(html, "{{appName}}", get_app_name());
    replace_all(html, "{{code}}", std::to_string(verify_code));
    replace_all(html, "{{year}}", current_year());
    return html;
}

std::string service::MailService::load_template(const std::string& path) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        LOG_ERROR << "failed to open email template, path=" << path;
        return {};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string service::MailService::current_year() const
{
    const std::time_t now = std::time(nullptr);
    std::tm local_time{};
#if defined(_WIN32)
    localtime_s(&local_time, &now);
#else
    localtime_r(&now, &local_time);
#endif
    return std::to_string(local_time.tm_year + 1900);
}

void service::MailService::replace_all(std::string& text, const std::string& from, const std::string& to)
{
    if (from.empty())
    {
        return;
    }

    std::size_t pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos)
    {
        text.replace(pos, from.length(), to);
        pos += to.length();
    }
}
