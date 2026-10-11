# stock-lab-backend C++ 生产部署文档

本文档用于将 C++17 + Drogon 版 `stock-lab-backend` 部署到生产服务器。当前服务器系统按以下环境编写：

```text
Amazon Linux 2023.10
```

后端默认监听：

```text
0.0.0.0:8088
```

推荐生产架构：

```text
用户 -> AWS EC2 (Nginx :80/:443)
        ├── /      -> Flutter 静态文件 (/var/www/stock-lab)
        └── /api/  -> 反向代理到 127.0.0.1:8088 (Drogon C++ 后端)
```

前端和 API 同域部署时，Flutter 侧无需额外配置 CORS。

## 一、部署前检查

### 1. 数据库

C++ 版本复用 Java 项目的 MySQL 表结构。当前仓库没有独立 SQL migration 文件，因此部署前需要确认生产 MySQL 中已存在 Java 项目使用的库表和基础数据。

至少确认：

```text
users
email_whitelist
transaction_plans
transaction_records
position_snapshots
```

如果生产库仍由 Java 版本使用，建议先完整备份数据库，再切换流量。

### 2. 外部服务

后端依赖：

```text
MySQL
Finnhub API
Resend Email API
```

生产环境需要准备：

```text
DB_HOST
DB_NAME
DATASOURCE_USERNAME
DATASOURCE_PASSWORD
FINNHUB_TOKEN
MAIL_API_KEY
```

注意：`config/config.prod.json` 当前读取的是 `DATASOURCE_USERNAME` 和 `DATASOURCE_PASSWORD`。如果你本地 `.env` 里使用的是 `DB_USERNAME`、`DB_PASSWORD`，生产部署时需要额外补充 `DATASOURCE_USERNAME`、`DATASOURCE_PASSWORD`，或统一修改配置文件里的变量名。

不要把真实密钥提交到 Git。生产服务器上使用 `.env` 保存，或改为 systemd `EnvironmentFile` 管理。

### 3. 端口与安全组

服务器安全组建议只开放：

```text
22    SSH，仅允许自己的 IP
80    HTTP
443   HTTPS
```

`8088` 只给本机 Nginx 访问，不建议对公网开放。

## 二、服务器初始化

### 1. 登录服务器

```bash
ssh -i your-key.pem ec2-user@your-ec2-public-ip
```

### 2. 更新系统

```bash
sudo dnf update -y
```

### 3. 安装基础工具

```bash
sudo dnf install -y \
  git \
  gcc \
  gcc-c++ \
  cmake \
  ninja-build \
  make \
  tmux \
  tar \
  zip \
  unzip \
  pkgconf-pkg-config \
  kernel-headers \
  perl-core \
  perl-IPC-Cmd \
  nasm \
  autoconf \
  automake \
  libtool \
  openssl-devel \
  zlib-devel \
  nginx
```

Amazon Linux 2023 默认可能已安装 `curl-minimal`，它会和完整版 `curl` 包冲突。文档中的 `curl` 验证命令使用系统已有的 `curl-minimal` 即可，不需要额外安装 `curl`。

启动 Nginx：

```bash
sudo systemctl enable --now nginx
```

## 三、安装 C++ 依赖

推荐使用 vcpkg 管理 Drogon、jwt-cpp、Boost、croncpp 等依赖，避免 Amazon Linux 系统包版本差异。

### 1. 安装 vcpkg

```bash
cd ~
git clone https://github.com/microsoft/vcpkg.git
cd ~/vcpkg
./bootstrap-vcpkg.sh
```

如果输出了 vcpkg 版本号，例如：

```text
vcpkg package management program version ...
```

说明 bootstrap 成功。后面的 telemetry 提示不是错误。

### 2. 小内存 EC2 先增加 swap

如果服务器内存只有 1GB 左右，vcpkg 编译 Drogon、OpenSSL 等依赖时可能触发 OOM，表现为编译中断、SSH 卡死或实例短时间连不上。建议先增加 4GB swap：

```bash
free -h
swapon --show

sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

# 持久化，重启后仍生效
echo '/swapfile swap swap defaults 0 0' | sudo tee -a /etc/fstab

free -h
swapon --show
```

建议在 `tmux` 中执行耗时编译，避免 SSH 断开导致命令中断：

```bash
tmux new -s vcpkg-build
```

如果 SSH 断开，重新登录后恢复会话：

