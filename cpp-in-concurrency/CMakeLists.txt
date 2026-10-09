cmake_minimum_required(VERSION 4.3)
project(cpp_in_concurrency)

set(CMAKE_CXX_STANDARD 17)

add_executable(cpp_in_concurrency main.cpp
        chapter02/include/ThreadSafeStack.h
        chapter02/ThreadSafeStack.cpp
        chapter02/include/Wrapper.h
        chapter02/HierarchicalMutex.cpp
        chapter02/include/HierarchicalMutex.h)

# 每章一个可执行文件：自动收集 ${name}/ 下的所有 .cpp 和 ${name}/include/ 下的 .h
# 新章节只需创建目录并添加一行 add_chapter(chapterXX)
function(add_chapter name)
    file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${name}/*.cpp")
    file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${name}/include/*.h")
    add_executable(${name} ${sources} ${headers})
    target_include_directories(${name} PRIVATE ${name}/include)
endfunction()

add_chapter(chapter02)