#!/usr/bin/env python3
"""
实验一 逻辑测试驱动

不比对文本格式，只比对「操作序列 -> 最终向量内容」这个逻辑结果：
每个场景都跑一次真实的 Vector_test / List_test 可执行文件，
从输出里取出最后一次 show 的元素序列和最后一次 size 的值来判定。

用法： python3 cli_logic_test.py <build_dir_bin>
       例：python3 cli_logic_test.py ../../build/bin
"""

import os
import re
import struct
import subprocess
import sys
import tempfile

BIN = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else "../../build/bin")
VEC = os.path.join(BIN, "Vector_test")
LST = os.path.join(BIN, "List_test")

PASS, FAIL = [], []


def run(exe, cmds, cwd):
    """把指令喂给交互程序，返回 (退出码, stdout 行列表, stderr)"""
    p = subprocess.run([exe], input="\n".join(cmds) + "\n",
                       capture_output=True, text=True, cwd=cwd, timeout=60)
    return p.returncode, p.stdout.split("\n"), p.stderr


def parse_ints(line):
    line = line.strip()
    if not line:
        return []
    return [int(t) for t in line.split()]


def final_state(exe, cmds, cwd):
    """在指令序列末尾追加 show+size，取最终元素序列和元素个数"""
    rc, out, err = run(exe, cmds + ["show", "size", "exit"], cwd)
    # 输出尾部固定是： <元素序列> / <个数> / End executing!
    idx = None
    for i, ln in enumerate(out):
        if ln.strip() == "End executing!":
            idx = i
    if idx is None or idx < 2:
        return rc, None, None
    size = parse_ints(out[idx - 1])
    elems = parse_ints(out[idx - 2])
    return rc, elems, (size[0] if size else None)


def check(name, got, want, extra=""):
    if got == want:
        PASS.append(name)
        print(f"  [ok]   {name}")
    else:
        FAIL.append(name)
        print(f"  [FAIL] {name}\n         want={want}\n         got ={got} {extra}")


def check_rc(name, rc, want_nonzero):
    ok = (rc != 0) if want_nonzero else (rc == 0)
    if ok:
        PASS.append(name)
        print(f"  [ok]   {name} (exit={rc})")
    else:
        FAIL.append(name)
        print(f"  [FAIL] {name} (exit={rc})")


