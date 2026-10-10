//
// Created by yuzhang on 2026/10/7.
//
#include "CronUtil.h"

#include <croncpp.h>
#include <trantor/utils/Logger.h>

namespace util
{
    std::optional<std::chrono::system_clock::time_point> next_time_point(
        const std::string& cron_expr, std::chrono::system_clock::time_point base)
    {
        try
        {
            const auto cron = cron::make_cron<cron::cron_quartz_traits>(cron_expr);
            const auto next = cron::cron_next<cron::cron_quartz_traits>(cron, base);

            if (next == std::chrono::system_clock::time_point::min())
            {
                return std::nullopt;
            }
            return next;
        }
        catch (const cron::bad_cronexpr &e)
        {
            LOG_ERROR << "failed to get next_time_point, e=" << e.what();
            return std::nullopt;
        }
    }

    std::string format_local(std::chrono::system_clock::time_point time_point)
    {
        const std::time_t timestamp = std::chrono::system_clock::to_time_t(time_point);

        std::tm local_time{};
        localtime_r(&timestamp, &local_time);

        std::ostringstream oss;
        oss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    std::string next_execution_time(const std::string& cron_expr)
    {
        const auto next = next_time_point(cron_expr);
        return next.has_value() ? format_local(next.value()) : "";
    }

    bool is_valid_cron(const std::string& cron_expr)
    {
        if (cron_expr.empty())
        {
            return false;
        }

        try
        {
            cron::make_cron<cron::cron_quartz_traits>(cron_expr);
            return true;
        }
        catch (const cron::bad_cronexpr& e)
        {
            LOG_WARN << "invalid cron expression: " << cron_expr << ", e=" << e.what();
            return false;
        }
    }
}