```bash
tmux attach -t vcpkg-build
```

### 3. 安装项目依赖

先验证 OpenSSL 编译依赖中的 Perl 模块是否可用：

```bash
perl -MIPC::Cmd -e 'print "IPC::Cmd OK\n"'
```

如果这里报 `Can't locate IPC/Cmd.pm`，重新安装：

```bash
sudo dnf install -y kernel-headers perl-core perl-IPC-Cmd
```

再安装 vcpkg 依赖。显式指定 `x64-linux`，避免依赖装到其他 triplet 后 CMake 找不到：

```bash
VCPKG_DISABLE_METRICS=1 VCPKG_MAX_CONCURRENCY=1 ~/vcpkg/vcpkg install \
  'drogon[ctl,mysql]:x64-linux' \
  jwt-cpp:x64-linux \
  boost-headers:x64-linux \
  boost-multiprecision:x64-linux \
  croncpp:x64-linux
```

安装完成后确认关键依赖存在：

```bash
~/vcpkg/vcpkg list | grep -E 'drogon|jwt-cpp|boost-multiprecision|croncpp'
find ~/vcpkg/installed -iname '*Drogon*Config*.cmake' -o -iname '*drogon*config*.cmake'
find ~/vcpkg -type f -name drogon_ctl
```

## 四、部署后端代码

以下目录约定：

```text
/opt/stock-lab/backend/current
```

### 1. 创建部署目录

```bash
sudo mkdir -p /opt/stock-lab/backend
sudo chown -R ec2-user:ec2-user /opt/stock-lab
```

### 2. 上传或拉取代码

方式一：服务器直接拉取 Git 仓库：

```bash
cd /opt/stock-lab/backend
git clone your-repo-url current
cd current
```

方式二：本地打包上传：

必须在本项目根目录执行打包命令，否则服务器解压后的目录结构可能不正确。

```bash
# 本地执行
cd /Users/yuzhang/clion-projects/cpp-in-action/stock-lab-backend
rm -f /tmp/stock-lab-backend.tar.gz
tar --exclude='.git' \
  --exclude='cmake-build-*' \
  --exclude='build' \
  --exclude='*.tar.gz' \
  -czf /tmp/stock-lab-backend.tar.gz .
scp -i your-key.pem /tmp/stock-lab-backend.tar.gz ec2-user@your-ec2-public-ip:/opt/stock-lab/backend/

# 服务器执行
cd /opt/stock-lab/backend
mkdir -p current
tar -xzf stock-lab-backend.tar.gz -C current
cd current
```

## 五、生产配置

项目启动时会：

1. 从当前工作目录向上查找项目根目录。
2. 读取项目根目录下的 `.env`。
3. 读取 `config/config.json`。
4. 根据 `PROFILES_ACTIVE` 或 `STOCK_LAB_ENV` 合并 `config/config.<profile>.json`。

生产环境应使用：

```text
PROFILES_ACTIVE=prod
```

### 1. 创建 `.env`

在服务器上执行：

```bash
cd /opt/stock-lab/backend/current
vi .env
```

内容模板：

```dotenv
PROFILES_ACTIVE=prod
STOCK_LAB_ENV=prod

DB_HOST=your-mysql-host
DB_NAME=your-database-name
DATASOURCE_USERNAME=your-database-user
DATASOURCE_PASSWORD=your-database-password

# 可选：如果开发环境 config.dev.json 仍使用 DB_USERNAME / DB_PASSWORD，可同步保留
DB_USERNAME=your-database-user
DB_PASSWORD=your-database-password

FINNHUB_TOKEN=your-finnhub-token
MAIL_API_KEY=your-resend-api-key
```

设置权限：

```bash
chmod 600 .env
```

### 2. 检查生产配置文件

确认存在：

```bash
ls -la config/config.json config/config.prod.json
```

`config/config.prod.json` 应通过环境变量占位符读取数据库配置，不要写死真实密码。

## 六、编译后端

在服务器项目目录执行：

```bash
cd /opt/stock-lab/backend/current

# 如果 CMake 配置阶段曾失败，先删除旧缓存再重新配置
rm -rf build

cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DDROGON_CTL_COMMAND=$HOME/vcpkg/installed/x64-linux/tools/drogon/drogon_ctl

cmake --build build -j1
```

编译完成后确认二进制存在：

