//
// Created by yuzhang on 2026/10/7.
//

#pragma once
#include <optional>
#include <string>
#include <json/json.h>

#include "ModelAliases.h"
#include "utils/CronUtil.h"

namespace vo
{
    struct TransactionPlanVo
    {
        std::string id_;
        std::string symbol_;
        std::string name_;
        std::string amount_;
        std::string frequency_;
        std::string frequency_desc_;
        std::optional<int> week_day_;
        std::optional<int> week_order_;
        std::string remind_time_;
        std::string start_date_;
        std::string next_execute_time_;
        bool enabled_{false};

        static std::string strip_trailing_zeros(std::string value)
        {
            const auto dot_pos = value.find('.');
            if (dot_pos == std::string::npos)
            {
                return value;
            }

            while (!value.empty() && value.back() == '0')
            {
                value.pop_back();
            }
            if (!value.empty() && value.back() == '.')
            {
                value.pop_back();
            }
            return value.empty() ? "0" : value;
        }

        static std::string format_time_hh_mm(const std::string& value)
        {
            return value.size() >= 5 ? value.substr(0, 5) : value;
        }

        static std::string format_date_yyyy_mm_dd(const trantor::Date& value)
        {
            const std::string db_time = value.toDbStringLocal();
            return db_time.size() >= 10 ? db_time.substr(0, 10) : db_time;
        }

        static TransactionPlanVo from_model(const drogon_model::stock_lab::TransactionPlan& plan)
        {
            TransactionPlanVo vo;
            vo.id_ = std::to_string(plan.getValueOfId());
            vo.symbol_ = plan.getValueOfSymbol();

            if (plan.getAmount())
            {
                vo.amount_ = strip_trailing_zeros(plan.getValueOfAmount());
            }
            if (plan.getFrequency())
            {
                vo.frequency_ = plan.getValueOfFrequency();
            }
            if (plan.getFrequencyDesc())
            {
                vo.frequency_desc_ = plan.getValueOfFrequencyDesc();
            }
            if (plan.getWeekDay())
            {
                vo.week_day_ = plan.getValueOfWeekDay();
            }
            if (plan.getWeekOrder())
            {
                vo.week_order_ = plan.getValueOfWeekOrder();
            }
            if (plan.getCron())
            {
                vo.next_execute_time_ = util::next_execution_time(plan.getValueOfCron());
            }
            if (plan.getRemindTime())
            {
                vo.remind_time_ = format_time_hh_mm(plan.getValueOfRemindTime());
            }
            if (plan.getStartDate())
            {
                vo.start_date_ = format_date_yyyy_mm_dd(*plan.getStartDate());
            }
            if (plan.getStatus())
            {
                vo.enabled_ = plan.getValueOfStatus() == "ENABLED";
            }

            return vo;
        }

        Json::Value to_json() const
        {
            Json::Value root;
            root["id"] = id_;
            root["symbol"] = symbol_;

            if (!name_.empty())
            {
                root["name"] = name_;
            }
            if (!amount_.empty())
            {
                root["amount"] = amount_;
            }
            if (!frequency_.empty())
            {
                root["frequency"] = frequency_;
            }
            if (!frequency_desc_.empty())
            {
                root["frequencyDesc"] = frequency_desc_;
            }
            if (week_day_.has_value())
            {
                root["weekDay"] = week_day_.value();
            }
            if (week_order_.has_value())
            {
                root["weekOrder"] = week_order_.value();
            }
            if (!remind_time_.empty())
            {
                root["remindTime"] = remind_time_;
            }
            if (!start_date_.empty())
            {
                root["startDate"] = start_date_;
            }
            if (!next_execute_time_.empty())
            {
                root["nextExecuteTime"] = next_execute_time_;
            }

            root["enabled"] = enabled_;
            return root;
        }
    };
}
