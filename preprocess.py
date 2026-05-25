"""
Препроцессинг датасета Stack Overflow для инвертированного индекса.

Читает Questions.csv, токенизирует текст (title + body),
сохраняет результат в JSONL-файл.

Запуск:
    python preprocess.py --input data/Questions.csv --output data/processed/docs.jsonl
    python preprocess.py --input data/Questions.csv --output data/processed/docs.jsonl --limit 50000
"""

import argparse
import csv
import html
import json
import re
import sys
from pathlib import Path


def strip_html(text: str) -> str: # чтобы html-мусор не попадал в индекс
    text = html.unescape(text)
    return re.sub(r"<[^>]+>", " ", text)


def tokenize(text: str) -> list[str]:
    text = strip_html(text)
    text = text.lower()

    text = text.replace("++", "pp") # довольно важный контекст
    text = text.replace("#", "sharp") 

    text = re.sub(r"[^\w\s]", " ", text)
    return [w for w in text.split() if len(w) > 1] # чтобы не пропали например go, os итд


def preprocess(input_path: Path, output_path: Path, limit: int | None) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    count = 0

    #csv.field_size_limit(sys.maxsize)

    with open(input_path, "r", encoding="utf-8", errors="ignore", newline="") as f, \
         open(output_path, "w", encoding="utf-8") as out:

        reader = csv.DictReader(f)

        for row in reader:
            if limit is not None and count >= limit:
                break

            raw_id = row.get("Id", "").strip()
            title = row.get("Title", "").strip()
            body = row.get("Body", "").strip()

            if not raw_id:
                continue

            tokens = tokenize(title + " " + body)

            if not tokens:
                continue

            doc = {
                "doc_id": int(raw_id),
                "title": title,
                "tokens": tokens
            }

            out.write(json.dumps(doc, ensure_ascii=False) + "\n")
            count += 1

            if count % 10_000 == 0:
                print(f"Обработано: {count} документов", file=sys.stderr)

    print(f"Готово. Всего документов: {count}", file=sys.stderr)
    print(f"Результат: {output_path}", file=sys.stderr)


def main() -> None:
    parser = argparse.ArgumentParser(description="Препроцессинг Stack Overflow датасета")
    parser.add_argument("--input", required=True, type=Path, help="Путь к Questions.csv")
    parser.add_argument("--output", required=True, type=Path, help="Путь к выходному JSONL-файлу")
    parser.add_argument("--limit", type=int, default=None, help="Максимальное количество документов")
    args = parser.parse_args()

    if not args.input.exists():
        print(f"Ошибка: файл не найден: {args.input}", file=sys.stderr)
        sys.exit(1)

    preprocess(args.input, args.output, args.limit)


if __name__ == "__main__":
    main()