```bash
ls -lh build/stock-lab-backend
```

## 七、手动启动验证

首次部署建议先手动启动，确认配置、数据库和外部 API 没有问题。

重要：当前邮件模板路径为 `../templates/email/...`，请在 `build` 目录启动后端。

```bash
cd /opt/stock-lab/backend/current/build
./stock-lab-backend
```

另开一个 SSH 窗口验证：

```bash
curl -i http://127.0.0.1:8088/api/auth/login \
  -X POST \
  -H 'Content-Type: application/json' \
  -d '{}'
```

能收到 JSON 响应即可说明服务已启动。即使参数不完整返回业务错误，也说明 Nginx 前的后端进程可访问。

如果启动失败，优先检查：

```bash
cd /opt/stock-lab/backend/current
grep -E '^(PROFILES_ACTIVE|STOCK_LAB_ENV|DB_HOST|DB_NAME|DATASOURCE_USERNAME|DATASOURCE_PASSWORD|FINNHUB_TOKEN|MAIL_API_KEY)=' .env \
  | sed -E 's/=.*/=***MASKED***/'
ls -la config/
ls -la templates/email/
```

不要在排查记录或截图里暴露 `.env` 真实密钥。

## 八、配置 systemd

生产环境建议用 systemd 托管后端。

### 1. 创建服务文件

```bash
sudo vi /etc/systemd/system/stock-lab-backend.service
```

写入：

```ini
[Unit]
Description=Stock Lab C++ Backend
After=network.target

[Service]
Type=simple
User=ec2-user
Group=ec2-user
WorkingDirectory=/opt/stock-lab/backend/current/build
ExecStart=/opt/stock-lab/backend/current/build/stock-lab-backend
Restart=always
RestartSec=5

# 如需覆盖 .env，可在这里额外指定环境变量：
# Environment=PROFILES_ACTIVE=prod

StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

`WorkingDirectory` 必须保持为 `build` 目录，否则当前邮件模板相对路径可能无法解析。

### 2. 启动服务

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now stock-lab-backend
```

### 3. 查看状态和日志

```bash
sudo systemctl status stock-lab-backend --no-pager
sudo journalctl -u stock-lab-backend -f
```

### 4. 重启服务

```bash
sudo systemctl restart stock-lab-backend
```

## 九、配置 Nginx

Amazon Linux 2023 的 Nginx 默认加载：

```text
/etc/nginx/conf.d/*.conf
```

### 1. 创建站点配置

```bash
sudo vi /etc/nginx/conf.d/stock-lab.conf
```

仅 HTTP 配置：

```nginx
server {
    listen 80 default_server;
    server_name your-domain.com www.your-domain.com;

    root /var/www/stock-lab;
    index index.html;

    location / {
        try_files $uri $uri/ /index.html;
    }

    location /api/ {
        proxy_pass http://127.0.0.1:8088/api/;
        proxy_http_version 1.1;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

如果只用公网 IP 测试，可以临时写：

```nginx
server_name _;
```

### 2. 检查并重载 Nginx

```bash
sudo nginx -t
sudo systemctl reload nginx
```

### 3. 验证 API 代理

```bash
curl -i http://your-domain.com/api/auth/login \
  -X POST \
  -H 'Content-Type: application/json' \
  -d '{}'
```

## 十、部署 Flutter 前端

Flutter 前端继续按 Java 项目 README 的方式构建，只需要把 API 地址指向同域 `/api`。

本地 Flutter 项目执行示例：

```bash
# 使用域名
DOMAIN=your-domain.com ./build_web_prod.sh

# 如果只是用 HTTP 和公网 IP 测试
USE_HTTPS=false DOMAIN=your-ec2-public-ip ./build_web_prod.sh
```

上传构建产物：

```bash
rsync -avz -e "ssh -i your-key.pem" \
  build/web/ \
  ec2-user@your-ec2-public-ip:/tmp/stock-lab-web/

