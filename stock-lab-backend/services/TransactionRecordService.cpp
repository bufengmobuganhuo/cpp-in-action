//
// Created by yuzhang on 2026/10/7.
//

#include "TransactionRecordService.h"

#include <drogon/HttpAppFramework.h>

#include "dots/JsonResult.h"
#include "dots/PageResult.h"
#include "enums/OrderAction.h"
#include "enums/PositionStatus.h"
#include "repositories/TransactionPlanRepository.h"
#include "repositories/TransactionRecordRepository.h"
#include "utils/Math.h"
#include "utils/Snowflake.h"
#include "utils/TimeUtil.h"
#include "vo/TransactionRecordVo.h"
#include "vo/TransactionStatisticsVo.h"

void service::TransactionRecordService::add_transaction_record(const dto::TransactionRecordDto& transaction_record_dto,
                                                               int64_t user_id, ResponseCallback callback)
{
    auto callback_ptr = std::make_shared<ResponseCallback>(std::move(callback));
    auto record_ptr = std::make_shared<dto::TransactionRecordDto>(transaction_record_dto);
    check(user_id, transaction_record_dto,
          [this, callback_ptr, record_ptr, user_id](bool success, std::optional<std::string> msg)
          {
              if (success)
              {
                  std::optional<trantor::Date> filled_datetime = util::parse_datetime(record_ptr->filled_datetime_);
                  if (!filled_datetime.has_value())
                  {
                      (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
                      return;
                  }

                  util::Decimal filled_qty_decimal((*record_ptr).filled_qty_);
                  util::Decimal filled_price_decimal((*record_ptr).filled_price_);
                  util::Decimal filled_amount = filled_price_decimal * filled_qty_decimal;
                  const int64_t id = util::Snowflake::instance().next_id();

                  drogon_model::stock_lab::TransactionRecord transaction_record = (*record_ptr).to_model(
                      id, user_id,
                      util::format_decimal(filled_amount),
                      filled_datetime.value());

                  auto db_client = drogon::app().getDbClient();
                  db_client->newTransactionAsync(
                      [this, callback_ptr, record_ptr, transaction_record = std::move(transaction_record), user_id](
                      const std::shared_ptr<drogon::orm::Transaction>& transaction) mutable
                      {
                          if (!transaction)
                          {
                              (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                              return;
                          }

                          transaction->setCommitCallback(
                              [callback_ptr](bool committed)
                              {
                                  (*callback_ptr)(committed
                                                      ? dto::JsonResult::ok()
                                                      : dto::JsonResult::fail(dto::ResultCode::InternalError));
                              });

                          drogon::orm::Mapper<drogon_model::stock_lab::TransactionRecord> record_mapper(transaction);
                          record_mapper.insert(
                              transaction_record,
                              [this, transaction, callback_ptr, record_ptr, user_id](
                              const drogon_model::stock_lab::TransactionRecord&)
                              {
                                  drogon::orm::Mapper<drogon_model::stock_lab::PositionSnapshot> snapshot_mapper(
                                      transaction);
                                  snapshot_mapper.findBy(
                                      drogon::orm::Criteria(
                                          drogon_model::stock_lab::PositionSnapshot::Cols::_user_id,
                                          drogon::orm::CompareOperator::EQ,
                                          user_id
                                      ) &&
                                      drogon::orm::Criteria(
                                          drogon_model::stock_lab::PositionSnapshot::Cols::_status,
                                          drogon::orm::CompareOperator::EQ,
                                          enums::name(enums::HOLDING)
                                      ) &&
                                      drogon::orm::Criteria(
                                          drogon_model::stock_lab::PositionSnapshot::Cols::_symbol,
                                          drogon::orm::CompareOperator::EQ,
                                          record_ptr->symbol_
                                      ),
                                      [this, transaction, callback_ptr, record_ptr, user_id](
                                      std::vector<drogon_model::stock_lab::PositionSnapshot> snapshots) mutable
                                      {
                                          std::optional<drogon_model::stock_lab::PositionSnapshot> current_snapshot;
                                          if (!snapshots.empty())
                                          {
                                              current_snapshot = snapshots.front();
                                          }

                                          const auto action = enums::parse_order_action(record_ptr->action_);
                                          if (!action.has_value())
                                          {
                                              transaction->rollback();
                                              (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
                                              return;
                                          }

                                          drogon::orm::Mapper<drogon_model::stock_lab::PositionSnapshot> snapshot_mapper(
                                              transaction);
                                          if (action.value() == enums::SELL)
                                          {
                                              if (!current_snapshot.has_value())
                                              {
                                                  LOG_ERROR << "There is no position to sell, symbol="
                                                      << record_ptr->symbol_ << ", user_id=" << user_id;
                                                  transaction->rollback();
                                                  (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
                                                  return;
                                              }

                                              auto snapshot = current_snapshot.value();
                                              if (!update_position_snapshot_for_sell(snapshot, *record_ptr, user_id))
                                              {
                                                  transaction->rollback();
                                                  (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
                                                  return;
                                              }

                                              snapshot_mapper.update(
                                                  snapshot,
                                                  [transaction](size_t)
                                                  {
                                                  },
                                                  [transaction, callback_ptr, user_id](
                                                  const drogon::orm::DrogonDbException& e)
                                                  {
                                                      LOG_ERROR << "failed to update position snapshot after sell, "
                                                          << "user_id=" << user_id << ", e=" << e.base().what();
                                                      transaction->rollback();
                                                      (*callback_ptr)(dto::JsonResult::fail(
                                                          dto::ResultCode::InternalError));
                                                  });
                                              return;
                                          }

                                          auto updated_snapshot = build_or_update_position_snapshot_for_buy(
                                              current_snapshot,
                                              *record_ptr,
                                              user_id);
                                          if (!updated_snapshot.has_value())
                                          {
                                              transaction->rollback();
                                              (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
                                              return;
                                          }

                                          if (current_snapshot.has_value())
                                          {
                                              snapshot_mapper.update(
                                                  updated_snapshot.value(),
                                                  [transaction](size_t)
                                                  {
                                                  },
                                                  [transaction, callback_ptr, user_id](
                                                  const drogon::orm::DrogonDbException& e)
                                                  {
                                                      LOG_ERROR << "failed to update position snapshot after buy, "
                                                          << "user_id=" << user_id << ", e=" << e.base().what();
                                                      transaction->rollback();
                                                      (*callback_ptr)(dto::JsonResult::fail(
                                                          dto::ResultCode::InternalError));
                                                  });
                                          }
                                          else
                                          {
                                              snapshot_mapper.insert(
                                                  updated_snapshot.value(),
                                                  [transaction](
                                                  const drogon_model::stock_lab::PositionSnapshot&)
                                                  {
                                                  },
                                                  [transaction, callback_ptr, user_id](
                                                  const drogon::orm::DrogonDbException& e)
                                                  {
                                                      LOG_ERROR << "failed to insert position snapshot after buy, "
                                                          << "user_id=" << user_id << ", e=" << e.base().what();
                                                      transaction->rollback();
                                                      (*callback_ptr)(dto::JsonResult::fail(
                                                          dto::ResultCode::InternalError));
                                                  });
                                          }
                                      },
                                      [transaction, callback_ptr, user_id](const drogon::orm::DrogonDbException& e)
                                      {
                                          LOG_ERROR << "failed to select position snapshot, user_id="
                                              << user_id << ", e=" << e.base().what();
                                          transaction->rollback();
                                          (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                                      });
                              },
                              [transaction, callback_ptr, user_id](const drogon::orm::DrogonDbException& e)
                              {
                                  LOG_ERROR << "failed to insert transaction record, user_id="
                                      << user_id << ", e=" << e.base().what();
                                  transaction->rollback();
                                  (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                              });
                      });
              }
              else
              {
                  LOG_ERROR << "check failed, msg=" << msg.value_or("");
                  dto::ResultCodeInfo result_code_info;
                  dto::ResultCodeInfo default_result_info = dto::result_code_info(dto::ResultCode::ParamError);
                  result_code_info.code_ = default_result_info.code_;
                  result_code_info.msg_ = msg.has_value() ? std::string_view(*msg) : default_result_info.msg_;
                  result_code_info.result_code_ = default_result_info.result_code_;
                  (*callback_ptr)(dto::JsonResult::fail(result_code_info));
              }
          });
}

void service::TransactionRecordService::get_transaction_records(const std::string& filled_since,
                                                                const std::string& filled_to, int64_t user_id,
                                                                std::optional<int64_t> plan_id, int64_t last_id,
                                                                int page_size,
                                                                ResponseCallback callback)
{
    if (filled_since.empty() || filled_to.empty() || page_size <= 0)
    {
        callback(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }
    const std::string filled_datetime_since = filled_since + " 00:00:00";
    const std::string filled_datetime_to = filled_to + " 23:59:59";
    auto callback_ptr = std::make_shared<ResponseCallback>(callback);
    repository::TransactionRecordRepository::select_by_page(
        filled_datetime_since,
        filled_datetime_to,
        user_id,
        plan_id,
        last_id,
        page_size,
        [page_size, callback_ptr](std::vector<drogon_model::stock_lab::TransactionRecord> rows, size_t cnt)
        {
            dto::PageResult<vo::TransactionRecordVo> page_result;
            page_result.page_num_ = 1;
            page_result.page_size_ = page_size;
            page_result.total_ = cnt;
            page_result.data_.reserve(rows.size());

            for (const auto& record : rows)
            {
                page_result.data_.push_back(vo::TransactionRecordVo::from_model(record));
            }

            (*callback_ptr)(dto::JsonResult::ok(page_result.to_json()));
        },
        [callback_ptr, user_id, filled_datetime_since, filled_datetime_to, plan_id, last_id](
        const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "failed to get transaction records, filled_since=" << filled_datetime_since
                << ", filled_to=" << filled_datetime_to
                << ", user_id=" << user_id
                << ", plan_id=" << (plan_id.has_value() ? plan_id.value() : -1)
                << ", last_id=" << last_id;
            (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
        }
    );
}

void service::TransactionRecordService::get_invest_statistics(int64_t plan_id, int64_t user_id,
                                                              std::function<void(vo::TransactionStatisticsVo)> callback)
{
    auto callback_ptr = std::make_shared<std::function<void(vo::TransactionStatisticsVo)>>(callback);
    repository::TransactionRecordRepository::statistics_by_plan_id(
        user_id, plan_id,
        [callback_ptr, user_id, plan_id](size_t total_invest_times, std::string total_invest_amount,
                                         std::string total_invest_qty, std::string total_invest_commission)
        {
            repository::TransactionPlanRepository::select_by_plan_id(
                user_id, plan_id,
                [total_invest_times, total_invest_amount, total_invest_qty, total_invest_commission, callback_ptr](
                drogon_model::stock_lab::TransactionPlan plan)
                {
                    util::Decimal plan_amount(plan.getValueOfAmount());
                    util::Decimal next_target_value(plan_amount * (total_invest_times + 1));
                    vo::TransactionStatisticsVo transaction_statistics_vo;
                    transaction_statistics_vo.total_invest_times_ = std::to_string(total_invest_times);
                    transaction_statistics_vo.total_invest_amount_ = util::format_decimal(
                        util::Decimal(total_invest_amount));
                    transaction_statistics_vo.total_invest_qty_ = util::format_decimal(util::Decimal(total_invest_qty));
                    transaction_statistics_vo.total_invest_commission_ = util::format_decimal(
                        util::Decimal(total_invest_commission));
                    transaction_statistics_vo.next_target_value_ = util::format_decimal(next_target_value);
                    (*callback_ptr)(transaction_statistics_vo);
                },
                [callback_ptr, plan_id, user_id](const drogon::orm::DrogonDbException& e)
                {
                    LOG_ERROR << "failed to select_by_plan_id, plan_id=" << plan_id << ", user_id=" << user_id << ", e="
                        << e.base().what();
                }
            );
        },
        [plan_id, user_id](const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "failed to statistics_by_plan_id, plan_id=" << plan_id << ", user_id=" << user_id;
        }
    );
}

void service::TransactionRecordService::check(int64_t user_id, const dto::TransactionRecordDto& record_dto,
                                              std::function<void(bool, std::optional<std::string> msg)> check_result)
{
    std::optional<std::string> msg = record_dto.validate();
    if (msg.has_value())
    {
        check_result(false, msg);
        return;
    }
    repository::TransactionPlanRepository::select_by_plan_id(
        user_id,
        record_dto.plan_id_,
        [check_result = std::move(check_result)](drogon_model::stock_lab::TransactionPlan plan)
        {
            check_result(true, std::nullopt);
        },
        [check_result = std::move(check_result)](const drogon::orm::DrogonDbException& e)
        {
            check_result(false, "执行计划不存在");
        }
    );
}

bool service::TransactionRecordService::update_position_snapshot_for_sell(
    drogon_model::stock_lab::PositionSnapshot& snapshot,
    const dto::TransactionRecordDto& record_dto,
    int64_t user_id)
{
    util::Decimal position_qty(snapshot.getValueOfPositionQuantity());
    util::Decimal filled_qty(record_dto.filled_qty_);
    if (position_qty < filled_qty)
    {
        LOG_ERROR << "transaction qty > position qty, symbol=" << record_dto.symbol_
            << ", user_id=" << user_id;
        return false;
    }

    util::Decimal filled_price(record_dto.filled_price_);
    util::Decimal commission(record_dto.commission_);
    util::Decimal filled_amount = filled_price * filled_qty;
    util::Decimal total_sell_amount(snapshot.getValueOfTotalSellAmount());
    util::Decimal total_commission(snapshot.getValueOfCommission());
    util::Decimal new_position_qty = position_qty - filled_qty;

    snapshot.setTotalSellAmount(util::format_decimal(total_sell_amount + filled_amount));
    snapshot.setPositionQuantity(util::format_decimal(new_position_qty));
    snapshot.setCommission(util::format_decimal(total_commission + commission));
    snapshot.setStatus(enums::name(new_position_qty == 0 ? enums::LIQUIDATED : enums::HOLDING));
    snapshot.setUpdatedAt(trantor::Date::now());
    return true;
}

std::optional<drogon_model::stock_lab::PositionSnapshot>
service::TransactionRecordService::build_or_update_position_snapshot_for_buy(
    const std::optional<drogon_model::stock_lab::PositionSnapshot>& snapshot,
    const dto::TransactionRecordDto& record_dto,
    int64_t user_id)
{
    std::optional<dto::Contract> contract = contract_service_.get_by_symbol(record_dto.symbol_);
    if (!contract.has_value())
    {
        LOG_ERROR << "Contract not found, symbol=" << record_dto.symbol_
            << ", user_id=" << user_id;
        return std::nullopt;
    }

    drogon_model::stock_lab::PositionSnapshot updated_snapshot;
    if (snapshot.has_value())
    {
        updated_snapshot = snapshot.value();
    }
    else
    {
        const trantor::Date now = trantor::Date::now();
        updated_snapshot.setId(util::Snowflake::instance().next_id());
        updated_snapshot.setUserId(user_id);
        updated_snapshot.setSymbol(record_dto.symbol_);
        updated_snapshot.setDescription(contract->description);
        updated_snapshot.setPositionQuantity("0.00000");
        updated_snapshot.setTotalBuyAmount("0.00000");
        updated_snapshot.setTotalSellAmount("0.00000");
        updated_snapshot.setCommission("0.00000");
        updated_snapshot.setDeleted("0");
        updated_snapshot.setCreatedAt(now);
        updated_snapshot.setUpdatedAt(now);
    }

    util::Decimal filled_price(record_dto.filled_price_);
    util::Decimal filled_qty(record_dto.filled_qty_);
    util::Decimal commission(record_dto.commission_);
    util::Decimal filled_amount = filled_price * filled_qty;
    util::Decimal total_buy_amount(updated_snapshot.getValueOfTotalBuyAmount());
    util::Decimal position_qty(updated_snapshot.getValueOfPositionQuantity());
    util::Decimal total_commission(updated_snapshot.getValueOfCommission());

    updated_snapshot.setTotalBuyAmount(util::format_decimal(total_buy_amount + filled_amount));
    updated_snapshot.setPositionQuantity(util::format_decimal(position_qty + filled_qty));
    updated_snapshot.setCommission(util::format_decimal(total_commission + commission));
    updated_snapshot.setStatus(enums::name(enums::HOLDING));
    updated_snapshot.setUpdatedAt(trantor::Date::now());
    return updated_snapshot;
}
