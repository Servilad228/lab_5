import argparse
import json
import re
from pathlib import Path


def extract_text(value) -> str:
    # тг хранит text либо строкой, либо списком частей
    if isinstance(value, str):
        return value

    if isinstance(value, list):
        parts = []
        for item in value:
            if isinstance(item, str):
                parts.append(item)
            elif isinstance(item, dict):
                parts.append(item.get("text", ""))
        return "".join(parts)

    return ""


def tokenize(text: str, max_tokens: int) -> list[str]:
    # приводим текст к единому виду
    text = text.lower()

    # сохраняем важные программистские токены: c++ -> cpp, c# -> csharp
    text = text.replace("++", "pp")
    text = text.replace("#", "sharp")

    # убираем ссылки и пунктуацию
    text = re.sub(r"https?://\S+", " ", text)
    text = re.sub(r"[^\w\s]", " ", text)

    # берём токены длиной хотя бы 2 символа
    tokens = [w for w in text.split() if len(w) > 1]
    return tokens[:max_tokens]


def unique_tokens(tokens: list[str]) -> list[str]:
    # убираем повторы внутри одного поста, сохраняя порядок
    seen = set()
    result = []

    for token in tokens:
        if token not in seen:
            seen.add(token)
            result.append(token)

    return result


def make_title(post_id: int, text: str) -> str:
    # делаем короткий заголовок для вывода результата поиска
    preview = " ".join(text.split())

    # убираем символы, которые могут ломать простой C-парсер
    preview = preview.replace('"', "'").replace("\\", "")

    if len(preview) > 90:
        preview = preview[:87] + "..."

    if not preview:
        return f"TG post #{post_id}"

    return f"TG post #{post_id}: {preview}"


def preprocess(input_path: Path, output_path: Path, max_posts: int | None, max_tokens: int) -> None:
    # создаём папку для результата, если её ещё нет
    output_path.parent.mkdir(parents=True, exist_ok=True)

    with input_path.open("r", encoding="utf-8") as f:
        data = json.load(f)

    count = 0

    with output_path.open("w", encoding="utf-8") as out:
        for msg in data.get("messages", []):
            if max_posts is not None and count >= max_posts:
                break

            # Берём только обычные сообщения, не service-записи
            if msg.get("type") != "message":
                continue

            text = extract_text(msg.get("text"))

            # Пропускаем посты без текста
            if not text.strip():
                continue

            tokens = tokenize(text, max_tokens=max_tokens)
            tokens = unique_tokens(tokens)

            if not tokens:
                continue

            post_id = int(msg["id"])

            # Формат совместим с основным docs.jsonl
            doc = {
                "doc_id": post_id,
                "title": make_title(post_id, text),
                "tokens": tokens,
            }

            out.write(json.dumps(doc, ensure_ascii=False) + "\n")
            count += 1

    print(f"Готово. Текстовых постов: {count}")
    print(f"Результат: {output_path}")


def main() -> None:
    # Аргументы командной строки
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--max-posts", type=int, default=None)
    parser.add_argument("--max-tokens", type=int, default=120)

    args = parser.parse_args()

    preprocess(
        input_path=args.input,
        output_path=args.output,
        max_posts=args.max_posts,
        max_tokens=args.max_tokens,
    )


if __name__ == "__main__":
    main()