//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <functional>
#include <string>

namespace service
{
    class MailService
    {
    private:
        std::string get_resend_api_key() const;
        std::string get_mail_from() const;
        std::string get_app_name() const;
        std::string get_verify_code_template_path() const;
        std::string build_verify_code_html(int verify_code) const;
        std::string load_template(const std::string &path) const;
        std::string current_year() const;
        static void replace_all(std::string &text, const std::string &from, const std::string &to);
    public:
        using Callback = std::function<void(bool success)>;
        void send_verify_code_email(
            const std::string &email,
            int verify_code,
            Callback callback
        ) const;
    };
}