# ============================================================
def vector_tests(d):
    print("\n=== Vector: 手册 2.4 节样例序列（逐条比对元素序列）===")
    src = os.path.join(d, "exp1_example_input.txt")
    with open(src, "w") as f:
        f.write("5\n3 65 678 32 6\n")

    # 手册规定的每一步期望元素序列
    steps = [
        (["load exp1_example_input.txt"], None),
        (["show"], [3, 65, 678, 32, 6]),
        (["push_back 23"], [3, 65, 678, 32, 6, 23]),
        (["pop_back"], [3, 65, 678, 32, 6]),
        (["insert 2 123"], [3, 123, 65, 678, 32, 6]),
        (["erase 3"], [3, 123, 678, 32, 6]),
        (["update 2 120"], [3, 120, 678, 32, 6]),
        (["size"], 5),
    ]
    cmds = ["load exp1_example_input.txt"]
    for cmd, want in steps[1:]:
        rc, out, _ = run(VEC, cmds + cmd + ["exit"], d)
        line = [l for l in out if l.strip() and "successfully" not in l
                and "End executing" not in l]
        got = parse_ints(line[-1]) if line else []
        got = got[0] if isinstance(want, int) and len(got) == 1 else got
        check(f"手册样例: {' '.join(cmd)}", got, want)
        cmds += cmd

    # 保存后的文件内容（逻辑：saved 的应该就是当前向量）
    run(VEC, ["load exp1_example_input.txt", "push_back 23", "pop_back",
              "insert 2 123", "erase 3", "update 2 120",
              "save exp1_example_output.txt", "exit"], d)
    with open(os.path.join(d, "exp1_example_output.txt")) as f:
        toks = f.read().split()
    check("手册样例: save 后文件内容", [int(t) for t in toks], [5, 3, 120, 678, 32, 6])

    print("\n=== Vector: 用 Vector_data.txt 做操作序列测试 ===")
    # Vector_data.txt = 3 65 678 32 6
    base = ["load Vector_data.txt"]
    loaded = [
        ("insert 1 x 头插", ["insert 1 100"], [100, 3, 65, 678, 32, 6]),
        ("insert size+1 x 尾插", ["insert 6 100"], [3, 65, 678, 32, 6, 100]),
        ("erase 1 删头", ["erase 1"], [65, 678, 32, 6]),
        ("erase size 删尾", ["erase 5"], [3, 65, 678, 32]),
        ("update 1/update 5", ["update 1 0", "update 5 0"], [0, 65, 678, 32, 0]),
        ("负数元素", ["push_back -7", "push_back -1"], [3, 65, 678, 32, 6, -7, -1]),
        ("重复元素 push 3 push 3 erase 2", ["push_back 3", "push_back 3", "erase 2"],
         [3, 678, 32, 6, 3, 3]),
        ("pop_back 直到空", ["pop_back"] * 5, []),
        ("反复 push/pop", ["push_back 1", "pop_back", "push_back 2", "pop_back"],
         [3, 65, 678, 32, 6]),
    ]
    for name, ops, want in loaded:
        rc, elems, _ = final_state(VEC, base + ops, d)
        check(name, elems, want, f"(exit={rc})")

    print("\n=== Vector: 空向量（不 load）===")
    for name, ops, want in [
        ("空向量 show/size", [], []),
        ("空向量 pop_back 后仍为空", ["pop_back"], []),
        ("空向量 erase 1 后仍为空", ["erase 1"], []),
        ("空向量 update 1 9 后仍为空", ["update 1 9"], []),
        ("push_back 到空向量", ["push_back 9"], [9]),
        ("空向量连续 push 12 个（跨一次扩容）",
         ["push_back %d" % i for i in range(1, 13)], list(range(1, 13))),
    ]:
        rc, elems, _ = final_state(VEC, ops, d)
        check(name, elems, want, f"(exit={rc})")

    print("\n=== Vector: 越界位置参数的行为（当前实现是静默忽略）===")
    for name, ops in [
        ("insert 0", ["insert 0 99"]),
        ("insert size+2", ["insert 7 99"]),
        ("insert 1000000", ["insert 1000000 99"]),
        ("erase 0", ["erase 0"]),
        ("erase size+1", ["erase 6"]),
        ("erase 1000000", ["erase 1000000"]),
        ("update 0", ["update 0 99"]),
        ("update size+1", ["update 6 99"]),
    ]:
        rc, elems, _ = final_state(VEC, base + ops, d)
        check(f"越界 {name} 不改变向量（也不崩）", (rc, elems), (0, [3, 65, 678, 32, 6]))

    print("\n=== Vector: 扩容 / 缩容（逻辑层面）===")
    # 连续 push_back 300 个，应完整跨过多次倍增
    rc, elems, size = final_state(VEC, ["push_back %d" % i for i in range(1, 301)], d)
    check("push_back 1..300 元素个数", size, 300)
    check("push_back 1..300 内容完整", elems, list(range(1, 301)))

    # push 300 后 erase 到只剩 10 个，检查缩容后数据没丢
    ops = ["push_back %d" % i for i in range(1, 301)] + ["erase 1"] * 290
    rc, elems, size = final_state(VEC, ops, d)
    check("push 300 再删 290 个后内容", elems, list(range(291, 301)))

    # 缩容后继续插入
    rc, elems, size = final_state(VEC, ops + ["push_back 999"], d)
    check("缩容后继续 push_back", elems, list(range(291, 301)) + [999])

    print("\n=== Vector: 文件读写往返 ===")
    rc, _, _ = run(VEC, base + ["save rt.txt", "save rt.bin", "exit"], d)
    for ext in ("txt", "bin"):
        rc, elems, size = final_state(VEC, [f"load rt.{ext}"], d)
        check(f".{ext} 往返内容一致", (size, elems), (5, [3, 65, 678, 32, 6]))
    # txt <-> bin 交叉
    rc, elems, _ = final_state(VEC, ["load rt.txt", "save cross.bin", "load cross.bin"], d)
    check("txt -> bin -> 读回", elems, [3, 65, 678, 32, 6])
    rc, elems, _ = final_state(VEC, ["load rt.bin", "save cross.txt", "load cross.txt"], d)
    check("bin -> txt -> 读回", elems, [3, 65, 678, 32, 6])
    # 大小写扩展名
    rc, elems, _ = final_state(VEC, base + ["save up.TXT", "load up.TXT"], d)
    check("大写扩展名 .TXT", elems, [3, 65, 678, 32, 6])
    rc, elems, _ = final_state(VEC, base + ["save up.BIN", "load up.BIN"], d)
    check("大写扩展名 .BIN", elems, [3, 65, 678, 32, 6])

    # 元素个数为 0 的文件
    with open(os.path.join(d, "zero.txt"), "w") as f:
        f.write("0\n")
    rc, elems, size = final_state(VEC, ["load zero.txt"], d)
    check("load 0 元素文件", (size, elems), (0, []))
    rc, elems, _ = final_state(VEC, ["load zero.txt", "push_back 5"], d)
    check("load 0 元素文件后 push_back", elems, [5])

    # 重复 load 覆盖
    rc, elems, _ = final_state(VEC, ["load zero.txt", "load Vector_data.txt"], d)
    check("连续 load 用新文件覆盖", elems, [3, 65, 678, 32, 6])

    print("\n=== Vector: 大数据量（10 万元素）===")
    big = list(range(1, 100001))
    with open(os.path.join(d, "big.txt"), "w") as f:
        f.write("%d\n%s\n" % (len(big), " ".join(map(str, big))))
    with open(os.path.join(d, "big.bin"), "wb") as f:
        f.write(struct.pack("<i", len(big)))
        f.write(struct.pack("<%di" % len(big), *big))

    for ext in ("txt", "bin"):
        rc, out, err = run(VEC, [f"load big.{ext}", f"save bigout.{ext}",
                                 "size", "exit"], d)
        idx = [i for i, l in enumerate(out) if l.strip() == "End executing!"]
        got_size = int(out[idx[0] - 1]) if idx else None
        check(f"10 万元素 .{ext}：元素个数", got_size, len(big))
        with open(os.path.join(d, f"bigout.{ext}")) as f:
            toks = f.read().split() if ext == "txt" else None
        if ext == "txt":
            check("10 万元素 .txt：save 后内容一致",
                  [int(t) for t in toks], [len(big)] + big)
        else:
            with open(os.path.join(d, "bigout.bin"), "rb") as f:
                raw = f.read()
            n = struct.unpack("<i", raw[:4])[0]
            vals = list(struct.unpack("<%di" % n, raw[4:]))
            check("10 万元素 .bin：save 后内容一致", (n, vals), (len(big), big))

    print("\n=== Vector: 错误路径 ===")
    rc, out, err = run(VEC, ["load nope.txt", "exit"], d)
    check_rc("不存在的文件 -> 非 0 退出", rc, True)
    rc, out, err = run(VEC, ["load data.docx", "exit"], d)
    check_rc("不支持的扩展名 -> 非 0 退出", rc, True)
    combined = "\n".join(out) + err
    check("不支持的扩展名有提示", "Unsupported format!" in combined, True)

    # 截断的 txt：头写 5 实际 3 个
    with open(os.path.join(d, "short.txt"), "w") as f:
        f.write("5\n3 65 678\n")
    rc, elems, size = final_state(VEC, ["load short.txt"], d)
    ok = (size == 5 and elems[:3] == [3, 65, 678] and len(elems) == 5)
    print(f"  [note] 截断 txt（头 5 实际 3 个数）-> size={size} elems={elems}")
    print(f"         {'头部与已有数据一致，但后 2 个元素来自未初始化内存' if ok else '行为异常'}")
    # 空 txt
    with open(os.path.join(d, "empty.txt"), "w") as f:
        f.write("")
    rc, out, err = run(VEC, ["load empty.txt", "size", "exit"], d)
    check("空 txt -> 拒绝并给出提示", "Broken file!" in "\n".join(out) + err, True)
    # 负数头
    with open(os.path.join(d, "negcount.txt"), "w") as f:
        f.write("-3\n1 2 3\n")
    rc, out, err = run(VEC, ["load negcount.txt", "size", "exit"], d)
    check("负数元素个数 -> 拒绝并给出提示", "Broken file!" in "\n".join(out) + err, True)
    # 裸 n 无数据的 bin
    with open(os.path.join(d, "bare.bin"), "wb") as f:
        f.write(struct.pack("<i", 1000000))
    rc, out, err = run(VEC, ["load bare.bin", "size", "exit"], d)
    txt = "\n".join(out)
    bad = "1000000" in txt
    print(f"  [note] 只含 4 字节 n=1000000 的 .bin -> "
          f"{'被当成 1000000 个元素接受（后 4MB 是未初始化内存）' if bad else '被拒绝'}")


