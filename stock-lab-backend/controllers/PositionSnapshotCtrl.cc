#include "PositionSnapshotCtrl.h"

#include "dots/JsonResult.h"

using namespace controller;

// Add definition of your processing function here
void PositionSnapshotCtrl::get_position_snapshot(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::optional<int64_t> user_id = get_user_id(req);
    if (user_id.has_value())
    {
        position_snapshot_service_.get_position_snapshots(user_id.value(), std::move(callback));
    }
    else
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
    }
}

void PositionSnapshotCtrl::delete_by_id(const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback, const std::string& symbol)
{
    std::optional<int64_t> user_id = get_user_id(req);
    if (user_id.has_value())
    {
        position_snapshot_service_.reset_position_snaphosts(symbol, user_id.value(), std::move(callback));
    }
    else
    {
        callback(dto::JsonResult::fail(dto::ResultCode::AuthError));
    }
}

