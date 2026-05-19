cmake_minimum_required(VERSION 3.16)

# 设置c++17标准
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_NAME MyProject)
project(${PROJECT_NAME})

# ================= 关键修复区域 =================
# 自动判断当前使用的编译器，赋予对应的编译参数
if(MSVC)
    # 微软 MSVC 编译器的参数（/W3 是警告等级，/Zi 是生成调试信息）
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /Zi")
else()
    # GCC/Clang 编译器的参数（你最初的设定）
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-attributes -Wno-unused-variable")
endif()
# ================================================

# 1. 包含头文件目录
include_directories("include")

# 2. 收集 src 下所有的 .cpp 文件

file(GLOB_RECURSE DIR_SRC CONFIGURE_DEPENDS "src/*.cpp")

# 3. 将所有核心代码打包成一个名为 project1_lib 的库
add_library(project1_lib ${DIR_SRC})

# 4. 组长的主程序入口
add_executable(project1 main.cpp)
target_link_libraries(project1 project1_lib)

# 5. 测试入口（词法 & Parser A）
add_executable(test_lexer tests/test_main.cpp)
target_link_libraries(test_lexer project1_lib)