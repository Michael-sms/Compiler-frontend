#!/usr/bin/env bash
# 按 run.md 编译并运行全部自动化测试
set -euo pipefail
cd "$(dirname "$0")/../.."

echo "[1/3] CMake 编译 (同 run.md)..."
cmake -S . -B build
cmake --build build -j"$(nproc 2>/dev/null || echo 4)"

echo "[2/3] 对比 golden..."
python3 tests/scripts/compare_outputs.py

echo "[3/3] 保存 tests/output 供截图..."
python3 tests/scripts/compare_outputs.py --save-only

echo "完成。"
