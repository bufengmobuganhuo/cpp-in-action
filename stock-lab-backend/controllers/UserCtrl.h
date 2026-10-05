#pragma once

#include <drogon/HttpController.h>

#include "dots/JsonResult.h"
#include "dots/LoginDto.h"
#include "services/AuthService.h"

using namespace drogon;

namespace users
{
class UserCtrl : public drogon::HttpController<UserCtrl>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    // METHOD_ADD(UserCtrl::get, "/{2}/{1}", Get); // path is /users/UserCtrl/{arg2}/{arg1}
    // METHOD_ADD(UserCtrl::your_method_name, "/{1}/{2}/list", Get); // path is /users/UserCtrl/{arg1}/{arg2}/list
    // ADD_METHOD_TO(UserCtrl::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
    ADD_METHOD_TO(UserCtrl::send_verify_code, "/api/auth/verifyCode", Post);
    ADD_METHOD_TO(UserCtrl::login, "/api/auth/login", Post);
    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
    void send_verify_code(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> &&callback, dto::LoginDto &&login_dto);
    void login(const HttpRequestPtr &req, std::function<void(const HttpResponsePtr &)> && callback, dto::LoginDto &&login_dto);
private:
    service::AuthService auth_service_;
};
}
