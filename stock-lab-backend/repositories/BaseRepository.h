//
// Created by yuzhang on 2026/10/7.
//

#pragma once
#include <drogon/HttpAppFramework.h>

#include "PositionSnapshots.h"

namespace repository
{
    class BaseRepository
    {
    protected:
        static drogon::orm::DbClientPtr db_client()
        {
            return drogon::app().getDbClient();
        }

        template<typename Model>
        static drogon::orm::Mapper<Model> make_mapper()
        {
            return drogon::orm::Mapper<Model>(db_client());
        }
    };
}

