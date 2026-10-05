下面是一份可直接作为 Markdown 文档使用的规划稿。

# stock-lab-backend C++ 重写路线图

## 1. 目标

使用 C++17重写现有 Java Spring Boot 项目 `stock-lab-backend`(文件目录：/Users/yuzhang/IdeaProjects/stock-lab-backend)，保持现有 Flutter 前端接口基本兼容。

现有 Java 项目核心能力：

- 邮箱验证码登录
- JWT 鉴权
- 股票合约查询
- 定投计划管理
- 交易记录管理
- 持仓快照统计
- 定时邮件提醒
- MySQL 数据持久化

## 2. 推荐技术选型

### 2.1 Web 框架

推荐使用：
```text
Drogon
```

选择原因：

- 支持 C++17，符合项目约束
- 内置 HTTP Server、路由、Filter、中间件能力
- 支持 JSON
- 支持 MySQL / PostgreSQL / Redis
- 自带异步数据库访问与轻量 ORM
- 比手写 socket/reactor 更适合业务后端
- 比 TARS、brpc、tRPC 更适合直接重写 REST API 项目

### 2.2 数据库访问

推荐使用：
```text
Drogon DbClient / Mapper
```
原因：

- 和 Drogon 集成度高
- 支持 MySQL
- 可以先用手写 SQL，后续再逐步抽象 Repository
- 比直接使用 MySQL Connector/C++ 更适合新手快速落地

### 2.3 JSON

推荐使用：
```text
Drogon 内置 JSON / JsonCpp
```
### 2.4 JWT

推荐使用：
```text
jwt-cpp
```
用于实现：

- 登录后签发 Token
- 请求拦截时验证 Token
- 从 Token 中解析 userId

### 2.5 HTTP Client

推荐使用：
```text
Drogon HttpClient
```
用于调用：

- Finnhub 股票搜索接口
- Finnhub 行情报价接口
- Resend 邮件发送接口

### 2.6 定时任务

第一版推荐：
```text
后台线程 + 每分钟扫描 MySQL
```
不建议第一版直接实现 Quartz 等价能力。

后续可选：
```text
croncpp / 自研 cron 解析 / 独立调度服务
```
### 2.7 构建工具

推荐：
```text
CMake + vcpkg
```
原因：

- C++ 项目主流组合
- CLion 支持好
- 依赖管理比手动编译简单

## 4. 推荐项目结构
```text
stock-lab-cpp/
  CMakeLists.txt

  config/
    config.dev.json
    config.prod.json

  src/
    main.cc

    controllers/
      AuthController.h
      AuthController.cc
      ContractController.h
      ContractController.cc
      TransactionPlanController.h
      TransactionPlanController.cc
      TransactionRecordController.h
      TransactionRecordController.cc
      PositionSnapshotController.h
      PositionSnapshotController.cc

    filters/
      AuthFilter.h
      AuthFilter.cc

    services/
      AuthService.h
      AuthService.cc
      ContractService.h
      ContractService.cc
      TransactionPlanService.h
      TransactionPlanService.cc
      TransactionRecordService.h
      TransactionRecordService.cc
      PositionSnapshotService.h
      PositionSnapshotService.cc
      MailService.h
      MailService.cc
      SchedulerService.h
      SchedulerService.cc

    repositories/
      UserRepository.h
      UserRepository.cc
      EmailWhitelistRepository.h
      EmailWhitelistRepository.cc
      TransactionPlanRepository.h
      TransactionPlanRepository.cc
      TransactionRecordRepository.h
      TransactionRecordRepository.cc
      PositionSnapshotRepository.h
      PositionSnapshotRepository.cc

    models/
      User.h
      TransactionPlan.h
      TransactionRecord.h
      PositionSnapshot.h

    dto/
      JsonResult.h
      PageResult.h
      LoginDto.h
      TransactionPlanDto.h
      TransactionRecordDto.h

    utils/
      JwtUtil.h
      JwtUtil.cc
      Snowflake.h
      Snowflake.cc
      TimeUtil.h
      TimeUtil.cc
      DecimalUtil.h
      DecimalUtil.cc
```
## 5. Java 模块到 C++ 模块映射

