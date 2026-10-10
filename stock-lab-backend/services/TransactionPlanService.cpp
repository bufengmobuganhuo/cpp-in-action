//
// Created by yuzhang on 2026/10/6.
//

#include "TransactionPlanService.h"

#include <cctype>
#include <ctime>
#include <optional>
#include <sstream>

#include "dots/JsonResult.h"
#include "dots/PageResult.h"
#include "dots/ResultCode.h"
#include "enums/TransactionPlanFrequency.h"
#include "enums/TransactionPlanStatus.h"
#include "repositories/TransactionRecordRepository.h"
#include "utils/CronUtil.h"
#include "vo/TransactionPlanVo.h"

namespace
{
    struct DayOfWeekInfo
    {
        int quartz_value;
        const char* quartz_name;
        const char* desc;
    };

    bool is_digit_string(const std::string& value)
    {
        if (value.empty())
        {
            return false;
        }
        for (const char ch : value)
        {
            if (!std::isdigit(static_cast<unsigned char>(ch)))
            {
                return false;
            }
        }
        return true;
    }

    bool parse_remind_time(const std::string& remind_time, int& hour, int& minute)
    {
        const auto first_colon = remind_time.find(':');
        if (first_colon == std::string::npos)
        {
            return false;
        }

        const auto second_colon = remind_time.find(':', first_colon + 1);
        const std::string hour_part = remind_time.substr(0, first_colon);
        const std::string minute_part = remind_time.substr(
            first_colon + 1,
            second_colon == std::string::npos
                ? std::string::npos
                : second_colon - first_colon - 1
        );

        if (!is_digit_string(hour_part) || !is_digit_string(minute_part))
        {
            return false;
        }

        if (second_colon != std::string::npos)
        {
            const std::string second_part = remind_time.substr(second_colon + 1);
            if (!is_digit_string(second_part))
            {
                return false;
            }
            const int second = std::stoi(second_part);
            if (second < 0 || second > 59)
            {
                return false;
            }
        }

        hour = std::stoi(hour_part);
        minute = std::stoi(minute_part);
        return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
    }

    std::optional<DayOfWeekInfo> parse_day_of_week(const std::optional<int>& week_day)
    {
        if (!week_day.has_value())
        {
            return std::nullopt;
        }

        switch (week_day.value())
        {
        case 1:
            return DayOfWeekInfo{1, "SUN", "周日"};
        case 2:
            return DayOfWeekInfo{2, "MON", "周一"};
        case 3:
            return DayOfWeekInfo{3, "TUE", "周二"};
        case 4:
            return DayOfWeekInfo{4, "WED", "周三"};
        case 5:
            return DayOfWeekInfo{5, "THU", "周四"};
        case 6:
            return DayOfWeekInfo{6, "FRI", "周五"};
        case 7:
            return DayOfWeekInfo{7, "SAT", "周六"};
        default:
            return std::nullopt;
        }
    }

    std::string build_cron(int minute, int hour, const std::string& day_part)
    {
        std::ostringstream oss;
        oss << "0 " << minute << " " << hour << " " << day_part;
        return oss.str();
    }

    std::optional<trantor::Date> parse_date(const std::string& value)
    {
        if (value.empty())
        {
            return std::nullopt;
        }

        std::tm tm{};
        tm.tm_isdst = -1;
        char* end = strptime(value.c_str(), "%Y-%m-%d", &tm);
        if (end == nullptr || *end != '\0')
        {
            return std::nullopt;
        }

        return trantor::Date(mktime(&tm) * 1000000);
    }

    bool is_duplicate_key_error(const drogon::orm::DrogonDbException& e)
    {
        const std::string message = e.base().what();
        return message.find("Duplicate") != std::string::npos ||
            message.find("duplicate") != std::string::npos;
    }

    std::optional<drogon_model::stock_lab::TransactionPlan> build_transaction_plan(
        const dto::TransactionPlanDto& transaction_plan_dto,
        int64_t id,
        int64_t user_id,
        const service::InvestFrequency& invest_frequency)
    {
        drogon_model::stock_lab::TransactionPlan plan;
        plan.setId(id);
        plan.setUserId(user_id);
        plan.setSymbol(transaction_plan_dto.symbol_);
        plan.setAmount(transaction_plan_dto.amount_);
        plan.setFrequency(transaction_plan_dto.frequency_);
        if (transaction_plan_dto.week_order_.has_value())
        {
            plan.setWeekOrder(static_cast<int32_t>(transaction_plan_dto.week_order_.value()));
        }
        if (transaction_plan_dto.week_day_.has_value())
        {
            plan.setWeekDay(static_cast<int32_t>(transaction_plan_dto.week_day_.value()));
        }
        plan.setFrequencyDesc(invest_frequency.frequency_desc_);

        if (!transaction_plan_dto.start_date_.empty())
        {
            auto start_date = parse_date(transaction_plan_dto.start_date_);
            if (!start_date.has_value())
            {
                return std::nullopt;
            }
            plan.setStartDate(start_date.value());
        }

        plan.setRemindTime(transaction_plan_dto.remind_time_);
        plan.setCron(invest_frequency.cron_);
        plan.setStatus(enums::name(transaction_plan_dto.enabled_ ? enums::ENABLED : enums::DISABLED));

        const auto now = trantor::Date::now();
        plan.setCreatedAt(now);
        plan.setUpdatedAt(now);
        return plan;
    }
}

