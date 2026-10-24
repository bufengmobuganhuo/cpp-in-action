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
DB_USERNAME
DB_PASSWORD
FINNHUB_TOKEN
MAIL_API_KEY
```

`config/config.prod.json` 使用 `DB_USERNAME` 和 `DB_PASSWORD`，`.env` 中的变量名需与之保持一致。

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

推荐使用 vcpkg 管理 jwt-cpp、Boost、croncpp、JsonCpp、MariaDB Connector/C、OpenSSL 等基础依赖。

重要：Drogon 和它的 Trantor 子模块一起从源码安装到 `$HOME/drogon-install`。编译后端时必须同时使用这套安装中的 `libdrogon.a` 和 `libtrantor.a`；不要让 vcpkg 的 Trantor 混入链接。当前仓库的 `models/` 代码还依赖较新的 `drogon_ctl` 生成结果。

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

如果服务器内存只有 1GB 左右，编译 Drogon、OpenSSL 等依赖时可能触发 OOM，表现为编译中断、SSH 卡死或实例短时间连不上。建议先增加 4GB swap：

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
tmux new -s stock-lab-build
```

如果 SSH 断开，重新登录后恢复会话：

```bash
tmux attach -t stock-lab-build
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
  jwt-cpp:x64-linux \
  boost-headers:x64-linux \
  boost-multiprecision:x64-linux \
  croncpp:x64-linux \
  jsoncpp:x64-linux \
  libmariadb:x64-linux \
  openssl:x64-linux \
  zlib:x64-linux \
  brotli:x64-linux \
  c-ares:x64-linux \
  libuuid:x64-linux
```

安装完成后确认关键依赖存在：

```bash
~/vcpkg/vcpkg list | grep -E 'jwt-cpp|boost-multiprecision|croncpp|jsoncpp|libmariadb|openssl'
find ~/vcpkg/installed/x64-linux -path '*jsoncpp*' | sort
```

### 4. 源码安装 Drogon

在服务器上从源码安装 Drogon，并把安装结果固定到 `$HOME/drogon-install`：

```bash
cd ~
git clone https://github.com/drogonframework/drogon.git
cd ~/drogon
git submodule update --init --recursive

# vcpkg 的 jsoncpp 库名是 libjsoncpp.a；Drogon 源码构建可能链接 -ljsoncpp_lib。
# 创建兼容软链接，避免链接 drogon_ctl 时报 cannot find -ljsoncpp_lib。
ln -sf $HOME/vcpkg/installed/x64-linux/lib/libjsoncpp.a \
       $HOME/vcpkg/installed/x64-linux/lib/libjsoncpp_lib.a

cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DCMAKE_INSTALL_PREFIX=$HOME/drogon-install \
  -DCMAKE_INSTALL_LIBDIR=lib64 \
  -DINSTALL_LIB_DIR=lib64 \
  -DINSTALL_DROGON_CMAKE_DIR=lib64/cmake/Drogon \
  -DINSTALL_TRANTOR_CMAKE_DIR=lib64/cmake/Trantor \
  -DUSE_SUBMODULE=ON \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_ORM=ON \
  -DBUILD_MYSQL=ON \
  -DBUILD_CTL=ON \
  -DCMAKE_PREFIX_PATH="$HOME/vcpkg/installed/x64-linux" \
  -Djsoncpp_DIR=$HOME/vcpkg/installed/x64-linux/share/jsoncpp \
  -DCMAKE_CXX_FLAGS="-I$HOME/vcpkg/installed/x64-linux/include" \
  -DCMAKE_EXE_LINKER_FLAGS="-L$HOME/vcpkg/installed/x64-linux/lib"

grep '^#define USE_MYSQL 1' build/drogon/config.h
```

`grep` 应输出 `#define USE_MYSQL 1`。如果没有输出，先检查 MariaDB Connector/C 是否被 CMake 找到，再继续构建和安装。

```bash
cmake --build build -j1
cmake --install build
```

编译期间类似下面的 deprecated warning 可以忽略：

```text
CT_APPLICATION_X_JAVASCRIPT is deprecated: use CT_TEXT_JAVASCRIPT
```

安装完成后确认 Drogon、同源 Trantor 和 `drogon_ctl` 均已安装到同一前缀：

```bash
ls -l $HOME/drogon-install/lib64/cmake/Drogon/DrogonConfig.cmake \
      $HOME/drogon-install/lib64/cmake/Trantor/TrantorConfig.cmake \
      $HOME/drogon-install/lib64/libdrogon.a \
      $HOME/drogon-install/lib64/libtrantor.a \
      $HOME/drogon-install/bin/drogon_ctl
grep -R "utf8Length" -n $HOME/drogon-install/include/drogon/utils/Utilities.h
$HOME/drogon-install/bin/drogon_ctl version
```

