//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <functional>
#include <optional>
#include <string>

namespace service
{
    struct TransactionPlanReminderEmail
    {
        std::string symbol;
        std::string amount;
        std::string frequency_desc;
        std::string total_invest_times;
        std::string total_invest_amount;
        std::optional<std::string> target_total_value;
    };

    class MailService
    {
    private:
        std::string get_resend_api_key() const;
        std::string get_mail_from() const;
        std::string get_app_name() const;
        std::string get_verify_code_template_path() const;
        std::string get_transaction_remind_template_path() const;
        std::string build_verify_code_html(int verify_code) const;
        std::string build_transaction_remind_html(const TransactionPlanReminderEmail &email) const;
        std::string load_template(const std::string &path) const;
        std::string current_year() const;
        static void replace_all(std::string &text, const std::string &from, const std::string &to);
        static std::string escape_html(const std::string &text);
    public:
        using Callback = std::function<void(bool success)>;
        void send_verify_code_email(
            const std::string &email,
            int verify_code,
            Callback callback
        ) const;
        void send_transaction_plan_remind_email(
            const std::string &email,
            const TransactionPlanReminderEmail &reminder,
            Callback callback
        ) const;
    };
}
