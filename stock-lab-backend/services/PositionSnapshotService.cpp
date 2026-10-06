//
// Created by yuzhang on 2026/10/6.
//
#include "PositionSnapshotService.h"

#include <drogon/HttpAppFramework.h>
#include <algorithm>

#include "dots/Contract.h"
#include "dots/JsonResult.h"
#include "enums/PositionStatus.h"
#include "utils/Math.h"
#include "vo/PositionSnapshotDetailVo.h"

void service::PositionSnapshotService::get_position_snapshots(int64_t user_id, ResponseCallback callback)
{
    auto resp_callback_ptr = std::make_shared<ResponseCallback>(callback);
    position_snapshot_repository_.select_by_user_id(
        user_id,
        enums::name(enums::HOLDING),
        [resp_callback_ptr, this] (std::vector<drogon_model::stock_lab::PositionSnapshot> position_snapshots)
        {
            util::Decimal total_net_investment_amount = 0;
            std::vector<std::pair<vo::PositionSnapshotDetailVo, util::Decimal>> result;
            for (const auto& position_snapshot : position_snapshots)
            {
                vo::PositionSnapshotDetailVo detail_vo = vo::PositionSnapshotDetailVo::from_model(position_snapshot);

                util::Decimal total_buy_amount(position_snapshot.getValueOfTotalBuyAmount());
                util::Decimal total_sell_amount(position_snapshot.getValueOfTotalSellAmount());
                util::Decimal commission(position_snapshot.getValueOfCommission());
                util::Decimal position_quantity(position_snapshot.getValueOfPositionQuantity());

                util::Decimal net_investment_amount = total_buy_amount - total_sell_amount + commission;
                detail_vo.net_investment_amount_ = util::format_decimal(net_investment_amount);

                util::Decimal diluted_cost = net_investment_amount / position_quantity;
                detail_vo.diluted_cost_ = util::format_decimal(diluted_cost);

                std::optional<dto::Quote> quote = quote_service_.get_latest_quote(position_snapshot.getValueOfSymbol());
                if (!quote.has_value())
                {
                    continue;
                }
                util::Decimal latest_price = quote.value().latest_price_;
                detail_vo.latest_price_ = util::format_decimal(latest_price);

                util::Decimal price_spread = latest_price - diluted_cost;
                util::Decimal total_profit = price_spread * position_quantity;
                detail_vo.total_profit_ = util::format_decimal(total_profit);

                util::Decimal ref_return_rate = total_profit / total_buy_amount * 100;
                detail_vo.ref_return_rate_ = util::format_decimal(ref_return_rate);

                if (net_investment_amount > 0)
                {
                    util::Decimal true_return_rate = total_profit / net_investment_amount * 100;
                    detail_vo.true_return_rate_ = util::format_decimal(true_return_rate);
                }

                total_net_investment_amount += net_investment_amount;
                result.emplace_back(std::move(detail_vo), net_investment_amount);
            }

            if (total_net_investment_amount > 0)
            {
                util::Decimal final_total_net_investment_amount = total_net_investment_amount;
                for (auto& pair : result)
                {
                    util::Decimal ratio = pair.second / final_total_net_investment_amount;
                    pair.first.ratio_ = util::format_decimal(ratio);
                }
                std::sort(
                    result.begin(),
                    result.end(),
                    [](const auto& lhs, const auto& rhs)
                    {
                        return lhs.second > rhs.second;
                    });
            }
            Json::Value position_details(Json::arrayValue);
                        for (const auto& item : result)
                        {
                            position_details.append(item.first.to_json());
                        }
            Json::Value data;
            data["totalNetInvestmentAmount"] =
                util::format_decimal(total_net_investment_amount);
            data["positionDetails"] = position_details;

            (*resp_callback_ptr)(dto::JsonResult::ok(data));
        },
        [resp_callback_ptr, user_id] (const drogon::orm::DrogonDbException &e)
        {
            LOG_ERROR << "failed to select position_snapshots, "
            << "user_id=" << user_id
            << "excep=" << e.base().what();
            (*resp_callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
        }
        );
}

void service::PositionSnapshotService::reset_position_snaphosts(const std::string& symbol, int64_t user_id,
                                                                ResponseCallback callback)
{
    auto response_callback = std::make_shared<ResponseCallback>(callback);
    auto db_client = drogon::app().getDbClient();

    db_client->newTransactionAsync(
        [response_callback, symbol, user_id](const std::shared_ptr<drogon::orm::Transaction>& transaction)
        {
            if (!transaction)
            {
                (*response_callback)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                return;
            }
            transaction->setCommitCallback(
                [response_callback](bool committed)
                {
                    if (committed)
                    {
                        (*response_callback)(dto::JsonResult::ok());
                    }
                    else
                    {
                        (*response_callback)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                    }
                }
            );
            drogon::orm::Mapper<drogon_model::stock_lab::PositionSnapshot> position_snapshot_mapper(transaction);
            position_snapshot_mapper.deleteBy(
                drogon::orm::Criteria(
                    drogon_model::stock_lab::PositionSnapshot::Cols::_symbol,
                    drogon::orm::CompareOperator::EQ,
                    symbol
                ) &&
                drogon::orm::Criteria(
                    drogon_model::stock_lab::PositionSnapshot::Cols::_user_id,
                    drogon::orm::CompareOperator::EQ,
                    user_id
                ),
                [transaction, response_callback, symbol, user_id](size_t)
                {
                    drogon::orm::Mapper<drogon_model::stock_lab::TransactionRecord> transaction_record_mapper(
                        transaction);
                    transaction_record_mapper.deleteBy(
                        drogon::orm::Criteria(
                            drogon_model::stock_lab::TransactionRecord::Cols::_symbol,
                            drogon::orm::CompareOperator::EQ,
                            symbol
                        ) &&
                        drogon::orm::Criteria(
                            drogon_model::stock_lab::TransactionRecord::Cols::_user_id,
                            drogon::orm::CompareOperator::EQ,
                            user_id
                        ),
                        [transaction, response_callback, symbol, user_id](size_t)
                        {
                            (*response_callback)(dto::JsonResult::ok());
                        },
                        [transaction, response_callback, symbol, user_id](const drogon::orm::DrogonDbException& e)
                        {
                            LOG_ERROR << "failed to delete transaction_records, "
                                << "symbol=" << symbol
                                << ", user_id="
                                << user_id << ", excp="
                                << e.base().what();
                            transaction->rollback();
                            (*response_callback)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                        }
                    );
                },
                [transaction, response_callback, symbol, user_id](const drogon::orm::DrogonDbException& e)
                {
                    LOG_ERROR << "failed to delete position_snapshots, "
                        << "symbol=" << symbol
                        << ", user_id="
                        << user_id << ", excp="
                        << e.base().what();
                    transaction->rollback();
                    (*response_callback)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                }
            );
        }
    );
}
