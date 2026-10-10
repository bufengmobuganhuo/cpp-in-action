//
// Created by yuzhang on 2026/10/5.
//

#ifndef STOCK_LAB_BACKEND_EMAILREPOSITORY_H
#define STOCK_LAB_BACKEND_EMAILREPOSITORY_H
#include <functional>
#include <string>
#include <drogon/orm/Exception.h>

#include "BaseRepository.h"

namespace repository
{
    class EmailWhitelistRepository : public BaseRepository
    {
    public:
        using ExistsCallback = std::function<void(bool)>;
        static void exists_by_email(const std::string &email, ExistsCallback on_success, std::function<void(const drogon::orm::DrogonDbException &)> on_error) ;
    };
}
#endif //STOCK_LAB_BACKEND_EMAILREPOSITORY_H
