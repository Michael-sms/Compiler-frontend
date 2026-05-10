# 测试的使用说明
## 关于文件结构的说明
`main.cpp` 只是一个测试文件，用来测试附属代码能否编译成功

`src`是包含源码的目录，在这个目录下包括了源码的文件，之后可以在此目录下添加头文件的定义文件

`include`是包含头文件的目录，在这个目录下包括了自定义的文件，之后可以在此目录下添加自定义的头文件

## 使用方法
`cmake` 

因为无法统一机器环境，目前采用cmake工具进行管理，需要将main.cpp置于与CmakeLists文件相同位置，分别将h文件和cpp文件放到include和src中(前端也可以进行如此设置)，创建build文件夹，在其内部进行（cmake ..）而后进行make 能够获取执行文件

`script` 脚本文件，可以在linux及mac使用，windows暂时不支持（.sh文件格式不支持），内部涉及mkdir文件夹创建，cmake指令执行，make指令执行，若windows环境可以执行以上命令，则可以进行单独输入进行模拟

## 关于文件的修改

由于文件间涉及多文件调用，所以单一的编译会出现问题（暂时不清楚如何解决，没有试过。。）,cmake版本已经测通，修改类内部分函数的声明与定义情况

## 词法分析器 (Lexer) 编译与测试指南

本项目使用 CMake 进行构建，支持将词法分析器作为独立的模块进行单元测试。

### 1. 编译步骤
请确保你的电脑上已安装 CMake 和 C++ 编译环境（如 MSVC 或 GCC）。
在项目根目录（`compiler_ir`）下打开终端，依次执行以下命令：

```bash
# 创建并进入构建目录（Out-of-source Build）
mkdir build
cd build

# 生成构建文件
cmake ..

# 编译项目（包含主程序和词法测试程序）
cmake --build .

# Windows 环境：
.\Debug\test_lexer.exe

# Linux/Mac 环境：
./test_lexer