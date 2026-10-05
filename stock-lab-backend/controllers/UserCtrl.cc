#include "UserCtrl.h"

using namespace users;

// Add definition of your processing function here
void UserCtrl::send_verify_code(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, dto::LoginDto&& login_dto)
{
    auth_service_.send_verify_code(std::move(login_dto), std::move(callback));
}

void UserCtrl::login(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback,
    dto::LoginDto&& login_dto)
{
    auth_service_.verify_code(std::move(login_dto), std::move(callback));
}
