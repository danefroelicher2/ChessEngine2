#!/usr/bin/env python3
"""Train ONLY on self-play data (from scratch)"""

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import TensorDataset, DataLoader
import json
import chess
import numpy as np
from chess_model import ChessEvaluationNet

def board_to_tensor(board):
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

print("="*60)
print("TRAINING ON SELF-PLAY DATA (FROM SCRATCH)")
print("="*60)

# Load data
print("Loading self-play positions...")
positions, evaluations = [], []

with open("data/self_play/positions_with_evals_FINAL.jsonl") as f:
    for line in f:
        data = json.loads(line)
        board = chess.Board(data['fen'])
        positions.append(board_to_tensor(board))
        evaluations.append(max(-10.0, min(10.0, data['evaluation'])))

X = torch.from_numpy(np.array(positions, dtype=np.float32))
y = torch.from_numpy(np.array(evaluations, dtype=np.float32))

print(f"Loaded {len(X)} positions")

# Shuffle and split 80/20
torch.manual_seed(42)
indices = torch.randperm(len(X))
X, y = X[indices], y[indices]

split = int(0.8 * len(X))
X_train, y_train = X[:split], y[:split]
X_val, y_val = X[split:], y[split:]

print(f"Train: {len(X_train)}, Val: {len(X_val)}\n")

# Setup
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
model = ChessEvaluationNet().to(device)
criterion = nn.MSELoss()
optimizer = optim.Adam(model.parameters(), lr=0.001)

train_loader = DataLoader(TensorDataset(X_train, y_train), batch_size=128, shuffle=True)
val_loader = DataLoader(TensorDataset(X_val, y_val), batch_size=128)

print("Training for 200 epochs...")
print("="*60)

best_val_mae = float('inf')
patience = 30
no_improve = 0

for epoch in range(1, 201):
    # Train
    model.train()
    train_loss = train_mae = 0.0
    
    for X_batch, y_batch in train_loader:
        X_batch, y_batch = X_batch.to(device), y_batch.to(device).unsqueeze(1)
        
        optimizer.zero_grad()
        outputs = model(X_batch)
        loss = criterion(outputs, y_batch)
        loss.backward()
        optimizer.step()
        
        train_loss += loss.item()
        train_mae += torch.abs(outputs - y_batch).mean().item()
    
    train_loss /= len(train_loader)
    train_mae /= len(train_loader)
    
    # Validate
    model.eval()
    val_loss = val_mae = 0.0
    
    with torch.no_grad():
        for X_batch, y_batch in val_loader:
            X_batch, y_batch = X_batch.to(device), y_batch.to(device).unsqueeze(1)
            outputs = model(X_batch)
            loss = criterion(outputs, y_batch)
            val_loss += loss.item()
            val_mae += torch.abs(outputs - y_batch).mean().item()
    
    val_loss /= len(val_loader)
    val_mae /= len(val_loader)
    
    if epoch % 10 == 0 or val_mae < best_val_mae:
        print(f"Epoch {epoch:3d}/200 | Train MAE: {train_mae:.4f} | Val MAE: {val_mae:.4f}")
    
    if val_mae < best_val_mae:
        best_val_mae = val_mae
        no_improve = 0
        torch.save({
            'epoch': epoch,
            'model_state_dict': model.state_dict(),
            'optimizer_state_dict': optimizer.state_dict(),
            'val_mae': val_mae,
        }, 'models/chess_eval_selfplay.pt')
        if epoch % 10 == 0:
            print(f"  → Best model saved!")
    else:
        no_improve += 1
        if no_improve >= patience:
            print(f"\nEarly stopping at epoch {epoch}")
            break

print("\n" + "="*60)
print(f"✅ TRAINING COMPLETE")
print(f"Best validation MAE: {best_val_mae:.4f} pawns")
print("Model saved to: models/chess_eval_selfplay.pt")
print("="*60)
