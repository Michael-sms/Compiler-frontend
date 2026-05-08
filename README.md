# C-- 编译器前端（Compiler Frontend）

本项目为天津大学编译原理大作业的编译器前端实现，目标是完成 C-- 语言到 LLVM IR（`.ll`）的编译前端，包括词法分析、SLR 语法分析、语义/中间代码生成衔接。

## 项目简介
- **源语言**：C--（C 语言子集，`.sy` 单文件）。
- **目标语言**：LLVM IR（`.ll`）。
- **限制**：不得使用 Lex/Yacc/ANTLR；必须手写 DFA 确定化与最小化。

## 当前进度
- 已完成计划书 `plan1.md`。
- 已将老师提供的中端代码克隆到本地 `compiler_ir` 目录，后续开发以该目录为主。

## 项目结构（当前实际）
> 说明：仓库根目录用于文档管理，代码开发在 `compiler_ir` 目录内完成。

```
compiler-frontend/
  README.md
  plan1.md
  compiler_ir/
    CMakeLists.txt
    main.cpp
    Readme.md
    include/        # 中端 IR 接口头文件
    src/            # 中端 IR 实现
    script/         # 辅助脚本
    中端逻辑结构图.png
```

## 分工（5 位成员）
- **成员1（组长）**：整体架构与中端衔接、公共接口定义、合并审计。
- **成员2**：词法分析器、DFA 确定化与最小化、符号表与词法输出。
- **成员3**：LR(0) 项目集、FIRST/FOLLOW、SLR 预测分析表。
- **成员4**：SLR 驱动与 AST 构建、错误处理与规约序列输出。
- **成员5**：测试用例、输出对比、开发/测试报告与 PPT。

## 技术栈
- **语言**：C++（推荐，便于对接中端）。
- **编译理论**：DFA、SLR。
- **后端接口**：LLVM IR（`.ll`）。
- **辅助**：必要时可使用 Python 3.10+ 进行测试脚本/数据整理。

## 依赖安装（建议）
- **C++17+ 编译器**：MSVC / g++ / clang。
- **LLVM 工具链**：用于生成与查看 `.ll`（版本按课程要求为准）。
- **Python 3.10+（可选）**：用于测试脚本与报告数据处理。

> 若课程提供了固定环境或中端工具，请以课程说明为准。

## 快速开始（项目启动阶段）
1. 准备 C++ 编译环境与 LLVM 工具链。
2. 进入 `compiler_ir` 目录作为项目主目录。
3. 阅读 `plan1.md` 了解分工与接口统一约定。
4. 按模块落地代码结构后，再补充具体编译与运行命令。

## 统一形式（输出与接口约定摘要）
- **词法输出**：`[单词符号] [TAB] <[种别],[内容]>`。
- **语法输出**：`[序号] [TAB] [栈顶符号]#[面临输入符号] [TAB] [执行动作]`。
- **动作枚举**：`move` / `reduction` / `accept` / `error`。
- **模块接口**：Lexer → Parser 输出 `vector<Token>`；Parser → IR 输出 AST 根结点与符号表。

## 参考
- 课程附件与文法说明（见 `plan1.md`）。
- LLVM IR 语法：<https://llvm.org/docs/LangRef.html>
- 中端代码仓库：<https://gitee.com/happy-traveller/compiler_ir>
