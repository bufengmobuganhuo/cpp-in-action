//
// Created by yuzhang on 2026/10/5.
//

#ifndef STOCK_LAB_BACKEND_EMAILREPOSITORY_H
#define STOCK_LAB_BACKEND_EMAILREPOSITORY_H
#include <functional>
#include <string>
#include <drogon/orm/Exception.h>

namespace repository
{
    class EmailWhitelistRepository
    {
    public:
        using ExistsCallback = std::function<void(bool)>;
        void exists_by_email(const std::string &email, ExistsCallback on_success, std::function<void(const drogon::orm::DrogonDbException &)> on_error) const;
    };
}
#endif //STOCK_LAB_BACKEND_EMAILREPOSITORY_H
