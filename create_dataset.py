import torch
import json
import chess
import numpy as np
from pathlib import Path

def fen_to_tensor(fen):
    """Convert FEN string to 768-dimensional tensor (12 piece types × 64 squares)"""
    board = chess.Board(fen)
    
    # 12 piece types: 6 white (PNBRQK) + 6 black (pnbrqk)
    piece_types = [
        chess.PAWN, chess.KNIGHT, chess.BISHOP, 
        chess.ROOK, chess.QUEEN, chess.KING
    ]
    
    tensor = np.zeros(768, dtype=np.float32)
    
    for square in range(64):
        piece = board.piece_at(square)
        if piece:
            # Determine piece index (0-11)
            piece_idx = piece_types.index(piece.piece_type)
            if piece.color == chess.BLACK:
                piece_idx += 6
            
            # Set the corresponding bit in tensor
            tensor[piece_idx * 64 + square] = 1.0
    
    return tensor

def main():
    input_file = "data/processed/labeled_positions.jsonl"
    output_file = "data/training/chess_dataset.pt"
    
    print("=" * 80)
    print("CREATING TRAINING DATASET")
    print("=" * 80)
    print(f"Input: {input_file}")
    print(f"Output: {output_file}")
    print()
    
    # Load all positions
    print("Loading positions...")
    positions = []
    evaluations = []
    
    with open(input_file) as f:
        for i, line in enumerate(f):
            data = json.loads(line)
            positions.append(data['fen'])
            evaluations.append(data['evaluation'])
            
            if (i + 1) % 100000 == 0:
                print(f"  Loaded: {i + 1:,} positions")
    
    total_positions = len(positions)
    print(f"Total positions loaded: {total_positions:,}")
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
                print(f"  Converted: {i + 1:,} / {total_positions:,} ({(i+1)/total_positions*100:.1f}%)")
        except Exception as e:
            print(f"  Error on position {i}: {e}")
            continue
    
    X = np.array(X, dtype=np.float32)
    y = np.array(y, dtype=np.float32)
    
    print(f"Dataset shape: X={X.shape}, y={y.shape}")
    print()
    
    # Split into train/val/test (80/10/10)
    print("Splitting into train/val/test sets...")
    total = len(X)
    train_size = int(0.8 * total)
    val_size = int(0.1 * total)
    
    # Shuffle
    indices = np.random.permutation(total)
    X = X[indices]
    y = y[indices]
    
    # Split
    X_train = torch.from_numpy(X[:train_size])
    y_train = torch.from_numpy(y[:train_size])
    
    X_val = torch.from_numpy(X[train_size:train_size + val_size])
    y_val = torch.from_numpy(y[train_size:train_size + val_size])
    
    X_test = torch.from_numpy(X[train_size + val_size:])
    y_test = torch.from_numpy(y[train_size + val_size:])
    
    print(f"  Train: {X_train.shape[0]:,} samples")
    print(f"  Val:   {X_val.shape[0]:,} samples")
    print(f"  Test:  {X_test.shape[0]:,} samples")
    print()
    
    # Save dataset
    print(f"Saving to {output_file}...")
    Path("data/training").mkdir(parents=True, exist_ok=True)
    
    torch.save({
        'X_train': X_train,
        'y_train': y_train,
        'X_val': X_val,
        'y_val': y_val,
        'X_test': X_test,
        'y_test': y_test
    }, output_file)
    
    print("=" * 80)
    print("DATASET CREATION COMPLETE")
    print(f"Saved to: {output_file}")
    print("=" * 80)

if __name__ == "__main__":
    main()
