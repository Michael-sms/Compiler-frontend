# Parser B 测试说明

自动化 golden 位于 `expected/*.golden`，由 `tests/scripts/compare_outputs.py` 维护。

| 用例 | 说明 | 期望 |
|------|------|------|
| case1.sy | `int a;` | accept |
| case2.sy | `int main() { return 0; }` | accept |
| case3.sy | `int ;` | syntax error |
| case4_var_init.sy | 变量初始化 | accept |
| case5_const.sy | const 声明 | accept |
| case6_main_block.sy | main + 块内声明 | accept |
| case7_assign.sy | 赋值 | accept |
| case8_expr.sy | 算术表达式 | accept |
| case9_if_else.sy | if-else | accept |
| case10_logic.sy | 关系/逻辑运算 | accept |

文法对 `main` 使用 `FuncName`/`funcType` 等产生式，与课程关键字 `main` 对齐。
