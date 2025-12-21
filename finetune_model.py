#!/usr/bin/env python3
"""
Fine-tune the existing self-play model on NEW positions only.
"""

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import TensorDataset, DataLoader
import json
import chess
import numpy as np
from pathlib import Path
from chess_model import ChessEvaluationNet

def board_to_tensor(board):
    """Convert chess board to 768-dimensional input vector"""
    tensor = np.zeros(768, dtype=np.float32)
    
    piece_idx = {
        chess.PAWN: 0, chess.KNIGHT: 1, chess.BISHOP: 2,
        chess.ROOK: 3, chess.QUEEN: 4, chess.KING: 5
    }
    
    for square in chess.SQUARES:
        piece = board.piece_at(square)
        if piece:
            color_offset = 0 if piece.color == chess.WHITE else 6
            piece_type = piece_idx[piece.piece_type]
            idx = (piece_type + color_offset) * 64 + square
            tensor[idx] = 1.0
    
    return tensor


def load_new_data():
    """Load the NEW 5,273 positions"""
    print("Loading new positions...")
    
    positions = []
    evaluations = []
    
    with open("data/training/new_positions_only.jsonl") as f:
        for line in f:
            data = json.loads(line)
            board = chess.Board(data['fen'])
            tensor = board_to_tensor(board)
            positions.append(tensor)
            
            # Clamp evaluation between -10 and +10
            eval_score = max(-10.0, min(10.0, data['evaluation']))
            evaluations.append(eval_score)
    
    X = np.array(positions, dtype=np.float32)
    y = np.array(evaluations, dtype=np.float32)
    
    print(f"Loaded {len(X):,} new positions")
    print(f"Eval range: [{y.min():.2f}, {y.max():.2f}]")
    
    # Shuffle
    np.random.seed(42)
    indices = np.random.permutation(len(X))
    X = X[indices]
    y = y[indices]
    
    # Split 80/20 (train/val)
    split_idx = int(0.8 * len(X))
    X_train, y_train = X[:split_idx], y[:split_idx]
    X_val, y_val = X[split_idx:], y[split_idx:]
    
    print(f"Train: {len(X_train):,}, Val: {len(X_val):,}")
    
    return X_train, y_train, X_val, y_val


def finetune():
    """Fine-tune the existing model"""
    
    # Load existing model
    print("\n" + "="*60)
    print("LOADING EXISTING MODEL")
    print("="*60)
    
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f"Using device: {device}")
    
    model = ChessEvaluationNet().to(device)
    
    # Load the trained weights - FIXED: access model_state_dict
    checkpoint = torch.load('models/chess_eval_selfplay.pt', map_location=device)
    
    # Check if it's a checkpoint dictionary or raw state_dict
    if isinstance(checkpoint, dict) and 'model_state_dict' in checkpoint:
        model.load_state_dict(checkpoint['model_state_dict'])
        print(f"✓ Loaded checkpoint from epoch {checkpoint.get('epoch', 'unknown')}")
        print(f"  Previous val MAE: {checkpoint.get('val_mae', 'unknown'):.3f}")
    else:
        model.load_state_dict(checkpoint)
        print("✓ Loaded model weights")
    
    # Load new data
    X_train, y_train, X_val, y_val = load_new_data()
    
    # Create data loaders
    train_dataset = TensorDataset(
        torch.FloatTensor(X_train),
        torch.FloatTensor(y_train)
    )
    val_dataset = TensorDataset(
        torch.FloatTensor(X_val),
        torch.FloatTensor(y_val)
    )
    
    train_loader = DataLoader(train_dataset, batch_size=32, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=32)
    
    # Fine-tuning settings (LOWER learning rate than training from scratch)
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=0.0001)  # 10x lower than initial training
    
    print("\n" + "="*60)
    print("FINE-TUNING")
    print("="*60)
    print(f"Learning rate: 0.0001 (lower for fine-tuning)")
    print(f"Epochs: 50")
    print(f"Early stopping: patience=10")
    print()
    
    best_val_loss = float('inf')
    patience = 10
    patience_counter = 0
    
    for epoch in range(50):
        # Training
        model.train()
        train_loss = 0
        for X_batch, y_batch in train_loader:
            X_batch = X_batch.to(device)
            y_batch = y_batch.to(device)
            
            optimizer.zero_grad()
            outputs = model(X_batch).squeeze()
            loss = criterion(outputs, y_batch)
            loss.backward()
            optimizer.step()
            
            train_loss += loss.item()
        
        train_loss /= len(train_loader)
        
        # Validation
        model.eval()
        val_loss = 0
        with torch.no_grad():
            for X_batch, y_batch in val_loader:
                X_batch = X_batch.to(device)
                y_batch = y_batch.to(device)
                outputs = model(X_batch).squeeze()
                loss = criterion(outputs, y_batch)
                val_loss += loss.item()
        
        val_loss /= len(val_loader)
        
        # Convert to MAE for readability
        train_mae = np.sqrt(train_loss)
        val_mae = np.sqrt(val_loss)
        
        print(f"Epoch {epoch+1:2d} | Train MAE: {train_mae:.3f} | Val MAE: {val_mae:.3f}", end="")
        
        # Early stopping
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            patience_counter = 0
            torch.save(model.state_dict(), 'models/chess_eval_finetuned.pt')
            print(" ← BEST")
        else:
            patience_counter += 1
            print(f" (patience: {patience_counter}/{patience})")
            
            if patience_counter >= patience:
                print(f"\nEarly stopping at epoch {epoch+1}")
                break
    
    print("\n" + "="*60)
    print("FINE-TUNING COMPLETE")
    print("="*60)
    print(f"Best validation MAE: {np.sqrt(best_val_loss):.3f} pawns")
    print(f"Model saved: models/chess_eval_finetuned.pt")
    print()
    print("COMPARISON:")
    print(f"  Original model (7,503 pos):  0.35 MAE")
    print(f"  Fine-tuned (+5,273 pos):     {np.sqrt(best_val_loss):.3f} MAE")
    print()
    print("NEXT STEP: Export to ONNX")
    print("  python3 export_finetuned.py")


if __name__ == '__main__':
    finetune()
