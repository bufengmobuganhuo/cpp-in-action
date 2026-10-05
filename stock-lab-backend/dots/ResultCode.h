//
// Created by yuzhang on 2026/10/5.
//
#pragma once
#include <string_view>
#include <array>

namespace dto
{
    enum class ResultCode
    {
        OK,
        InvestmentRemindNotUnique,
        ParamError,
        AuthError,
        PlanNotFound,
        RecordsExceedsThreshold,
        PlanExceedsThreshold,
        VerifyCodeExceedsThreshold,
        VerifyCodeExpired,
        VerifyCodeMismatch,
        PositionSnapshotExists,
        PositionSnapshotExceedsThreshold,
        PositionSnapshotDuplicated,
        PositionSnapshotEmpty,
        PositionSnapshotSymbolInvalid,
        EmailNotInvited,
        InternalError
    };

    struct ResultCodeInfo
    {
        ResultCode result_code_;
        int code_;
        std::string_view msg_;
    };

    inline constexpr std::array<ResultCodeInfo, 17> k_result_code_infos
    {
        {
            {ResultCode::OK, 0, "请求成功"},
            {ResultCode::InvestmentRemindNotUnique, 101, "定投提醒已存在"},
            {ResultCode::ParamError, 102, "参数错误"},
            {ResultCode::AuthError, 103, "认证失败"},
            {ResultCode::PlanNotFound, 104, "未找到定投计划"},
            {ResultCode::RecordsExceedsThreshold, 105, "当年定投记录数超过最大值"},
            {ResultCode::PlanExceedsThreshold, 106, "定投计划数超过最大值"},
            {ResultCode::VerifyCodeExceedsThreshold, 107, "5分钟内最多发送一次验证码"},
            {ResultCode::VerifyCodeExpired, 108, "验证码已过期"},
            {ResultCode::VerifyCodeMismatch, 109, "验证失败"},
            {ResultCode::PositionSnapshotExists, 110, "持仓已存在,无法初始化"},
            {ResultCode::PositionSnapshotExceedsThreshold, 111, "持仓数超过最大支持数量"},
            {ResultCode::PositionSnapshotDuplicated, 112, "持仓已存在,无法添加"},
            {ResultCode::PositionSnapshotEmpty, 113, "持仓为空"},
            {ResultCode::PositionSnapshotSymbolInvalid, 114, "持仓标的不合法"},
            {ResultCode::EmailNotInvited, 115, "您还未受邀请，可联系support@mengyv.com处理。"},
            {ResultCode::InternalError, 500, "内部错误"},
        }
    };

    inline constexpr ResultCodeInfo result_code_info(ResultCode result_code)
    {
        for (const auto &info : k_result_code_infos)
        {
            if (info.result_code_ == result_code)
            {
                return info;
            }
        }
        return {ResultCode::InternalError, 500, "内部错误"};
    };

    inline constexpr int code_of(ResultCode result_code)
    {
        return result_code_info(result_code).code_;
    }

    inline constexpr std::string_view msg_of(ResultCode result_code)
    {
        return result_code_info(result_code).msg_;
    }
}
