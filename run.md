# 运行说明（run.md）

本说明基于当前仓库结构与先前给出的编译方法，适用于 Windows PowerShell 环境。

## 一、进入项目目录
```powershell
cd "c:\Users\Lenovo\Desktop\2526source\编译原理\大作业\compiler-frontend\compiler_ir"
```

## 二、配置与编译
### 1) MSVC (Visual Studio / Build Tools)
```powershell
cmake -S . -B build
cmake --build build --config Debug
```

### 2) MinGW-w64
首次切换到 MinGW 时建议使用新的构建目录：
```powershell
cmake -S . -B build-mingw -G "MinGW Makefiles"
cmake --build build-mingw
```

## 三、运行测试入口（合并后）
测试入口已合并到 `tests\test_main.cpp`，Parser A/B 代码已整合到 `slr_parser.h/.cpp`，通过参数选择模式：

### 1) 词法测试
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\test_lexer.exe lexer
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\test_lexer.exe lexer
```

### 2) Parser A 表构建
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\test_lexer.exe parserA
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\test_lexer.exe parserA
```

### 3) Parser B 驱动与 AST
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\test_lexer.exe parserB "path\to\input.sy"
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\test_lexer.exe parserB "path\to\input.sy"
```

### 4) 指定 Parser A 输出目录（可选）
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\test_lexer.exe parserA ".\parser_a_output"
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\test_lexer.exe parserA ".\parser_a_output"
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

### Parser A 表构建（main.cpp）
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\project1.exe parserA
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\project1.exe parserA
```

### 指定 Parser A 输出目录（可选）
- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\project1.exe parserA ".\parser_a_output"
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\project1.exe parserA ".\parser_a_output"
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

### 生成 LLVM IR（.ll）
使用 `ir` 模式生成 `.ll`，可指定输出文件名：

- **MSVC（常见于 Windows）**
```powershell
.\build\Debug\project1.exe ir "path\to\test.sy" ".\output.ll"
```

- **非 MSVC（如 MinGW/Clang）**
```powershell
.\build\project1.exe ir "path\to\test.sy" ".\output.ll"
```

## 五、词法测试输入说明
当前测试入口位于 `compiler_ir\tests\test_main.cpp`：
- 修改其中的 `sourceCode` 字符串即可测试不同的 C-- 代码片段。
- 输出包含词法单词序列与符号表。

## 六、常见问题
- 若找不到可执行文件，请确认构建命令成功完成，并检查 `build` 目录下对应的输出路径。
- 若你使用的是 Release 配置，请将运行路径中的 `Debug` 替换为 `Release`。
- 使用 MinGW 时无 `Debug` 子目录，直接使用 `build-mingw` 下的可执行文件。
