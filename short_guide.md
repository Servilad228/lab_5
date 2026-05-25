mkdir -p data/processed

python3 preprocess.py --input data/Questions.csv --output data/processed/docs.jsonl

python3 generate_queries.py

make clean
make

для 50_000 документов
./app experiment --type=avl --limit=50000
./app experiment --type=rb --limit=50000
./app experiment --type=btree --limit=50000

для остальных поменять значение в limit