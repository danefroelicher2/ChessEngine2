import chess
import json
import torch
import numpy as np
from pathlib import Path

def board_to_tensor(board):
    """
    Convert chess board to 768-dimensional input vector
    12 piece types (6 pieces × 2 colors) × 64 squares = 768 features
    """
    tensor = np.zeros(768, dtype=np.float32)
    
    piece_idx = {
        chess.PAWN: 0, chess.KNIGHT: 1, chess.BISHOP: 2,
        chess.ROOK: 3, chess.QUEEN: 4, chess.KING: 5
    }
    
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            # Color offset: white = 0-5, black = 6-11
            color_offset = 0 if piece.color == chess.WHITE else 6
            piece_type = piece_idx[piece.piece_type]
            
            # Feature index: (piece_type + color_offset) * 64 + square
            idx = (piece_type + color_offset) * 64 + square
            tensor[idx] = 1.0
    
    return tensor

def prepare_dataset():
    """Load positions and convert to tensors"""
    input_file = "data/processed/positions_with_evals.jsonl"
    output_dir = Path("data/training")
    output_dir.mkdir(parents=True, exist_ok=True)

    print("Loading and converting positions...")
    print("=" * 60)

    positions = []
    evaluations = []

    with open(input_file) as f:
        for i, line in enumerate(f):
            data = json.loads(line)

            # Convert FEN to board
            board = chess.Board(data['fen'])

            # Convert board to tensor
            tensor = board_to_tensor(board)
            positions.append(tensor)

            # Store evaluation (clamp between -10 and +10)
            eval_score = max(-10.0, min(10.0, data['evaluation']))
            evaluations.append(eval_score)

            if (i + 1) % 50000 == 0:
                print(f"Processed: {i+1:,} positions")

    # Convert to numpy arrays
    X = np.array(positions, dtype=np.float32)
    y = np.array(evaluations, dtype=np.float32)

    print(f"\n{'='*60}")
    print("Dataset Statistics")
    print("=" * 60)
    print(f"Dataset shapes:")
    print(f"  X: {X.shape}")
    print(f"  y: {y.shape}")
    print(f"\nEvaluation distribution:")
    print(f"  Mean: {y.mean():.4f}")
    print(f"  Std:  {y.std():.4f}")
    print(f"  Min:  {y.min():.4f}")
    print(f"  Max:  {y.max():.4f}")

    # Shuffle data with random seed 42
    print(f"\nShuffling data with seed 42...")
    np.random.seed(42)
    indices = np.random.permutation(len(X))
    X = X[indices]
    y = y[indices]

    # Split into train/validation/test (80/10/10)
    n = len(X)
    train_end = int(0.8 * n)
    val_end = int(0.9 * n)

    X_train, y_train = X[:train_end], y[:train_end]
    X_val, y_val = X[train_end:val_end], y[train_end:val_end]
    X_test, y_test = X[val_end:], y[val_end:]

    print(f"\n{'='*60}")
    print("Dataset Splits")
    print("=" * 60)
    print(f"Training set:   {len(X_train):7,} samples ({len(X_train)/n*100:.1f}%)")
    print(f"Validation set: {len(X_val):7,} samples ({len(X_val)/n*100:.1f}%)")
    print(f"Test set:       {len(X_test):7,} samples ({len(X_test)/n*100:.1f}%)")
    print(f"Total:          {n:7,} samples")

    # Save as PyTorch tensors
    print(f"\nSaving dataset to {output_dir / 'chess_dataset.pt'}...")
    torch.save({
        'X_train': torch.from_numpy(X_train),
        'y_train': torch.from_numpy(y_train),
        'X_val': torch.from_numpy(X_val),
        'y_val': torch.from_numpy(y_val),
        'X_test': torch.from_numpy(X_test),
        'y_test': torch.from_numpy(y_test)
    }, output_dir / 'chess_dataset.pt')

    file_size_mb = (output_dir / 'chess_dataset.pt').stat().st_size / (1024 * 1024)
    print(f"\n{'='*60}")
    print(f"Dataset saved successfully!")
    print(f"  Location: {output_dir / 'chess_dataset.pt'}")
    print(f"  File size: {file_size_mb:.2f} MB")
    print("=" * 60)

if __name__ == "__main__":
    prepare_dataset()