| Java 模块                         | C++ 模块               |
| --------------------------------- | ---------------------- |
| `web/controller`                  | `controllers`          |
| `web/interceptor/AuthInterceptor` | `filters/AuthFilter`   |
| `web/service/AuthService`         | `services/AuthService` |
| `service/impl`                    | `services`             |
| `model/dao`                       | `repositories`         |
| `model/entity`                    | `models`               |
| `service/dto`、`service/vo`       | `dto`                  |
| `Quartz Job`                      | `SchedulerService`     |
| `JsonResult`                      | `dto/JsonResult`       |
| `JwtTokenUtil`                    | `utils/JwtUtil`        |

## 6. 迁移步骤

### 阶段 1：冻结接口契约

目标：

- 整理现有 Java API
- 明确请求路径、请求参数、响应字段、错误码
- 保持 Flutter 前端无需大改

需要整理的接口：
```text
POST   /api/auth/verifyCode
POST   /api/auth/login

GET    /api/contract/base-info

POST   /api/transaction/plan
PUT    /api/transaction/plan/status
DELETE /api/transaction/plan
GET    /api/transaction/plan/list

POST   /api/transaction/record
GET    /api/transaction/record/list
GET    /api/transaction/record/list-by-plan
GET    /api/transaction/record/statistic

GET    /api/position-snapshot/list
DELETE /api/position-snapshot/{symbol}
```
统一响应格式：
```json
{
  "code": 0,
  "msg": "请求成功",
  "data": {}
}
```
### 阶段 2：搭建 C++ Drogon 空项目

目标：

- 创建独立 C++ 项目
- 启动 HTTP 服务
- 支持配置文件
- 支持统一 JSON 返回
- 支持 MySQL 连接

最小验证接口：
```text
GET /api/health
```
返回：
```json
{
  "code": 0,
  "msg": "请求成功",
  "data": "ok"
}
```
### 阶段 3：迁移认证模块

对应 Java：
```text
AuthController
AuthServiceImpl
JwtTokenUtil
AuthInterceptor
```
实现功能：

- 邮箱格式校验
- 邮箱白名单校验
- 生成 6 位验证码
- 验证码 5 分钟过期
- 5 分钟内限制重复发送
- 调用 Resend 发送邮件
- 登录成功后自动注册用户
- 生成 JWT
- AuthFilter 验证 Bearer Token

优先迁移原因：

- 模块边界清晰
- 表结构简单
- 可以快速形成闭环
- 后续接口都依赖 userId

### 阶段 4：迁移股票合约查询模块

对应 Java：
```text
ContractController
ContractQueryServiceImpl
```
实现功能：

- 调用 Finnhub search API
- 根据 symbol 查询合约
- 为定投计划和交易记录提供 symbol 校验

接口：
```text
GET /api/contract/base-info?keyword=AAPL
```
### 阶段 5：迁移定投计划模块

对应 Java：
```text
TransactionPlanController
TransactionPlanServiceImpl
TransactionPlanDao
```
实现功能：

- 新增定投计划
- 查询定投计划列表
- 修改计划状态
- 删除计划，逻辑删除为 `DELETED`
- 每个用户最多 10 个计划
- 支持频率：
  - `DAILY`
  - `WEEKLY`
  - `MONTHLY`

注意：

第一版可以先保存 cron 字符串，但不急着实现完整 Quartz 等价调度。

### 阶段 6：迁移交易记录模块

对应 Java：
```text
TransactionRecordController
TransactionRecordServiceImpl
TransactionRecordDao
```
实现功能：

- 新增买入 / 卖出记录
- 按日期分页查询交易记录
- 按计划查询交易记录
- 统计某计划的总次数、金额、数量、手续费、下次目标金额
- 按计划频率限制年度记录数：
  - 日投：366
  - 周投：53
  - 月投：12

关键风险：

交易记录写入后会更新持仓快照，必须使用 MySQL 事务保证一致性。

事务范围：
```text
insert transaction_record
update / insert position_snapshot
commit
```
失败时必须 rollback。

### 阶段 7：迁移持仓快照模块

对应 Java：
```text
PositionSnapshotController
PositionSnapshotServiceImpl
PositionSnapshotDao
```
实现功能：

- 查询当前持仓
- 根据交易记录维护持仓数量
- 买入更新：
  - 持仓数量
  - 总买入金额
  - 佣金
- 卖出更新：
  - 持仓数量
  - 总卖出金额
  - 佣金
  - 清仓后状态变为 `LIQUIDATED`
