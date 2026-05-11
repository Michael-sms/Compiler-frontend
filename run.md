# 运行说明（run.md）

本说明基于当前仓库结构与先前给出的编译方法，适用于 Windows PowerShell 环境。

## 一、进入项目目录
```powershell
cd "c:\Users\Lenovo\Desktop\2526source\编译原理\大作业\compiler-frontend\compiler_ir"
```

## 二、配置与编译
```powershell
cmake -S . -B build
cmake --build build --config Debug
```

## 三、运行词法分析器测试
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\test_lexer.exe
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\test_lexer.exe
```

## 四、运行 main.cpp（默认样例 or 指定文件）
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\project1.exe
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\project1.exe
```

### 指定输入文件（.sy）
将 `.sy` 文件路径作为参数传入：

- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\project1.exe "path\to\test.sy"
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\project1.exe "path\to\test.sy"
```

## 五、词法测试输入说明
当前测试入口位于 `compiler_ir\tests\lexer\test_main.cpp`：
- 修改其中的 `sourceCode` 字符串即可测试不同的 C-- 代码片段。
- 输出包含词法单词序列与符号表。

## 六、常见问题
- 若找不到可执行文件，请确认构建命令成功完成，并检查 `build` 目录下对应的输出路径。
- 若你使用的是 Release 配置，请将运行路径中的 `Debug` 替换为 `Release`。
