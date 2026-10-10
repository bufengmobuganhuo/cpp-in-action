#pragma once

#include <drogon/HttpController.h>

#include "BaseCtrl.h"
#include "services/PositionSnapshotService.h"

using namespace drogon;

namespace controller
{
class PositionSnapshotCtrl : public drogon::HttpController<PositionSnapshotCtrl>, public BaseCtrl
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    // METHOD_ADD(PositionSnapshotCtrl::get, "/{2}/{1}", Get); // path is /controller/PositionSnapshotCtrl/{arg2}/{arg1}
    // METHOD_ADD(PositionSnapshotCtrl::your_method_name, "/{1}/{2}/list", Get); // path is /controller/PositionSnapshotCtrl/{arg1}/{arg2}/list
    // ADD_METHOD_TO(PositionSnapshotCtrl::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
    ADD_METHOD_TO(PositionSnapshotCtrl::get_position_snapshot, "/api/position-snapshot/list", Get, "filter::AuthFilter");
    ADD_METHOD_TO(PositionSnapshotCtrl::delete_by_id, "/api/position-snapshot/{1}", Delete, "filter::AuthFilter");

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
    void get_position_snapshot(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback);
    void delete_by_id(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string& symbol);
private:
    service::PositionSnapshotService position_snapshot_service_;
};
}
