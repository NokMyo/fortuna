#!/usr/bin/env python3
import argparse
import csv
import gzip
import hashlib
import io
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "resources" / "lotto-history.csv.gz"
OUTPUT = ROOT / "build" / "embedded_history_data.inc"
EXPECTED_SHA256 = "10fe08fa5f52528e9fe2724662c1a859abbdd9ad42d1b36eac7d46a3e3d1f36b"
EXPECTED_DRAWS = 1242
EXPECTED_LAST_ROUND = 1242
EXPECTED_HEADER = ["round", "n1", "n2", "n3", "n4", "n5", "n6", "bonus"]


def load_and_validate():
    raw = gzip.decompress(SOURCE.read_bytes())
    digest = hashlib.sha256(raw).hexdigest()
    if digest != EXPECTED_SHA256:
        raise SystemExit(f"embedded history SHA-256 mismatch: {digest}")

    text = raw.decode("utf-8")
    rows = list(csv.reader(io.StringIO(text, newline="")))
    if not rows or rows[0] != EXPECTED_HEADER:
        raise SystemExit("embedded history header mismatch")
    data = rows[1:]
    if len(data) != EXPECTED_DRAWS:
        raise SystemExit(f"embedded history draw count mismatch: {len(data)}")

    expected_round = 1
    for line_no, row in enumerate(data, start=2):
        if len(row) != 8:
            raise SystemExit(f"embedded history column error at line {line_no}")
        try:
            values = [int(value) for value in row]
        except ValueError as exc:
            raise SystemExit(f"embedded history integer error at line {line_no}") from exc
        round_no, *rest = values
        numbers, bonus = rest[:6], rest[6]
        if round_no != expected_round:
            raise SystemExit(
                f"embedded history round error at line {line_no}: "
                f"expected {expected_round}, got {round_no}"
            )
        if numbers != sorted(numbers) or len(set(numbers)) != 6:
            raise SystemExit(f"embedded history main-number error at round {round_no}")
        if any(number < 1 or number > 45 for number in numbers):
            raise SystemExit(f"embedded history main-number range error at round {round_no}")
        if bonus < 1 or bonus > 45 or bonus in numbers:
            raise SystemExit(f"embedded history bonus error at round {round_no}")
        expected_round += 1

    if data[-1][0] != str(EXPECTED_LAST_ROUND):
        raise SystemExit("embedded history last round mismatch")
    return raw, text


def write_assembly(text):
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with OUTPUT.open("w", encoding="utf-8", newline="\n") as handle:
        for line in text.splitlines(keepends=True):
            escaped = line.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")
            handle.write(f'.ascii "{escaped}"\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()
    raw, text = load_and_validate()
    if not args.verify_only:
        write_assembly(text)
    print(
        f"PASS embedded history: {EXPECTED_DRAWS} draws, "
        f"last round {EXPECTED_LAST_ROUND}, sha256={hashlib.sha256(raw).hexdigest()}"
    )


if __name__ == "__main__":
    main()