# ============================================================
def list_tests(d):
    print("\n=== List: 用 List_data.txt 做操作序列测试 ===")
    # List_data.txt = 13 25 5 25 4
    base = ["load List_data.txt"]
    D = [13, 25, 5, 25, 4]
    cases = [
        ("load 后内容", [], D),
        ("insertAsFirst", ["insertAsFirst 99"], [99] + D),
        ("insertAsLast", ["insertAsLast 99"], D + [99]),
        ("首尾各插一个", ["insertAsFirst 1", "insertAsLast 2"], [1] + D + [2]),
        ("空表 show", None, []),
    ]
    for name, ops, want in cases:
        cmds = base if ops is not None else []
        if ops:
            cmds = base + ops
        rc, elems, _ = final_state(LST, cmds, d)
        check(name, elems, want, f"(exit={rc})")

    print("\n=== List: 文件读写往返 ===")
    run(LST, base + ["save lrt.txt", "save lrt.bin", "exit"], d)
    for ext in ("txt", "bin"):
        rc, elems, size = final_state(LST, [f"load lrt.{ext}"], d)
        check(f".{ext} 往返内容一致", (size, elems), (5, D))
    rc, elems, _ = final_state(LST, ["load lrt.txt", "save lcross.bin", "load lcross.bin"], d)
    check("txt -> bin -> 读回", elems, D)
    rc, elems, _ = final_state(LST, ["load lrt.bin", "save lcross.txt", "load lcross.txt"], d)
    check("bin -> txt -> 读回", elems, D)

    # 空表存盘再读回
    rc, elems, size = final_state(LST, ["save lempty.txt", "load lempty.txt"], d)
    check("空表 save/load", (size, elems), (0, []))
    rc, elems, size = final_state(LST, ["save lempty.bin", "load lempty.bin"], d)
    check("空表 save/load (.bin)", (size, elems), (0, []))

    # 连续 load 覆盖
    rc, elems, _ = final_state(LST, ["load lempty.txt", "load List_data.txt"], d)
    check("连续 load 覆盖", elems, D)

    print("\n=== List: 错误路径 ===")
    rc, out, err = run(LST, ["load nope.txt", "exit"], d)
    check_rc("不存在的文件 -> 非 0 退出", rc, True)
    rc, out, err = run(LST, ["load data.docx", "exit"], d)
    check_rc("不支持的扩展名 -> 非 0 退出", rc, True)
    with open(os.path.join(d, "empty.txt"), "w") as f:
        f.write("")
    rc, out, err = run(LST, ["load empty.txt", "exit"], d)
    check_rc("空 txt -> 非 0 退出", rc, True)
    with open(os.path.join(d, "negcount.txt"), "w") as f:
        f.write("-3\n1 2 3\n")
    rc, elems, size = final_state(LST, ["load negcount.txt"], d)
    print(f"  [note] 负数元素个数的 txt -> exit={rc} size={size} elems={elems}")
    with open(os.path.join(d, "bare.bin"), "wb") as f:
        f.write(struct.pack("<i", 200000))
    rc, out, err = run(LST, ["load bare.bin", "size", "exit"], d)
    txt = "\n".join(out)
    print(f"  [note] 只含 4 字节 n=200000 的 .bin -> "
          f"{'被接受' if '200000' in txt else '被拒绝'}")


# ============================================================
def main():
    with tempfile.TemporaryDirectory() as d:
        # 把用户的数据文件复制进临时工作目录（程序按相对路径读）
        for f in ("Vector_data.txt", "List_data.txt"):
            with open(os.path.join(BIN, f)) as s, open(os.path.join(d, f), "w") as t:
                t.write(s.read())
        vector_tests(d)
        list_tests(d)

    print("\n" + "=" * 60)
    print(f"通过 {len(PASS)} 项，失败 {len(FAIL)} 项")
    if FAIL:
        print("失败项：")
        for n in FAIL:
            print("  -", n)
    return 1 if FAIL else 0


if __name__ == "__main__":
    sys.exit(main())
