#!/usr/bin/env python3
"""
C-- 编译器前端自动化测试（对齐 run.md 中 project1 用法）。

  ./build/project1 <file.sy>              # 词法
  ./build/project1 parserB <file.sy>      # 语法
  ./build/project1 ir <file.sy> <out.ll>  # IR

用法:
  python3 tests/scripts/compare_outputs.py
  python3 tests/scripts/compare_outputs.py --update
  python3 tests/scripts/compare_outputs.py --save-only
"""

from __future__ import annotations

import argparse
import difflib
import re
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD_DIRS = [ROOT / "build", ROOT / "build-mingw"]
LEXER_CASES = ROOT / "tests" / "lexer_cases"
PARSER_CASES = ROOT / "tests" / "parser_b_cases"
IR_CASES = ROOT / "tests" / "ir_cases"
OUTPUT_DIR = ROOT / "tests" / "output"


@dataclass
class TestResult:
    name: str
    passed: bool
    message: str


def find_executable() -> Path:
    for base in BUILD_DIRS:
        for name in ("project1", "project1.exe"):
            for sub in ("", "Debug", "Release"):
                p = base / sub / name if sub else base / name
                if p.is_file():
                    return p
    raise FileNotFoundError(
        "未找到 project1，请在 compiler_ir 目录执行:\n"
        "  cmake -S . -B build && cmake --build build"
    )