若 `ls` 报文件不存在，用 `find $HOME/drogon-install -name DrogonConfig.cmake -o -name TrantorConfig.cmake -o -name libdrogon.a -o -name libtrantor.a` 核对实际位置。若已有安装使用 `lib` 而不是 `lib64`，后续 `Drogon_DIR` 和 `Trantor_DIR` 应一起改为对应的 `lib/cmake/...` 路径。

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

在服务器项目根目录执行。显式指定源码安装的 Drogon 和 Trantor；vcpkg 只提供其他依赖。已有 `build` 目录时，下面的 `-D` 参数会更新缓存中的路径，无需删除整个构建目录。

```bash
cd /opt/stock-lab/backend/current

# 确认同一安装前缀中同时有 Drogon 和 Trantor
ls -l $HOME/drogon-install/lib64/cmake/Drogon/DrogonConfig.cmake \
      $HOME/drogon-install/lib64/cmake/Trantor/TrantorConfig.cmake \
      $HOME/drogon-install/lib64/libdrogon.a \
      $HOME/drogon-install/lib64/libtrantor.a \
      $HOME/drogon-install/bin/drogon_ctl
```

这五个文件都应存在。若有文件不存在，先检查安装步骤；若已有安装使用 `lib` 而不是 `lib64`，请按实际路径同时调整下方的 `Drogon_DIR` 和 `Trantor_DIR`。

```bash
# 如果 vcpkg 里仍安装过旧版 Drogon，源码版 include 需要排在前面。
# CMAKE_CXX_FLAGS 里的 -I$HOME/drogon-install/include 用于避免误用 vcpkg 旧头文件。
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DDrogon_DIR:PATH=$HOME/drogon-install/lib64/cmake/Drogon \
  -DTrantor_DIR:PATH=$HOME/drogon-install/lib64/cmake/Trantor \
  -DDROGON_CTL_COMMAND=$HOME/drogon-install/bin/drogon_ctl \
  -DCMAKE_PREFIX_PATH="$HOME/drogon-install;$HOME/vcpkg/installed/x64-linux" \
  -DCMAKE_CXX_FLAGS="-I$HOME/drogon-install/include -I$HOME/vcpkg/installed/x64-linux/include" \
  -DCMAKE_EXE_LINKER_FLAGS="-L$HOME/vcpkg/installed/x64-linux/lib"

cmake --build build --target stock-lab-backend -j1
```

如果这套安装实际使用 `lib` 目录，把上面两个参数一起改成：

```bash
-DDrogon_DIR:PATH=$HOME/drogon-install/lib/cmake/Drogon
-DTrantor_DIR:PATH=$HOME/drogon-install/lib/cmake/Trantor
```

编译完成后，在**项目根目录**检查链接命令和二进制：

```bash
ninja -C build -t commands stock-lab-backend | tail -n 1 | tr ' ' '\n' | grep -E '/lib(drogon|trantor)\.a'
grep -E '^(Drogon_DIR|Trantor_DIR):' build/CMakeCache.txt
ls -lh build/stock-lab-backend
```

预期两个 `.a` 文件都位于 `$HOME/drogon-install/lib64/`（或同一安装前缀的 `lib/`）。如果已进入 `build` 目录，Ninja 命令应改为 `ninja -t commands stock-lab-backend`，不要再加 `-C build`。只要 `libtrantor.a` 仍来自 vcpkg，就不要继续启动验证，先按“十四、常见问题”中的混用库排查步骤重新配置。

## 七、手动启动验证

首次部署先手动启动，再按从简单到完整的顺序验证 HTTP、数据库和邮件链路。

重要：当前邮件模板路径为 `../templates/email/...`，请在 `build` 目录启动后端。

```bash
cd /opt/stock-lab/backend/current/build
./stock-lab-backend
```

另开一个 SSH 窗口验证：

```bash
# 先验证 HTTP 监听和路由；404 响应也是正常结果
curl -i http://127.0.0.1:8088/__probe__

# 参数校验，不会查询数据库或发送邮件
curl -i http://127.0.0.1:8088/api/auth/verifyCode \
  -X POST \
  -H 'Content-Type: application/json' \
  -d '{"email":"18681820448@163.com"}'
```
A
上述请求均能返回 HTTP 响应且进程保持运行，说明基础 HTTP 链路可用。要验证白名单查询和发送邮件，再把下面的地址换成**已加入白名单且由你控制**的邮箱：

```bash
curl -i http://127.0.0.1:8088/api/auth/verifyCode \
  -X POST \
  -H 'Content-Type: application/json' \
  -d '{"email":"your-invited-email@example.com"}'
```

只有收到预期成功响应并确认邮件到达后，才能认为数据库与邮件链路也通过验证；股票查询、交易和定时任务仍需分别验证。