void service::TransactionPlanService::add_transaction_plan(const dto::TransactionPlanDto& transaction_plan_dto,
                                                           const int64_t user_id, ResponseCallback callback)
{
    if (transaction_plan_dto.validate().has_value())
    {
        callback(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }

    auto callback_ptr = std::make_shared<ResponseCallback>(std::move(callback));
    resolve_invest_requency(transaction_plan_dto,
                            [this, transaction_plan_dto, user_id, callback_ptr](
                            InvestFrequency invest_frequency) mutable
                            {
                                check(
                                    transaction_plan_dto,
                                    invest_frequency.cron_,
                                    user_id,
                                    [this, transaction_plan_dto, user_id, invest_frequency, callback_ptr]()
                                    {
                                        drogon_model::stock_lab::TransactionPlan plan;
                                        try
                                        {
                                            const int64_t id = snowflake_.next_id();
                                            std::optional<drogon_model::stock_lab::TransactionPlan> optional_plan =
                                                build_transaction_plan(transaction_plan_dto, id, user_id,
                                                                       invest_frequency);
                                            if (!optional_plan.has_value())
                                            {
                                                (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
                                                return;
                                            }
                                            plan = optional_plan.value();
                                        }
                                        catch (const std::exception& e)
                                        {
                                            LOG_ERROR << "failed to build transaction plan: " << e.what();
                                            (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
                                            return;
                                        }

                                        repository::TransactionPlanRepository::insert(
                                            std::move(plan),
                                            [callback_ptr]()
                                            {
                                                (*callback_ptr)(dto::JsonResult::ok());
                                            },
                                            [callback_ptr](const drogon::orm::DrogonDbException& e)
                                            {
                                                LOG_ERROR << "failed to insert transaction plan: " << e.base().what();
                                                (*callback_ptr)(dto::JsonResult::fail(
                                                    is_duplicate_key_error(e)
                                                        ? dto::ResultCode::InvestmentRemindNotUnique
                                                        : dto::ResultCode::InternalError));
                                            }
                                        );
                                    },
                                    [callback_ptr](drogon::HttpResponsePtr response)
                                    {
                                        (*callback_ptr)(response);
                                    }
                                );
                            },
                            [callback_ptr](drogon::HttpResponsePtr response)
                            {
                                (*callback_ptr)(response);
                            });
}

void service::TransactionPlanService::get_transaction_plans(const int64_t user_id, int page_num, int page_size,
                                                            ResponseCallback callback)
{
    const auto resp_callback_ptr = std::make_shared<ResponseCallback>(std::move(callback));
    std::vector<std::string> statuses(
        enums::DISPLAYABLE_STATUS.begin(),
        enums::DISPLAYABLE_STATUS.end()
    );
    repository::TransactionPlanRepository::select_by_user_id(
        user_id, statuses, page_num, page_size,
        [resp_callback_ptr, page_num, page_size](std::vector<drogon_model::stock_lab::TransactionPlan> rows, size_t cnt)
        {
            dto::PageResult<vo::TransactionPlanVo> page_result;
            page_result.page_num_ = page_num;
            page_result.page_size_ = page_size;
            page_result.total_ = static_cast<int64_t>(cnt);
            for (const auto& row : rows)
            {
                page_result.data_.push_back(vo::TransactionPlanVo::from_model(row));
            }
            (*resp_callback_ptr)(dto::JsonResult::ok(page_result.to_json()));
        },
        [resp_callback_ptr](const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "failed to query transaction plans: " << e.base().what();
            (*resp_callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
        }
    );
}

void service::TransactionPlanService::update_transaction_plan_status(uint64_t id,
                                                                     int64_t user_id,
                                                                     enums::TransactionPlanStatus status,
                                                                     ResponseCallback callback)
{
    auto callback_ptr = std::make_shared<ResponseCallback>(std::move(callback));
    repository::TransactionPlanRepository::update_transaction_plan_status(
        id,
        user_id,
        enums::name(status),
        [callback_ptr]()
        {
            (*callback_ptr)(dto::JsonResult::ok());
        },
        [callback_ptr, id](const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "failed to update transaction plan status, user_id=" << id << ", e=" << e.base().what();
            (*callback_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
        }
    );
}

void service::TransactionPlanService::check(const dto::TransactionPlanDto& transaction_plan_dto,
                                            std::string& cron,
                                            int64_t user_id,
                                            std::function<void()> on_success,
                                            std::function<void(drogon::HttpResponsePtr)> on_error)
{
    auto on_error_ptr = std::make_shared<std::function<void(drogon::HttpResponsePtr)>>(on_error);
    auto on_success_ptr = std::make_shared<std::function<void()>>(on_success);
    if (!util::is_valid_cron(cron))
    {
        (*on_error_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }
    std::optional<dto::Contract> contract = contract_service_.get_by_symbol(transaction_plan_dto.symbol_);
    if (!contract.has_value())
    {
        (*on_error_ptr)(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }
    repository::TransactionPlanRepository::count_by_user_id(
        user_id,
        std::vector<std::string>{enums::DISPLAYABLE_STATUS.begin(), enums::DISPLAYABLE_STATUS.end()},
        [on_success_ptr, on_error_ptr](size_t cnt)
        {
            if (cnt >= MAX_PLAN_COUNT)
            {
                (*on_error_ptr)(dto::JsonResult::fail(dto::ResultCode::PlanExceedsThreshold));
                return;
            }
            (*on_success_ptr)();
        },
        [user_id, on_error_ptr](const drogon::orm::DrogonDbException& e)
        {
            LOG_ERROR << "failed to count transaction plan by user_id=" << user_id << ", e=" << e.base().what();
            (*on_error_ptr)(dto::JsonResult::fail(dto::ResultCode::InternalError));
        });
}

void service::TransactionPlanService::resolve_invest_requency(
    const dto::TransactionPlanDto& transaction_plan_dto, std::function<void(InvestFrequency)> on_success,
    std::function<void(drogon::HttpResponsePtr)> on_error)
{
    const std::optional<enums::TransactionPlanFrequency> frequency =
        enums::parse_frequency(transaction_plan_dto.frequency_);
    if (!frequency.has_value())
    {
        on_error(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }

    int hour = 0;
    int minute = 0;
    if (!parse_remind_time(transaction_plan_dto.remind_time_, hour, minute))
    {
        on_error(dto::JsonResult::fail(dto::ResultCode::ParamError));
        return;
    }

    InvestFrequency invest_frequency;
    switch (frequency.value())
    {
    case enums::DAILY:
        invest_frequency.cron_ = build_cron(minute, hour, "* * ?");
        invest_frequency.frequency_desc_ = "每天";
        on_success(invest_frequency);
        return;
    case enums::WEEKLY:
        {
            const std::optional<DayOfWeekInfo> day_of_week = parse_day_of_week(transaction_plan_dto.week_day_);
            if (!day_of_week.has_value())
            {
                on_error(dto::JsonResult::fail(dto::ResultCode::ParamError));
                return;
            }

            invest_frequency.cron_ = build_cron(minute, hour, std::string("? * ") + day_of_week->quartz_name);
            invest_frequency.frequency_desc_ = std::string("每") + day_of_week->desc;
            on_success(invest_frequency);
            return;
        }
    case enums::MONTHLY:
        {
            const std::optional<DayOfWeekInfo> day_of_week = parse_day_of_week(transaction_plan_dto.week_day_);
            if (!day_of_week.has_value() ||
                !transaction_plan_dto.week_order_.has_value() ||
                transaction_plan_dto.week_order_.value() < 1 ||
                transaction_plan_dto.week_order_.value() > 5)
            {
                on_error(dto::JsonResult::fail(dto::ResultCode::ParamError));
                return;
            }

            std::ostringstream day_part;
            day_part << "? * " << day_of_week->quartz_value << "#" << transaction_plan_dto.week_order_.value();
            invest_frequency.cron_ = build_cron(minute, hour, day_part.str());

            std::ostringstream frequency_desc;
            frequency_desc << "每月第" << transaction_plan_dto.week_order_.value() << "个" << day_of_week->desc;
            invest_frequency.frequency_desc_ = frequency_desc.str();
            on_success(invest_frequency);
            return;
        }
    }

    on_error(dto::JsonResult::fail(dto::ResultCode::ParamError));
}
