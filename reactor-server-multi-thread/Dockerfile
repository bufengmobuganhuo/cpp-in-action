FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y \
gcc-11 g++-11 make cmake gdb openssh-server net-tools iproute2

# 默认gcc11
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 11 \
&& update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 11

# ssh root登录，密码123456
RUN mkdir -p /var/run/sshd
RUN echo 'root:123456' | chpasswd
RUN sed -i 's/PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