如果启动失败，优先检查：

```bash
cd /opt/stock-lab/backend/current
grep -E '^(PROFILES_ACTIVE|STOCK_LAB_ENV|DB_HOST|DB_NAME|DB_USERNAME|DB_PASSWORD|FINNHUB_TOKEN|MAIL_API_KEY)=' .env \
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
curl -i http://your-domain.com/api/auth/verifyCode \
  -X POST \
  -H 'Content-Type: application/json' \
  -d '{}'
```

这里先验证 Nginx 到后端的代理链路。需要验证数据库和邮件时，参照“七、手动启动验证”使用已加入白名单的邮箱。

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
  -DDrogon_DIR:PATH=$HOME/drogon-install/lib64/cmake/Drogon \
  -DTrantor_DIR:PATH=$HOME/drogon-install/lib64/cmake/Trantor \
  -DDROGON_CTL_COMMAND=$HOME/drogon-install/bin/drogon_ctl \
  -DCMAKE_PREFIX_PATH="$HOME/drogon-install;$HOME/vcpkg/installed/x64-linux" \
  -DCMAKE_CXX_FLAGS="-I$HOME/drogon-install/include -I$HOME/vcpkg/installed/x64-linux/include" \
  -DCMAKE_EXE_LINKER_FLAGS="-L$HOME/vcpkg/installed/x64-linux/lib"

cmake --build build --target stock-lab-backend -j1
ninja -C build -t commands stock-lab-backend | tail -n 1 | tr ' ' '\n' | grep -E '/lib(drogon|trantor)\.a'
```

确认上面的链接检查输出中两个库均位于 `$HOME/drogon-install/lib64/`，再重启服务。若安装目录使用 `lib`，同样把 `Drogon_DIR`、`Trantor_DIR` 一起改为 `lib/cmake/...`。

```bash
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

cmake --build build --target stock-lab-backend -j1
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

如果服务在任意 HTTP 请求到来时崩溃，并出现 `Bad file descriptor`、`It is forbidden to run loop on threads other than event-loop thread`，先检查 Drogon 和 Trantor 是否链接自同一套源码安装。在项目根目录运行：

```bash
cd /opt/stock-lab/backend/current
grep -E '^(Drogon_DIR|Trantor_DIR):' build/CMakeCache.txt
ninja -C build -t commands stock-lab-backend | tail -n 1 | tr ' ' '\n' | grep -E '/lib(drogon|trantor)\.a'
```

若 `libdrogon.a` 位于 `$HOME/drogon-install/lib64/`，而 `libtrantor.a` 位于 `$HOME/vcpkg/installed/x64-linux/lib/`，按“六、编译后端”重新配置并编译，使两个库都指向 `$HOME/drogon-install/lib64/`。若路径一致但仍崩溃，再用 `gdb` 获取堆栈继续排查。

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

确认源码版 Drogon 和它配套的 Trantor 是否都已安装：

```bash
find $HOME/drogon-install -name DrogonConfig.cmake
find $HOME/drogon-install -name TrantorConfig.cmake
```

如果任意一个没有输出，回到“三、安装 C++ 依赖 / 源码安装 Drogon”检查 `USE_SUBMODULE=ON`、安装前缀和安装步骤。

如果输出路径是：

```bash
/home/ec2-user/drogon-install/lib64/cmake/Drogon/DrogonConfig.cmake
```

后端 CMake 参数应同时使用：

```bash
-DDrogon_DIR:PATH=$HOME/drogon-install/lib64/cmake/Drogon
-DTrantor_DIR:PATH=$HOME/drogon-install/lib64/cmake/Trantor
```

如果输出路径是：

```bash
/home/ec2-user/drogon-install/lib/cmake/Drogon/DrogonConfig.cmake
```

后端 CMake 参数应同时使用：

```bash
-DDrogon_DIR:PATH=$HOME/drogon-install/lib/cmake/Drogon
-DTrantor_DIR:PATH=$HOME/drogon-install/lib/cmake/Trantor
```

#### 找不到 `drogon_ctl`

如果 CMake 报：

```text
Could not find DROGON_CTL_COMMAND using the following names: drogon_ctl
```

查找工具路径：

```bash
find $HOME/drogon-install -name drogon_ctl
```

如果存在 `/home/ec2-user/drogon-install/bin/drogon_ctl`，CMake 配置时显式指定：

```bash
-DDROGON_CTL_COMMAND=$HOME/drogon-install/bin/drogon_ctl
```

如果找不到，重新执行源码安装 Drogon，并确认源码编译时包含：

