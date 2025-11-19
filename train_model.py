#!/usr/bin/env python3
"""
Training script for chess evaluation neural network.

Trains a deep neural network to evaluate chess positions from board representations.
Includes early stopping, learning rate scheduling, and comprehensive metrics.
"""

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import TensorDataset, DataLoader
from pathlib import Path
import time
from chess_model import ChessEvaluationNet


def evaluate(model, data_loader, criterion, device):
    """
    Evaluate model on a dataset.

    Args:
        model: Neural network model
        data_loader: DataLoader for the dataset
        criterion: Loss function
        device: torch device (cuda/cpu)

    Returns:
        Tuple of (average_loss, mean_absolute_error)
    """
    model.eval()
    total_loss = 0.0
    total_mae = 0.0
    num_batches = 0

    with torch.no_grad():
        for X_batch, y_batch in data_loader:
            X_batch = X_batch.to(device)
            y_batch = y_batch.to(device).unsqueeze(1)

            outputs = model(X_batch)
            loss = criterion(outputs, y_batch)

            total_loss += loss.item()
            total_mae += torch.abs(outputs - y_batch).mean().item()
            num_batches += 1

    avg_loss = total_loss / num_batches
    avg_mae = total_mae / num_batches

    return avg_loss, avg_mae


def train_epoch(model, train_loader, criterion, optimizer, device):
    """
    Train model for one epoch.

    Args:
        model: Neural network model
        train_loader: DataLoader for training data
        criterion: Loss function
        optimizer: Optimizer
        device: torch device (cuda/cpu)

    Returns:
        Average training loss
    """
    model.train()
    total_loss = 0.0
    num_batches = 0

    for X_batch, y_batch in train_loader:
        X_batch = X_batch.to(device)
        y_batch = y_batch.to(device).unsqueeze(1)

        # Forward pass
        optimizer.zero_grad()
        outputs = model(X_batch)
        loss = criterion(outputs, y_batch)

        # Backward pass
        loss.backward()
        optimizer.step()

        total_loss += loss.item()
        num_batches += 1

    avg_loss = total_loss / num_batches
    return avg_loss


def train_model():
    """Main training function."""

    # Device selection
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("=" * 80)
    print("Chess Evaluation Model Training")
    print("=" * 80)
    print(f"Device: {device}")
    if device.type == "cuda":
        print(f"GPU: {torch.cuda.get_device_name(0)}")
    print()

    # Load data
    data_path = "data/training/chess_dataset.pt"
    print(f"Loading data from {data_path}...")
    dataset = torch.load(data_path)

    X_train = dataset['X_train']
    y_train = dataset['y_train']
    X_val = dataset['X_val']
    y_val = dataset['y_val']
    X_test = dataset['X_test']
    y_test = dataset['y_test']

    print(f"Training set: {X_train.shape[0]:,} samples")
    print(f"Validation set: {X_val.shape[0]:,} samples")
    print(f"Test set: {X_test.shape[0]:,} samples")
    print()

    # Create data loaders
    batch_size = 1024
    train_dataset = TensorDataset(X_train, y_train)
    val_dataset = TensorDataset(X_val, y_val)
    test_dataset = TensorDataset(X_test, y_test)

    train_loader = DataLoader(train_dataset, batch_size=batch_size, shuffle=True)
    val_loader = DataLoader(val_dataset, batch_size=batch_size, shuffle=False)
    test_loader = DataLoader(test_dataset, batch_size=batch_size, shuffle=False)
    
    # Initialize model
    model = ChessEvaluationNet().to(device)
    total_params = sum(p.numel() for p in model.parameters())
    print(f"Model parameters: {total_params:,}")
    print()

    # Loss function and optimizer
    criterion = nn.MSELoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    # Learning rate scheduler
    scheduler = optim.lr_scheduler.ReduceLROnPlateau(
        optimizer, mode='min', factor=0.5, patience=5, verbose=True
    )

    # Training configuration
    num_epochs = 100
    early_stopping_patience = 15
    best_val_loss = float('inf')
    patience_counter = 0
    best_epoch = 0

    # Create models directory
    models_dir = Path("models")
    models_dir.mkdir(exist_ok=True)
    best_model_path = models_dir / "chess_eval_best.pt"

    # Training loop
    print("=" * 80)
    print("Training Progress")
    print("=" * 80)
    print(f"{'Epoch':>5} | {'Train Loss':>11} | {'Val Loss':>11} | "
          f"{'MAE':>8} | {'LR':>10} | {'Time':>8}")
    print("-" * 80)

    start_time = time.time()

    for epoch in range(1, num_epochs + 1):
        epoch_start = time.time()

        # Training
        train_loss = train_epoch(model, train_loader, criterion, optimizer, device)

        # Validation
        val_loss, val_mae = evaluate(model, val_loader, criterion, device)

        # Learning rate
        current_lr = optimizer.param_groups[0]['lr']

        # Update scheduler
        scheduler.step(val_loss)

        # Epoch time
        epoch_time = time.time() - epoch_start

        # Print progress
        print(f"{epoch:5d} | {train_loss:11.6f} | {val_loss:11.6f} | "
              f"{val_mae:8.4f} | {current_lr:10.6f} | {epoch_time:6.1f}s")

        # Save best model
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            best_epoch = epoch
            patience_counter = 0

            # Save checkpoint
            checkpoint = {
                'model_state_dict': model.state_dict(),
                'epoch': epoch,
                'val_loss': val_loss,
                'val_mae': val_mae,
                'optimizer_state_dict': optimizer.state_dict(),
                'scheduler_state_dict': scheduler.state_dict()
            }
            torch.save(checkpoint, best_model_path)
        else:
            patience_counter += 1

        # Early stopping
        if patience_counter >= early_stopping_patience:
            print()
            print(f"Early stopping triggered after {epoch} epochs")
            print(f"Best model from epoch {best_epoch} with val_loss: {best_val_loss:.6f}")
            break

    total_time = time.time() - start_time
    print("-" * 80)
    print(f"Training completed in {total_time/60:.1f} minutes")
    print()

    # Load best model for testing
    print("=" * 80)
    print("Test Set Evaluation")
    print("=" * 80)
    checkpoint = torch.load(best_model_path)
    model.load_state_dict(checkpoint['model_state_dict'])
    print(f"Loaded best model from epoch {checkpoint['epoch']}")
    print(f"Best validation loss: {checkpoint['val_loss']:.6f}")
    print(f"Best validation MAE: {checkpoint['val_mae']:.4f}")
    print()

    # Evaluate on test set
    test_loss, test_mae = evaluate(model, test_loader, criterion, device)
    print(f"Test Loss: {test_loss:.6f}")
    print(f"Test MAE:  {test_mae:.4f}")
    print()

    # Show sample predictions
    print("=" * 80)
    print("Sample Predictions (first 10 from test set)")
    print("=" * 80)
    print(f"{'#':>3} | {'Actual':>8} | {'Predicted':>10} | {'Error':>8}")
    print("-" * 80)

    model.eval()
    with torch.no_grad():
        X_sample = X_test[:10].to(device)
        y_sample = y_test[:10]
        predictions = model(X_sample).squeeze().cpu()

        for i in range(10):
            actual = y_sample[i].item()
            pred = predictions[i].item()
            error = abs(actual - pred)
            print(f"{i+1:3d} | {actual:8.4f} | {pred:10.4f} | {error:8.4f}")

    print("=" * 80)
    print(f"Model saved to: {best_model_path}")
    print("=" * 80)

if __name__ == "__main__":
    Path("models").mkdir(exist_ok=True)
    train_model()