def run_cmd(exe: Path, *args: str) -> str:
    proc = subprocess.run(
        [str(exe), *args],
        cwd=ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    return proc.stdout + proc.stderr


def strip_noise(text: str) -> str:
    lines = []
    for line in text.splitlines():
        if line.startswith("SLRParserB build"):
            continue
        lines.append(line.rstrip())
    return "\n".join(lines) + ("\n" if lines else "")


def extract_lexer_tokens(full_output: str) -> str:
    lines: list[str] = []
    in_lexer = False
    for line in full_output.splitlines():
        if line.strip() == "--- Lexer Output ---":
            in_lexer = True
            continue
        if in_lexer and line.startswith("==="):
            break
        if in_lexer and line.strip():
            lines.append(line.rstrip())
    return "\n".join(lines) + ("\n" if lines else "")


def extract_parser_sections(full_output: str) -> dict[str, str]:
    text = strip_noise(full_output)
    sections = {"trace": "", "reductions": "", "errors": ""}
    current = None
    mapping = {
        "--- Parser B Trace ---": "trace",
        "--- Reduction Sequence ---": "reductions",
        "--- Errors ---": "errors",
    }
    buf: list[str] = []
    for line in text.splitlines():
        if line in mapping:
            if current:
                sections[current] = "\n".join(buf).strip() + ("\n" if buf else "")
            current = mapping[line]
            buf = []
            continue
        if current:
            buf.append(line)
    if current:
        sections[current] = "\n".join(buf).strip() + ("\n" if buf else "")

    red = sections["reductions"]
    m = re.search(r"Reduction sequence \(production ids\):\s*(.*)", red, re.S)
    if m:
        body = m.group(1).strip()
        sections["reductions"] = ("<empty>\n" if body == "<empty>" else body.replace("\n", " ").strip() + "\n")

    err = sections["errors"]
    if "Errors: <none>" in err:
        sections["errors"] = "<none>\n"
    else:
        err_lines = [ln.strip() for ln in err.splitlines() if ln.strip().startswith("- ")]
        sections["errors"] = ("\n".join(err_lines) + "\n") if err_lines else "<none>\n"
    return sections


def normalize_ir(text: str) -> str:
    lines = []
    for line in text.splitlines():
        if line.startswith("source_filename = "):
            m = re.search(r'source_filename = "(.*)"', line)
            if m:
                p = m.group(1).replace("\\", "/")
                if p.startswith("/"):
                    # 绝对路径转为 tests/... 相对形式以便跨机器对比
                    idx = p.find("tests/")
                    if idx >= 0:
                        p = p[idx:]
                line = f'source_filename = "{p}"'
        lines.append(line)
    return "\n".join(lines) + ("\n" if lines else "")


def compare_text(name: str, actual: str, expected_path: Path, update: bool) -> TestResult:
    if update:
        expected_path.parent.mkdir(parents=True, exist_ok=True)
        expected_path.write_text(actual, encoding="utf-8")
        return TestResult(name, True, f"已更新 {expected_path.relative_to(ROOT)}")

    if not expected_path.is_file():
        return TestResult(name, False, f"缺少 golden: {expected_path.relative_to(ROOT)}")

    expected = expected_path.read_text(encoding="utf-8")
    if actual == expected:
        return TestResult(name, True, "通过")

    diff = "\n".join(
        difflib.unified_diff(
            expected.splitlines(),
            actual.splitlines(),
            fromfile="expected",
            tofile="actual",
            lineterm="",
        )
    )
    return TestResult(name, False, f"不一致:\n{diff}")


def run_lexer_tests(exe: Path, update: bool, save_only: bool) -> list[TestResult]:
    results: list[TestResult] = []
    golden = LEXER_CASES / "expected"
    for sy in sorted(LEXER_CASES.glob("*.sy")):
        rel = sy.relative_to(ROOT).as_posix()
        out = run_cmd(exe, rel)
        tokens = extract_lexer_tokens(out)

        sub = OUTPUT_DIR / "lexer" / sy.stem
        if save_only or not update:
            sub.mkdir(parents=True, exist_ok=True)
            (sub / "tokens.txt").write_text(tokens, encoding="utf-8")
            (sub / "full.txt").write_text(out, encoding="utf-8")

        results.append(compare_text(f"lexer/{sy.name}", tokens, golden / f"{sy.stem}.tokens.golden", update))
    return results


def run_parser_tests(exe: Path, update: bool, save_only: bool) -> list[TestResult]:
    results: list[TestResult] = []
    golden = PARSER_CASES / "expected"
    for sy in sorted(PARSER_CASES.glob("*.sy")):
        rel = sy.relative_to(ROOT).as_posix()
        out = run_cmd(exe, "parserB", rel)
        sec = extract_parser_sections(out)

        sub = OUTPUT_DIR / "parser" / sy.stem
        if save_only or not update:
            sub.mkdir(parents=True, exist_ok=True)
            for k, v in sec.items():
                (sub / f"{k}.txt").write_text(v, encoding="utf-8")
            (sub / "full.txt").write_text(out, encoding="utf-8")

        for key in ("trace", "reductions", "errors"):
            results.append(
                compare_text(f"parser/{sy.name}:{key}", sec[key], golden / f"{sy.stem}.{key}.golden", update)
            )
    return results


def run_ir_tests(exe: Path, update: bool, save_only: bool) -> list[TestResult]:
    results: list[TestResult] = []
    golden = IR_CASES / "expected"
    for sy in sorted(IR_CASES.glob("*.sy")):
        rel = sy.relative_to(ROOT).as_posix()
        with tempfile.NamedTemporaryFile(suffix=".ll", delete=False) as tmp:
            tmp_path = Path(tmp.name)
        run_cmd(exe, "ir", rel, str(tmp_path))
        if not tmp_path.is_file() or tmp_path.stat().st_size == 0:
            results.append(TestResult(f"ir/{sy.name}", False, "未生成 .ll 文件"))
            continue
        actual = normalize_ir(tmp_path.read_text(encoding="utf-8"))
        tmp_path.unlink(missing_ok=True)

        sub = OUTPUT_DIR / "ir" / sy.stem
        if save_only or not update:
            sub.mkdir(parents=True, exist_ok=True)
            (sub / "output.ll").write_text(actual, encoding="utf-8")

        results.append(compare_text(f"ir/{sy.name}", actual, golden / f"{sy.stem}.ll.golden", update))
    return results


def print_summary(results: list[TestResult]) -> int:
    passed = sum(1 for r in results if r.passed)
    failed = len(results) - passed
    print("\n" + "=" * 60)
    print(f"汇总: {passed} 通过, {failed} 失败, 共 {len(results)} 项")
    print("=" * 60)
    for r in results:
        tag = "PASS" if r.passed else "FAIL"
        print(f"[{tag}] {r.name}")
        if not r.passed:
            print(r.message)
    print(f"\n截图目录: {OUTPUT_DIR.relative_to(ROOT)}/")
    return 0 if failed == 0 else 1


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--update", action="store_true", help="刷新 golden")
    ap.add_argument("--save-only", action="store_true", help="仅保存 tests/output")
    args = ap.parse_args()

    try:
        exe = find_executable()
    except FileNotFoundError as e:
        print(e, file=sys.stderr)
        return 2

    print(f"可执行文件: {exe}")
    results: list[TestResult] = []
    results.extend(run_lexer_tests(exe, args.update, args.save_only))
    results.extend(run_parser_tests(exe, args.update, args.save_only))
    results.extend(run_ir_tests(exe, args.update, args.save_only))

    if args.save_only and not args.update:
        print(f"已保存到 {OUTPUT_DIR}")
        return 0
    return print_summary(results)


if __name__ == "__main__":
    sys.exit(main())
