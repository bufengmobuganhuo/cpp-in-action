#include <iostream>
#include <thread>
#include "include/Wrapper.h"

// 第 2 章开篇示例：启动一个线程打印问候语
void hello() {
    std::cout << "Hello Concurrent World\n";
}

int main() {
    std::thread t(hello);
    t.join();  // 等待线程结束，避免 main 提前退出导致程序终止
    Wrapper w1, w2;
    swap_1(w1, w2);
    return 0;
}
