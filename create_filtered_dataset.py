import torch
import json
import chess
import numpy as np
from pathlib import Path

def fen_to_tensor(fen):
    """Convert FEN string to 768-dimensional tensor"""
    board = chess.Board(fen)
    piece_types = [chess.PAWN, chess.KNIGHT, chess.BISHOP, chess.ROOK, chess.QUEEN, chess.KING]
    tensor = np.zeros(768, dtype=np.float32)
    
    for square in range(64):
        piece = board.piece_at(square)
        if piece:
            piece_idx = piece_types.index(piece.piece_type)
            if piece.color == chess.BLACK:
                piece_idx += 6
            tensor[piece_idx * 64 + square] = 1.0
    
    return tensor

def main():
    input_file = "data/processed/labeled_positions.jsonl"
    output_file = "data/training/chess_dataset.pt"
    
    print("=" * 80)
    print("CREATING FILTERED TRAINING DATASET")
    print("=" * 80)
    print(f"Input: {input_file}")
    print(f"Filter: Keep only positions with |eval| <= 5.0")
    print(f"Output: {output_file}")
    print()
    
    # Load positions with filtering
    print("Loading and filtering positions...")
    positions = []
    evaluations = []
    filtered_count = 0
    
    with open(input_file) as f:
        for i, line in enumerate(f):
            data = json.loads(line)
            eval_score = data['evaluation']
            
            # Filter out extreme evaluations (mate scores, huge advantages)
            if abs(eval_score) <= 5.0:
                positions.append(data['fen'])
                evaluations.append(eval_score)
            else:
                filtered_count += 1
            
            if (i + 1) % 100000 == 0:
                print(f"  Processed: {i + 1:,} | Kept: {len(positions):,} | Filtered: {filtered_count:,}")
    
    total_positions = len(positions)
    print(f"\nTotal positions kept: {total_positions:,}")
    print(f"Positions filtered: {filtered_count:,}")
    print(f"Filter rate: {filtered_count/(total_positions+filtered_count)*100:.1f}%")
    print()
    
    # Convert to tensors
    print("Converting to tensors...")
    X = []
    y = []
    
    for i, (fen, eval_score) in enumerate(zip(positions, evaluations)):
        try:
            tensor = fen_to_tensor(fen)
            X.append(tensor)
            y.append(eval_score)
            
            if (i + 1) % 100000 == 0:
                print(f"  Converted: {i + 1:,} / {total_positions:,}")
        except Exception as e:
            continue
    
    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.float32)
    
    print(f"Dataset shape: X={X.shape}, y={y.shape}")
    print(f"Evaluation range: {y.min():.2f} to {y.max():.2f}")
    print()
    
    # Split into train/val/test
    print("Splitting into train/val/test...")
    total = len(X)
    train_size = int(0.8 * total)
    val_size = int(0.1 * total)
    
    indices = np.random.permutation(total)
    X = X[indices]
    y = y[indices]
    
    X_train = torch.from_numpy(X[:train_size])
    y_train = torch.from_numpy(y[:train_size])
    X_val = torch.from_numpy(X[train_size:train_size + val_size])
    y_val = torch.from_numpy(y[train_size:train_size + val_size])
    X_test = torch.from_numpy(X[train_size + val_size:])
    y_test = torch.from_numpy(y[train_size + val_size:])
    
    print(f"  Train: {X_train.shape[0]:,}")
    print(f"  Val:   {X_val.shape[0]:,}")
    print(f"  Test:  {X_test.shape[0]:,}")
    print()
    
    # Save
    print(f"Saving to {output_file}...")
    torch.save({
        'X_train': X_train,
        'y_train': y_train,
        'X_val': X_val,
        'y_val': y_val,
        'X_test': X_test,
        'y_test': y_test
    }, output_file)
    
    print("=" * 80)
    print("FILTERED DATASET CREATED")
    print("=" * 80)

if __name__ == "__main__":
    main()