```bash
-DBUILD_CTL=ON
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
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DDrogon_DIR:PATH=$HOME/drogon-install/lib64/cmake/Drogon \
  -DTrantor_DIR:PATH=$HOME/drogon-install/lib64/cmake/Trantor \
  -DDROGON_CTL_COMMAND=$HOME/drogon-install/bin/drogon_ctl \
  -DCMAKE_PREFIX_PATH="$HOME/drogon-install;$HOME/vcpkg/installed/x64-linux" \
  -DCMAKE_CXX_FLAGS="-I$HOME/drogon-install/include -I$HOME/vcpkg/installed/x64-linux/include" \
  -DCMAKE_EXE_LINKER_FLAGS="-L$HOME/vcpkg/installed/x64-linux/lib"
cmake --build build -j1
```

### 6. 编译时报 `utf8Length` 不是 `drogon::utils` 成员

如果报：

```text
error: 'utf8Length' is not a member of 'drogon::utils'
```

说明当前编译实际使用了旧版 Drogon 头文件。常见原因是 vcpkg 的旧 Drogon include 路径排在源码安装的 Drogon 前面。

先确认源码版 Drogon 头文件包含该函数：

```bash
grep -R "utf8Length" -n $HOME/drogon-install/include/drogon/utils/Utilities.h
grep -R "utf8Length" -n $HOME/vcpkg/installed/x64-linux/include/drogon/utils/Utilities.h
```

如果第一条有输出、第二条没有输出，重新配置后端时必须包含：

```bash
-DDrogon_DIR:PATH=$HOME/drogon-install/lib64/cmake/Drogon
-DTrantor_DIR:PATH=$HOME/drogon-install/lib64/cmake/Trantor
-DDROGON_CTL_COMMAND=$HOME/drogon-install/bin/drogon_ctl
-DCMAKE_CXX_FLAGS="-I$HOME/drogon-install/include -I$HOME/vcpkg/installed/x64-linux/include"
```

如果仍然误用 vcpkg 旧头文件，可以移除 vcpkg Drogon，保留其它 vcpkg 依赖：

```bash
cd $HOME/vcpkg
./vcpkg remove drogon:x64-linux
```

然后回项目目录重新执行“六、编译后端”。

### 7. 源码编译 Drogon 时 JsonCpp 相关错误

如果源码编译 Drogon 时报：

```text
fatal error: json/json.h: No such file or directory
```

先确认 vcpkg 里有头文件：

```bash
ls -l $HOME/vcpkg/installed/x64-linux/include/json/json.h
```

重新配置 Drogon 时加入：

```bash
-Djsoncpp_DIR=$HOME/vcpkg/installed/x64-linux/share/jsoncpp
-DCMAKE_CXX_FLAGS="-I$HOME/vcpkg/installed/x64-linux/include"
```

如果链接 `drogon_ctl` 时报：

```text
/usr/bin/ld: cannot find -ljsoncpp_lib
```

创建兼容软链接，并重新配置/编译 Drogon：

```bash
ln -sf $HOME/vcpkg/installed/x64-linux/lib/libjsoncpp.a \
       $HOME/vcpkg/installed/x64-linux/lib/libjsoncpp_lib.a
```

源码编译 Drogon 的完整命令见“三、安装 C++ 依赖 / 源码安装 Drogon”。

### 8. 启动时报 MySQL TLS/SSL 错误

如果启动后端时报：

```text
TLS/SSL error: SSL is required, but the server does not support it
Failed to mysql_real_connect()
```

先不要直接用 shell 里的 `$DB_HOST` 测试，因为项目会由 `ConfigLoader` 自动读取 `.env`，shell 不会自动加载这些变量。先在服务器项目根目录加载 `.env`：

```bash
cd /opt/stock-lab/backend/current

grep -nE '^(DB_HOST|DB_NAME|DB_USERNAME|DB_PASSWORD|PROFILES_ACTIVE|STOCK_LAB_ENV)=' .env \
  | sed -E 's/(DB_PASSWORD)=.*/\1=<hidden>/'

set -a
source .env
set +a
```

再测试真实数据库连接：

```bash
mysql --ssl=0 \
  -h "$DB_HOST" \
  -P 3306 \
  -u "$DB_USERNAME" \
  -p"$DB_PASSWORD" \
  "$DB_NAME" \
  -e "select 1;"
```

结果判断：

```text
mysql --ssl=0 能连上：数据库允许非 SSL，问题多半在 Drogon/MariaDB client SSL 默认行为。
仍报 SSL 错误：检查 MySQL 服务端或账号 SSL 策略。
报 Access denied：检查 .env 中用户名、密码、库名和 DB_HOST 是否正确。
```

### 9. 邮件发送失败

检查：

```text
MAIL_API_KEY 是否正确
Resend 发件人是否已验证
templates/email/code.html 是否存在
systemd WorkingDirectory 是否为 /opt/stock-lab/backend/current/build
```

### 10. 股票查询失败

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