ssh -i your-key.pem ec2-user@your-ec2-public-ip
sudo mkdir -p /var/www/stock-lab
sudo rsync -av --delete /tmp/stock-lab-web/ /var/www/stock-lab/
sudo chown -R nginx:nginx /var/www/stock-lab
```

验证：

```bash
curl -I http://your-domain.com/
```

浏览器打开：

```text
http://your-domain.com
```

## 十一、配置 HTTPS

如果已有域名解析到 EC2，可使用 Let's Encrypt。

Amazon Linux 2023 可以通过 snap 安装 certbot：

```bash
sudo dnf install -y snapd
sudo systemctl enable --now snapd.socket
sudo ln -s /var/lib/snapd/snap /snap 2>/dev/null || true
sudo snap install core
sudo snap refresh core
sudo snap install --classic certbot
sudo ln -s /snap/bin/certbot /usr/bin/certbot 2>/dev/null || true
```

申请证书并自动修改 Nginx：

```bash
sudo certbot --nginx -d your-domain.com -d www.your-domain.com
```

验证自动续期：

```bash
sudo certbot renew --dry-run
```

## 十二、更新部署

### 1. 更新后端

```bash
cd /opt/stock-lab/backend/current
git pull

cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DDROGON_CTL_COMMAND=$HOME/vcpkg/installed/x64-linux/tools/drogon/drogon_ctl

cmake --build build -j1

sudo systemctl restart stock-lab-backend
sudo systemctl status stock-lab-backend --no-pager
```

### 2. 更新前端

本地重新构建 Flutter：

```bash
DOMAIN=your-domain.com ./build_web_prod.sh
```

重新上传 `build/web/` 覆盖 `/var/www/stock-lab`。

### 3. 回滚后端

如果使用 Git 部署，可回退到上一个提交：

```bash
cd /opt/stock-lab/backend/current
git log --oneline -5
git checkout <previous-commit>

cmake --build build -j1
sudo systemctl restart stock-lab-backend
```

如果用打包上传部署，建议每次发布前备份旧目录：

```bash
cd /opt/stock-lab/backend
cp -a current "backup-$(date +%Y%m%d-%H%M%S)"
```

回滚时把备份目录恢复为 `current` 后重启服务。

## 十三、常用排查命令

### 后端进程

```bash
sudo systemctl status stock-lab-backend --no-pager
sudo journalctl -u stock-lab-backend -n 200 --no-pager
sudo journalctl -u stock-lab-backend -f
```

### 端口监听

```bash
ss -lntp | grep 8088
```

### 本机访问后端

```bash
curl -i http://127.0.0.1:8088/api/auth/login \
  -X POST \
  -H 'Content-Type: application/json' \
  -d '{}'
