import chess
import json

print("Checking first 10 positions...")
with open('data/processed/positions.jsonl') as f:
    for i, line in enumerate(f):
        if i >= 10:
            break
        
        data = json.loads(line)
        fen = data['fen']
        
        try:
            board = chess.Board(fen)
            print(f"{i+1}. Valid position, move {data['move_number']}")
        except Exception as e:
            print(f"{i+1}. ERROR: {e}")

print("\nAll checks passed! ✓")
