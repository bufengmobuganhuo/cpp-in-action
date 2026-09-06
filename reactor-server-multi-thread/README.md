docker build -t ubuntu2204-ssh-gcc11 .

# 2222映射ssh；8888是你的epoll服务监听端口
docker run -d \
--name epoll-dev \
-p 2222:22 \
-p 8888:8888 \
ubuntu2204-ssh-gcc11
