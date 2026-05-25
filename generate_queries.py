import json
import random
import sys
import os

def generate(docs_path, output_path):
    all_tokens = set()
    print(f"Открываем файл: {docs_path}")
    
    if not os.path.exists(docs_path):
        print(f"ОШИБКА: Файл {docs_path} не существует!")
        return
        
    try:
        with open(docs_path, 'r', encoding='utf-8') as f:
            lines_read = 0
            for _ in range(50000): 
                line = f.readline()
                if not line: break
                lines_read += 1
                try:
                    doc = json.loads(line)
                    all_tokens.update(doc.get("tokens", []))
                except json.JSONDecodeError:
                    continue
                    
        print(f"Прочитано строк: {lines_read}")
        print(f"Найдено уникальных токенов: {len(all_tokens)}")
        
    except Exception as e:
        print(f"Ошибка при чтении: {e}")
        sys.exit(1)

    tokens = list(all_tokens)
    if not tokens:
        print("ОШИБКА: Токены не найдены. Файл docs.jsonl пустой или имеет неверный формат.")
        print("Запустите препроцессинг заново: python3 preprocess.py --input data/Questions.csv --output data/processed/docs.jsonl")
        return
        
    with open(output_path, 'w', encoding='utf-8') as f:
        for _ in range(333):
            f.write(f"{random.choice(tokens)}\n")
        for _ in range(333):
            f.write(f"{random.choice(tokens)} {random.choice(tokens)}\n")
        for _ in range(334):
            f.write(f"{random.choice(tokens)} {random.choice(tokens)} {random.choice(tokens)}\n")
            
    print(f"Сгенерировано 1000 запросов в {output_path}")

if __name__ == "__main__":
    generate("data/processed/docs.jsonl", "data/queries.txt")