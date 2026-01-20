import argparse
import random
import shutil
import string
from pathlib import Path

ALPH = string.ascii_letters + string.digits + " _-./"
USERS = ["alice", "bob", "carol", "dave", "erin", "mallory", "trent", "peggy"]


def random_text(rng: random.Random, n: int) -> str:
    return "".join(rng.choices(ALPH, k=n))


def generate_text_file(
    rng: random.Random,
    path: Path,
    size_bytes: int,
    match_prob: float,
    patterns: list[str],
) -> tuple[int, int]:
    """
    Написать около size_bytes текста построчно.
    -> (bytes_written, matches_inserted).
    """
    path.parent.mkdir(parents=True, exist_ok=True)

    bytes_written = 0
    matches = 0

    base_line_len = 120
    base = random_text(rng, base_line_len)

    with path.open("w", encoding="utf-8", newline="\n") as f:
        while bytes_written < size_bytes:
            line = base

            if rng.random() < 0.2:
                line = random_text(rng, rng.randint(60, 200))

            if rng.random() < match_prob:
                pattern = rng.choice(patterns)
                pos = min(len(line), rng.randint(0, len(line)))
                line = line[:pos] + pattern + line[pos:]
                matches += 1

            line += "\n"
            f.write(line)
            bytes_written += len(line)

    return bytes_written, matches


def generate_binary_file(rng: random.Random, path: Path, size_bytes: int) -> int:
    """
    Сгенерить бинарный файл с нулевыми байтами.
    """
    path.parent.mkdir(parents=True, exist_ok=True)

    data = bytearray(rng.getrandbits(8) for _ in range(size_bytes))
    if size_bytes > 0:
        for i in range(0, min(size_bytes, 32), 7):
            data[i] = 0

    with path.open("wb") as f:
        f.write(data)

    return size_bytes


def generate_dataset_small(root: Path, rng: random.Random) -> None:
    """
    Сгенерить много маленьких файлов: ~20-30MB
    """
    base = root / "small"
    if base.exists():
        shutil.rmtree(base)
    base.mkdir(parents=True, exist_ok=True)

    # Для --regex "ERROR|WARN" и для отдельного литерала "TODO"
    patterns = [
        "ERROR",
        "WARN",
        "TODO",
        f"user={rng.choice(USERS)}",
        f"code={rng.randint(100,999)}",
    ]

    dirs = 100
    files_per_dir = 120        # итого 12k файлов
    file_size = 2048           # около 24MB
    match_prob = 0.03          # редкие совпадения

    for d in range(dirs):
        dpath = base / f"dir_{d:03d}"
        for i in range(files_per_dir):
            fpath = dpath / f"file_{i:04d}.log"
            generate_text_file(rng, fpath, file_size, match_prob, patterns)

    # Скрытая директория
    generate_text_file(rng, base / ".hidden" / "secret.log", 16_384, 0.2, patterns)
    # Бинарные файлы
    generate_binary_file(rng, base / "bin" / "data0.bin", 32_768)
    generate_binary_file(rng, base / "bin" / "data1.bin", 32_768)


def generate_dataset_mix(root: Path, rng: random.Random) -> None:
    """
    Сгенерить файлы с разными размерами: ~60-90MB
    """
    base = root / "mix"
    if base.exists():
        shutil.rmtree(base)
    base.mkdir(parents=True, exist_ok=True)

    patterns = [
        "ERROR",
        "WARN",
        "TODO",
        "timeout=500ms",
        f"user={rng.choice(USERS)}",
        f"rids={rng.randint(10_000, 99_999)}",
        f"ip=192.168.{rng.randint(0,255)}.{rng.randint(0,255)}",
    ]

    # Несколько уровней директорий, разные размеры
    for a in range(20):
        for b in range(10):
            dpath = base / f"bin_{a:02d}" / f"shard_{b:02d}"
            # 20*10*10 = 2000 файлов
            for i in range(10):
                # от 8KB до 128KB
                sz = rng.choice([8_192, 16_384, 32_768, 65_536, 131_072])
                # больше матчей
                match_prob = 0.08 if sz >= 32_768 else 0.05
                ext = rng.choice([".log", ".txt", ".trace"]) # разные расширения
                fpath = dpath / f"part_{i:02d}{ext}"
                generate_text_file(rng, fpath, sz, match_prob, patterns)

    # Для проверки --exclude "build/*"
    for i in range(200):
        generate_text_file(rng, base / "build" / f"obj_{i:04d}.txt", 4096, 0.02, patterns)

    # Скрытая директория
    generate_text_file(rng, base / ".cache" / "cache.log", 128_000, 0.15, patterns)


def generate_dataset_big(root: Path, rng: random.Random) -> None:
    """
    Сгенерить несколько больших файлов: ~64MB
    """
    base = root / "big"
    if base.exists():
        shutil.rmtree(base)
    base.mkdir(parents=True, exist_ok=True)

    patterns = [
        "ERROR",
        "WARN",
        "TODO",
        "exception=std::runtime_error",
        f"user={rng.choice(USERS)}",
        f"status={rng.choice([200,204,301,400,401,403,404,500,503])}",
    ]

    # 4 файла по 16MB, т.е. 64MB
    for i in range(4):
        fpath = base / f"huge_{i:02d}.log"
        # Много матчей для большой нагрузки
        generate_text_file(rng, fpath, 16 * 1024 * 1024, 0.06, patterns)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="benchmark/data", help="output directory (default: benchmark/data)")
    ap.add_argument("--seed", type=int, default=123, help="random seed (default: 123)")
    ap.add_argument("--only", choices=["all", "small", "mix", "big"], default="all")
    args = ap.parse_args()

    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    rng = random.Random(args.seed)

    if args.only in ("all", "small"):
        print("[gen] small ...")
        generate_dataset_small(out, rng)
    if args.only in ("all", "mix"):
        print("[gen] mix ...")
        generate_dataset_mix(out, rng)
    if args.only in ("all", "big"):
        print("[gen] big ...")
        generate_dataset_big(out, rng)

    print(f"[gen] done: {out.resolve()}")


if __name__ == "__main__":
    main()
