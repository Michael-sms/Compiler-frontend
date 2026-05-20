# 测试目录说明

测试用例与 `run.md` 中 **project1** 用法一致。

## 目录

| 目录 | 内容 |
|------|------|
| `lexer_cases/` | 词法 `.sy` + `expected/*.tokens.golden` |
| `parser_b_cases/` | 语法 `.sy` + `expected/*.{trace,reductions,errors}.golden` |
| `ir_cases/` | IR `.sy` + `expected/*.ll.golden` |
| `scripts/` | `compare_outputs.py`、`run_all_tests.sh` |
| `output/` | 运行后实际输出（截图用，已 gitignore） |

## 运行

```bash
cd compiler_ir
cmake -S . -B build && cmake --build build
./tests/scripts/run_all_tests.sh
```

或：

```bash
python3 tests/scripts/compare_outputs.py          # 对比
python3 tests/scripts/compare_outputs.py --update # 更新 golden
```

## 手工单例（run.md）

```bash
./build/project1 tests/lexer_cases/01_var_decl.sy
./build/project1 parserB tests/parser_b_cases/case1.sy
./build/project1 ir tests/ir_cases/ir03_main_return.sy tests/output/demo.ll
```
