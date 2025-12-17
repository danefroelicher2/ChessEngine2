#!/usr/bin/env python3
"""
Fine-tune the Lichess-trained model on self-play data.
Uses lower learning rate and trains ONLY on self-play positions.
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

def load_selfplay_data():
    """Load self-play positions"""
    print("Loading self-play data...")
    
    positions = []
    evaluations = []
    
    with open("data/self_play/positions_with_evals_FINAL.jsonl") as f:
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
    
    print(f"Loaded {len(X)} self-play positions")
    print(f"Eval range: [{y.min():.2f}, {y.max():.2f}]")
    
    # Shuffle with seed
    np.random.seed(42)
    indices = np.random.permutation(len(X))
    X = X[indices]
    y = y[indices]
    
    # Split 80/20 (train/val) - small dataset so no test set
    split_idx = int(0.8 * len(X))
    X_train, y_train = X[:split_idx], y[:split_idx]
    X_val, y_val = X[split_idx:], y[split_idx:]
    
    print(f"Train: {len(X_train)}, Val: {len(X_val)}")
    
    return (torch.from_numpy(X_train), torch.from_numpy(y_train),
            torch.from_numpy(X_val), torch.from_numpy(y_val))

def finetune():
    """Fine-tune model on self-play data"""
    
    print("="*60)
    print("FINE-TUNING ON SELF-PLAY DATA")
    print("="*60)
    
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f"Device: {device}")
    
    # Load self-play data
    X_train, y_train, X_val, y_val = load_selfplay_data()
    
    train_dataset = TensorDataset(X_train, y_train)
    val_dataset = TensorDataset(X_val, y_val)
    
    train_loader = DataLoader(train_dataset, batch_size=256, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=256, shuffle=False)
    
    # Load pre-trained model
    print("\nLoading Lichess-trained model...")
    model = ChessEvaluationNet().to(device)
    checkpoint = torch.load('models/chess_eval_best.pt')
    model.load_state_dict(checkpoint['model_state_dict'])
    
    # Fine-tuning settings (lower learning rate!)
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=0.0001)  # 10x lower than normal
    
    print(f"\nFine-tuning for 100 epochs...")
    print(f"Learning rate: 0.0001 (lower for fine-tuning)")
    print("="*60)
    
    best_val_loss = float('inf')
    patience = 20
    epochs_without_improvement = 0
    
    for epoch in range(1, 101):
        # Training
        model.train()
        train_loss = 0.0
        train_mae = 0.0
        
        for X_batch, y_batch in train_loader:
            X_batch = X_batch.to(device)
            y_batch = y_batch.to(device).unsqueeze(1)
            
            optimizer.zero_grad()
            outputs = model(X_batch)
            loss = criterion(outputs, y_batch)
            loss.backward()
            optimizer.step()
            
            train_loss += loss.item()
            train_mae += torch.abs(outputs - y_batch).mean().item()
        
        train_loss /= len(train_loader)
        train_mae /= len(train_loader)
        
        # Validation
        model.eval()
        val_loss = 0.0
        val_mae = 0.0
        
        with torch.no_grad():
            for X_batch, y_batch in val_loader:
                X_batch = X_batch.to(device)
                y_batch = y_batch.to(device).unsqueeze(1)
                
                outputs = model(X_batch)
                loss = criterion(outputs, y_batch)
                
                val_loss += loss.item()
                val_mae += torch.abs(outputs - y_batch).mean().item()
        
        val_loss /= len(val_loader)
        val_mae /= len(val_loader)
        
        print(f"Epoch {epoch:3d}/100 | "
              f"Train Loss: {train_loss:.4f} | Train MAE: {train_mae:.4f} | "
              f"Val Loss: {val_loss:.4f} | Val MAE: {val_mae:.4f}")
        
        # Save best model
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            epochs_without_improvement = 0
            
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict(),
                'train_loss': train_loss,
                'val_loss': val_loss,
            }, 'models/chess_eval_finetuned.pt')
            
            print(f"  → New best model saved! (Val MAE: {val_mae:.4f} pawns)")
        else:
            epochs_without_improvement += 1
            
            if epochs_without_improvement >= patience:
                print(f"\nEarly stopping after {epoch} epochs")
                break
    
    print("\n" + "="*60)
    print("FINE-TUNING COMPLETE")
    print(f"Best validation MAE: {best_val_loss:.4f}")
    print("="*60)

if __name__ == "__main__":
    finetune()
