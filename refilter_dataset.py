import torch
import json
import chess
import numpy as np
from pathlib import Path

def fen_to_tensor(fen):
    """Convert FEN to 768-dim tensor"""
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
    print("RE-FILTERING DATASET WITH STRICTER THRESHOLDS")
    print("=" * 80)
    print(f"Input: {input_file}")
    print(f"NEW Filter: Keep only |eval| >= 0.7 AND |eval| <= 5.0")
    print(f"Output: {output_file}")
    print()
    
    # Load and filter
    print("Loading and filtering positions...")
    positions = []
    evaluations = []
    
    stats = {
        'total': 0,
        'too_boring': 0,
        'too_extreme': 0,
        'kept': 0
    }
    
    with open(input_file) as f:
        for i, line in enumerate(f):
            data = json.loads(line)
            eval_score = data['evaluation']
            stats['total'] += 1
            
            # Filter: Keep only positions with clear advantage
            if abs(eval_score) < 0.7:
                stats['too_boring'] += 1
            elif abs(eval_score) > 5.0:
                stats['too_extreme'] += 1
            else:
                positions.append(data['fen'])
                evaluations.append(eval_score)
                stats['kept'] += 1
            
            if (i + 1) % 100000 == 0:
                print(f"  Processed: {i + 1:,} | Kept: {stats['kept']:,}")
    
    print()
    print(f"Total positions: {stats['total']:,}")
    print(f"Too boring (|eval| < 0.7): {stats['too_boring']:,} ({stats['too_boring']/stats['total']*100:.1f}%)")
    print(f"Too extreme (|eval| > 5.0): {stats['too_extreme']:,} ({stats['too_extreme']/stats['total']*100:.1f}%)")
    print(f"Kept: {stats['kept']:,} ({stats['kept']/stats['total']*100:.1f}%)")
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
                print(f"  Converted: {i + 1:,}")
        except:
            continue
    
    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.float32)
    
    print(f"Final dataset: {len(X):,} positions")
    print(f"Eval range: {y.min():.2f} to {y.max():.2f}")
    print()
    
    # Split
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
    torch.save({
        'X_train': X_train,
        'y_train': y_train,
        'X_val': X_val,
        'y_val': y_val,
        'X_test': X_test,
        'y_test': y_test
    }, output_file)
    
    print("=" * 80)
    print("STRICTER FILTERING COMPLETE")
    print(f"Saved to: {output_file}")
    print("=" * 80)

if __name__ == "__main__":
    main()