- 调用 Finnhub quote API 获取最新价格
- 本地缓存最新价格 1 分钟
- 计算：
  - 净投入金额
  - 摊薄成本
  - 总收益
  - 参考收益率
  - 真实收益率
  - 持仓占比

注意：

金额和数量不要直接使用 `double` 做核心计算。建议：

- 数据库存储继续使用 `DECIMAL`
- C++ 层优先用字符串承接金额
- 必要时引入 decimal 库
- 至少保证最终写库和返回前按固定精度处理

### 阶段 8：迁移定时提醒模块

对应 Java：
```text
TransactionPlanLoader
TransactionPlanScheduler
TransactionPlanRemindJob
```
第一版推荐实现：
```text
后台线程每 60 秒扫描 ENABLED 计划
判断是否达到提醒时间
调用 MailService 发送邮件
记录提醒日志，避免重复发送
```
不建议第一版实现完整 Quartz cron 行为。

后续增强：

- 支持 cron 表达式解析
- 支持进程重启恢复
- 支持失败重试
- 支持提醒发送记录表
- 支持幂等发送

## 7. 推荐迁移顺序
```text
1. API 契约整理
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
```
## 8. 灰度发布方案

不要一次性替换 Java 后端。

推荐 Nginx 分路径切换：
```text
第一步：/api/auth/**              -> C++ 服务
第二步：/api/contract/**          -> C++ 服务
第三步：/api/transaction/plan/**  -> C++ 服务
第四步：/api/transaction/record/** -> C++ 服务
第五步：/api/position-snapshot/** -> C++ 服务
```
Java 与 C++ 可以临时共用同一个 MySQL。

注意：

定时提醒任务只能保留一个服务执行，不能 Java 和 C++ 同时执行，否则可能重复发送邮件。

## 9. 主要风险

### 9.1 C++ 学习成本

C++ 没有 Spring Boot 那样完整的自动装配生态，需要自己处理：

- 对象生命周期
- 依赖组织
- 错误处理
- 事务边界
- JSON 字段转换
- 配置管理

### 9.2 金额精度风险

Java 使用 `BigDecimal`，C++ 如果直接用 `double`，可能出现精度误差。

建议：

- MySQL 继续使用 `DECIMAL`
- C++ 层封装 `DecimalUtil`
- 所有金额字段统一处理精度

### 9.3 定时任务重复执行

后台任务涉及：

- 时间判断
- 重复发送
- 进程重启
- 多实例部署

第一版应保持单实例部署，并增加提醒记录表或幂等控制。

### 9.4 事务一致性

交易记录与持仓快照必须在同一个事务中完成。

否则可能出现：

- 交易记录已写入，但持仓未更新
- 持仓已更新，但交易记录失败
- 卖出数量校验不一致

### 9.5 接口兼容风险

Flutter 前端可能依赖：

- 字段名
- 时间格式
- 错误码
- 空字段是否返回
- 分页结构

迁移前必须先固定 API 契约。

## 10. 回滚方式

每个阶段都应该可以独立回滚。

推荐策略：

- C++ 项目独立目录开发
- 不直接修改 Java 项目
- Nginx 分路径灰度
- 每次只切换一个 API 模块
- 出现问题时，将对应路径代理回 Java 服务

示例：
```text
/api/auth/** 从 C++ 回滚到 Java
/api/contract/** 继续保持 C++
```
## 11. 第一阶段建议任务清单

第一阶段只做最小闭环：

- 创建 `stock-lab-cpp` 项目
- 配置 CMake
- 引入 Drogon
- 实现 `/api/health`
- 实现 `JsonResult`
- 实现 MySQL 连接
- 实现 `ResultCode`
- 实现 Auth 模块：
  - `/api/auth/verifyCode`
  - `/api/auth/login`
  - JWT 生成
  - AuthFilter 验证

完成标准：

- C++ 服务可以启动
- 可以连接 MySQL
- 可以发送验证码
- 可以登录成功并返回 JWT
- 受保护接口可以解析出 userId

## 12. 总结

推荐路线：
```text
C++17 + Drogon + MySQL + jwt-cpp + CMake + vcpkg
```
迁移策略：
```text
先认证
再外部查询
再 CRUD
再交易事务
再持仓计算
最后定时任务
```