```

### Nginx

```bash
sudo nginx -t
sudo systemctl status nginx --no-pager
sudo tail -f /var/log/nginx/access.log
sudo tail -f /var/log/nginx/error.log
```

### 前端静态文件

```bash
ls -la /var/www/stock-lab
```

目录下应包含：

```text
index.html
main.dart.js
assets/
```

## 十四、常见问题

### 1. 浏览器仍显示 Welcome to nginx

检查是否存在默认配置抢占：

```bash
ls -la /etc/nginx/conf.d/
sudo nginx -T | grep -n "server_name\\|root"
```

确认 `stock-lab.conf` 已加载，且 `root` 指向 `/var/www/stock-lab`。

### 2. `/api` 返回 502

说明 Nginx 没连上后端。检查：

```bash
sudo systemctl status stock-lab-backend --no-pager
ss -lntp | grep 8088
curl -i http://127.0.0.1:8088/api/auth/login -X POST -H 'Content-Type: application/json' -d '{}'
```

### 3. 后端启动后立即退出

常见原因：

```text
.env 缺少必填变量
config/config.prod.json 不存在或 JSON 格式错误
MySQL 无法连接
vcpkg 依赖或动态库路径异常
```

查看日志：

```bash
sudo journalctl -u stock-lab-backend -n 200 --no-pager
```

### 4. 远程编译依赖问题

#### `curl-minimal` 和 `curl` 冲突

Amazon Linux 2023 默认可能安装 `curl-minimal`。如果安装基础工具时报：

```text
curl-minimal ... conflicts with curl
```

不要安装 `curl`，使用系统已有的 `curl-minimal` 即可。

#### OpenSSL 编译失败，缺少 `IPC::Cmd`

如果 vcpkg 编译 OpenSSL 时报：

```text
Can't locate IPC/Cmd.pm
Perl cannot find IPC::Cmd
```

安装缺失依赖：

```bash
sudo dnf install -y kernel-headers perl-core perl-IPC-Cmd
perl -MIPC::Cmd -e 'print "IPC::Cmd OK\n"'
```

#### 找不到 `DrogonConfig.cmake`

如果 CMake 报：

```text
Could not find a package configuration file provided by "Drogon"
```

确认 Drogon 是否装到当前 vcpkg：

```bash
~/vcpkg/vcpkg list | grep -i drogon
find ~/vcpkg/installed -iname '*Drogon*Config*.cmake' -o -iname '*drogon*config*.cmake'
```

如果没有输出，重新安装：

```bash
VCPKG_DISABLE_METRICS=1 VCPKG_MAX_CONCURRENCY=1 ~/vcpkg/vcpkg install 'drogon[ctl,mysql]:x64-linux'
```

#### 找不到 `drogon_ctl`

如果 CMake 报：

```text
Could not find DROGON_CTL_COMMAND using the following names: drogon_ctl
```

查找工具路径：

```bash
find ~/vcpkg -type f -name drogon_ctl
```

如果存在 `/home/ec2-user/vcpkg/installed/x64-linux/tools/drogon/drogon_ctl`，CMake 配置时显式指定：

```bash
-DDROGON_CTL_COMMAND=$HOME/vcpkg/installed/x64-linux/tools/drogon/drogon_ctl
```

如果找不到，重新安装带 `ctl` feature 的 Drogon：

```bash
VCPKG_DISABLE_METRICS=1 VCPKG_MAX_CONCURRENCY=1 ~/vcpkg/vcpkg install 'drogon[ctl,mysql]:x64-linux'
```

#### 找不到 Boost Multiprecision

如果编译时报：

```text
fatal error: boost/multiprecision/cpp_dec_float.hpp: No such file or directory
```

安装缺失依赖：

```bash
VCPKG_DISABLE_METRICS=1 VCPKG_MAX_CONCURRENCY=1 ~/vcpkg/vcpkg install boost-multiprecision:x64-linux
```

#### `coroutine - not found`

下面这行不是失败原因：

```text
-- Looking for C++ include coroutine - not found
```

GCC 11 下项目会继续使用 C++17。真正错误要看 CMake 或 Ninja 输出的最后几十行。

### 5. 编译时报 `constexpr` 和 `std::string` 相关错误

如果服务器使用 GCC 11 + C++17，类似下面的错误表示源码里有 C++17 下不合法的 `constexpr std::string` 操作，或第三方 `jsoncpp` 不能直接接收 `std::string_view`：

```text
error: call to non-'constexpr' function 'bool std::operator==(...)'
error: no match for 'operator=' (operand types are 'Json::Value' and 'std::string_view')
error: 'optional' in namespace 'std' does not name a template type
error: 'condition_variable' in namespace 'std' does not name a type
```

后两类错误通常是头文件缺少自己直接使用的标准库 include，例如 `<optional>`、`<condition_variable>`、`<mutex>`、`<unordered_map>`。请确认部署的是包含该修复的最新代码，然后重新打包上传或 `git pull` 后再编译：

```bash
cd /opt/stock-lab/backend/current
rm -rf build
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DDROGON_CTL_COMMAND=$HOME/vcpkg/installed/x64-linux/tools/drogon/drogon_ctl
cmake --build build -j1
```

### 6. 邮件发送失败

检查：

```text
MAIL_API_KEY 是否正确
Resend 发件人是否已验证
templates/email/code.html 是否存在
systemd WorkingDirectory 是否为 /opt/stock-lab/backend/current/build
```

### 7. 股票查询失败

检查：

```text
FINNHUB_TOKEN 是否正确
服务器是否能访问 finnhub.io
Finnhub 免费额度是否用完
```

可在服务器测试外网：

```bash
curl -I https://finnhub.io
```

## 十五、上线检查清单

```text
[ ] MySQL 已备份
[ ] 生产库表已初始化
[ ] 小内存 EC2 已启用 swap
[ ] vcpkg 依赖已安装到 x64-linux
[ ] drogon_ctl 路径存在
[ ] .env 已配置 prod
[ ] config/config.prod.json 存在
[ ] Release 二进制已编译
[ ] systemd 服务已启动
[ ] 127.0.0.1:8088 本机 curl 有响应
[ ] Nginx /api/ 反代有响应
[ ] Flutter 静态文件已上传
[ ] 域名 DNS 已解析到 EC2
[ ] HTTPS 证书已配置
[ ] EC2 安全组未开放 8088 到公网
```
