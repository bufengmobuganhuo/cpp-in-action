//
// Created by yuzhang on 2026/10/7.
//

#pragma once
#include <optional>
#include <chrono>
#include <string>

namespace util
{
    std::optional<std::chrono::system_clock::time_point> next_time_point(
        const std::string& cron_expr,
        std::chrono::system_clock::time_point base = std::chrono::system_clock::now()
    );

    std::string format_local(std::chrono::system_clock::time_point time_point);

    std::string next_execution_time(const std::string& cron_expr);

    bool is_valid_cron(const std::string& cron_expr);
}
