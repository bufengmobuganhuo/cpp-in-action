# AGENTS.md

## 项目背景

本项目是将 Java Spring Boot 版 `stock-lab-backend` 重写为 C++17 + Drogon 后端的迁移工程。

每次新会话开始处理本仓库问题时，优先阅读并参考：

- `stock-lab-backend C++ 重写路线图.md`

Java 原项目路径：

- `/Users/yuzhang/IdeaProjects/stock-lab-backend`

除非用户明确说明不需要，否则涉及接口、业务逻辑、错误码、DTO、数据库访问、邮件、JWT、定时任务等问题时，应主动对照 Java 原项目实现，保持 Flutter 前端接口基本兼容。

## 协作规则

- 收到实现类任务后，先调研代码和路线图，再给出方案。
- 未得到用户明确同意前，禁止修改文件，禁止运行会改仓库的命令。
- 方案必须写清：要改哪些文件、为什么、风险、回滚方式。
- 有多种做法时给出 2-3 个选项，并标明推荐项。
- 只有用户明确说“同意”、“按方案实施”、“开始改”、“直接改”后，才进入实现。
- 实现过程中如果方案需要变化，先停止并说明，再等确认。

## 技术方向

- Web 框架使用 Drogon。
- 数据库访问优先使用 Drogon DbClient / Mapper。
- 简单单表访问放到 `repositories/`，业务流程放到 `services/`，HTTP 路由放到 `controllers/`。
- 统一响应使用 `dto::JsonResult`，保持结构：
  ```json
  {
    "code": 0,
    "msg": "请求成功",
    "data": null
  }
  ```
- 错误码应集中在 `dto::ResultCode`，并尽量与 Java 项目 `ResultCode` 保持兼容。
- DTO 放在 `dto/` 或当前项目已有的 DTO 目录中，避免把业务校验散落在 Controller。
- Drogon 生成的 `models/` 文件不要手工改，业务封装应放到 Repository 或 Service。
- HTTP Client 使用 Drogon HttpClient。
- 第一版定时任务使用后台线程 + MySQL 扫描，不实现完整 Quartz 等价能力。
- 金额字段不要用 `double` 做核心计算，优先用字符串承接数据库 DECIMAL，后续再抽 `DecimalUtil`。

## 迁移顺序

优先遵循路线图中的阶段：

1. 冻结接口契约
2. Drogon 空项目
3. JsonResult / ResultCode
4. MySQL 连接
5. Auth 登录鉴权
6. Contract 股票查询
7. TransactionPlan 定投计划
8. TransactionRecord 交易记录
9. PositionSnapshot 持仓快照
10. Scheduler 定时提醒
11. Nginx 灰度切换
12. Java 后端下线

当前实现阶段应围绕路线图推进，不做无关重构。

## 接口兼容要求

迁移接口时优先保持 Java 后端和 Flutter 前端兼容：

- 请求路径一致
- 请求参数名一致
- 响应字段一致
- 错误码和错误信息尽量一致
- 时间格式、分页结构、空字段行为需谨慎处理

阶段 3 认证模块重点接口：

- `POST /api/auth/verifyCode`
- `POST /api/auth/login`

## 配置与敏感信息

- 不要把真实 API key、数据库密码、JWT secret 等敏感信息写入回答或新文件。
- 如果发现敏感信息已在配置中，提醒用户后续迁移到环境变量或本地私有配置。
- 修改配置前说明生效文件。目前 `main.cc` 默认加载 `config.json`。

## 代码风格

- 头文件避免 `using namespace`。
- 模板类实现通常放在 `.h`。
- Controller 方法通过 Drogon callback 返回 `HttpResponsePtr`。
- Service 不直接承担 HTTP 路由职责。
- Repository 负责数据访问，Service 负责业务流程。
- C++ 命名风格优先贴合当前项目已有代码，不做大范围统一重命名。
