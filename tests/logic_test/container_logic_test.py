#!/usr/bin/env python3
"""
容器逻辑测试驱动：把每个用例放在独立进程里跑，用 ASan/UBSan 编译，
崩溃、断言失败、内存错误都能单独定位。

用法： python3 container_logic_test.py [workspace_root]
默认 workspace_root = ../..
"""

import os
import subprocess
import sys

ROOT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "../..")
INC = os.path.join(ROOT, "container")
HERE = os.path.dirname(os.path.abspath(__file__))

# 每个用例：用例名 -> 期望（"ok" 正常通过；"nonzero" 期望非 0 退出）
VEC_CASES = {
    "vec_copy_ctor": "ok",
    "vec_copy_assign": "ok",
    "vec_self_assign": "ok",
    "vec_copyFrom_range": "ok",
    "vec_copyFrom_invalid": "ok",
    "vec_copyFrom_invalid_dirty": "ok",
    "vec_assign_different_sizes": "ok",
    "vec_expand_integrity": "ok",
    "vec_shrink_integrity": "ok",
    "vec_insert_erase": "ok",
    "vec_out_of_range_noop": "ok",
    "vec_reverse": "ok",
    "vec_const_index": "ok",
    "vec_empty_ops": "ok",
    "vec_load_zero_then_push": "ok",
}
LIST_CASES = {
    "list_basic": "ok",
    "list_circular": "ok",
    "list_insert_variants": "ok",
    "list_remove_edges": "ok",
    "list_remove_sentinel": "ok",
    "list_clear_reuse": "ok",
    "list_copy_ctor": "ok",
    "list_copy_assign": "ok",
    "list_range_ctor": "ok",
    "list_index": "ok",
    "list_index_oob": "nonzero",
    "list_self_assign": "ok",
    "list_insert_around_sentinels": "ok",
    "list_big": "ok",
    "list_empty_ops": "ok",
}

FLAGS = ["-std=c++17", "-g", "-O1", "-fsanitize=address,undefined",
         "-fno-omit-frame-pointer", "-Wall", "-Wextra"]

# 头文件可用性检查：(名称, 源码, 是否应当编译通过)
# 作为后续实验要复用的头文件，重复包含也应当能正常编译。
COMPILE_CASES = [
    ("两个头文件各包含一次", '#include "Vector.hpp"\n#include "List.hpp"\nint main(){return 0;}', True),
    ("Vector.hpp 重复包含", '#include "Vector.hpp"\n#include "Vector.hpp"\nint main(){return 0;}', True),
    ("List.hpp 重复包含", '#include "List.hpp"\n#include "List.hpp"\nint main(){return 0;}', True),
]

results = []


def check_includes():
    print("\n=== 头文件可用性（后续实验要同时复用两个容器）===")
    for name, src, should_compile in COMPILE_CASES:
        path = os.path.join(HERE, "_inc.cpp")
        with open(path, "w") as f:
            f.write(src)
        p = subprocess.run(["g++", "-std=c++17", "-fsyntax-only", "-I", INC, path],
                           capture_output=True, text=True)
        compiled = (p.returncode == 0)
        good = (compiled == should_compile)
        results.append((name, good, "" if good else
                        ("期望编译通过但失败" if should_compile else "期望报错但通过了")))
        print(f"  [{'ok' if good else 'FAIL'}] {name}"
              + ("" if good else f"  ({'期望通过' if should_compile else '期望报错'})"))
        if good:
            print(f"         当前行为：{'编译通过' if compiled else '报错（redefinition）'}")
        os.remove(path)


def build(src, out):
    cmd = ["g++"] + FLAGS + ["-I", INC, src, "-o", out]
    p = subprocess.run(cmd, capture_output=True, text=True)
    return p.returncode == 0, p.stderr


def run_case(exe, name, want):
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=1:abort_on_error=0")
    try:
        p = subprocess.run([exe, name], capture_output=True, text=True,
                           timeout=120, env=env)
    except subprocess.TimeoutExpired:
        return False, "超时（可能是死循环）"
    out, err = p.stdout.strip(), p.stderr
    if "LeakSanitizer" in err:
        first = [l for l in err.split("\n") if "SUMMARY" in l]
        return False, "内存泄漏: " + (first[0] if first else "见 LeakSanitizer 报告")
    if "AddressSanitizer" in err or "runtime error" in err:
        first = [l for l in err.split("\n")
                 if "ERROR:" in l or "runtime error" in l]
        return False, (first[0] if first else "ASan/UBSan 报错")
    if "FAIL:" in out:
        return False, [l for l in out.split("\n") if l.startswith("FAIL:")][0]
    if want == "ok":
        if p.returncode == 0 and out.endswith("OK"):
            return True, "OK"
        return False, f"exit={p.returncode} out={out!r}"
    if want == "nonzero":
        return (p.returncode != 0), f"exit={p.returncode}"
    return False, "未知期望"


def main():
    ok, err = build(os.path.join(HERE, "vector_logic_test.cpp"),
                    os.path.join(HERE, "_vec_test"))
    if not ok:
        print("vector_logic_test 编译失败：\n", err)
        return 1
    ok, err = build(os.path.join(HERE, "list_logic_test.cpp"),
                    os.path.join(HERE, "_list_test"))
    if not ok:
        print("list_logic_test 编译失败：\n", err)
        return 1

    for label, cases, exe in (("Vector", VEC_CASES, "_vec_test"),
                              ("List", LIST_CASES, "_list_test")):
        print(f"\n=== {label} 容器逻辑测试（ASan + UBSan）===")
        for name, want in cases.items():
            good, msg = run_case(os.path.join(HERE, exe), name, want)
            results.append((name, good, msg))
            print(f"  [{'ok' if good else 'FAIL'}] {name}"
                  + ("" if good else f"\n         {msg}"))

    check_includes()

    for f in ("_vec_test", "_list_test"):
        p = os.path.join(HERE, f)
        if os.path.exists(p):
            os.remove(p)

    bad = [r for r in results if not r[1]]
    print("\n" + "=" * 60)
    print(f"通过 {len(results) - len(bad)} 项，失败 {len(bad)} 项")
    for n, _, m in bad:
        print(f"  - {n}: {m}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
