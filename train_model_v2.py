import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import TensorDataset, DataLoader
import time

class ChessEvaluationNet(nn.Module):
    """Simpler neural network for better generalization"""
    def __init__(self):
        super(ChessEvaluationNet, self).__init__()
        
        # Smaller, more focused architecture
        self.fc1 = nn.Linear(768, 512)
        self.bn1 = nn.BatchNorm1d(512)
        self.dropout1 = nn.Dropout(0.2)
        
        self.fc2 = nn.Linear(512, 256)
        self.bn2 = nn.BatchNorm1d(256)
        self.dropout2 = nn.Dropout(0.2)
        
        self.fc3 = nn.Linear(256, 128)
        self.bn3 = nn.BatchNorm1d(128)
        self.dropout3 = nn.Dropout(0.2)
        
        self.fc4 = nn.Linear(128, 1)
        
    def forward(self, x):
        x = torch.relu(self.bn1(self.fc1(x)))
        x = self.dropout1(x)
        
        x = torch.relu(self.bn2(self.fc2(x)))
        x = self.dropout2(x)
        
        x = torch.relu(self.bn3(self.fc3(x)))
        x = self.dropout3(x)
        
        x = self.fc4(x)
        return x.squeeze()

def train_epoch(model, train_loader, criterion, optimizer, device):
    model.train()
    total_loss = 0
    total_mae = 0
    
    for X_batch, y_batch in train_loader:
        X_batch, y_batch = X_batch.to(device), y_batch.to(device)
        
        optimizer.zero_grad()
        predictions = model(X_batch)
        loss = criterion(predictions, y_batch)
        
        loss.backward()
        optimizer.step()
        
        total_loss += loss.item()
        total_mae += torch.abs(predictions - y_batch).mean().item()
    
    avg_loss = total_loss / len(train_loader)
    avg_mae = total_mae / len(train_loader)
    return avg_loss, avg_mae

def validate(model, val_loader, criterion, device):
    model.eval()
    total_loss = 0
    total_mae = 0
    
    with torch.no_grad():
        for X_batch, y_batch in val_loader:
            X_batch, y_batch = X_batch.to(device), y_batch.to(device)
            
            predictions = model(X_batch)
            loss = criterion(predictions, y_batch)
            
            total_loss += loss.item()
            total_mae += torch.abs(predictions - y_batch).mean().item()
    
    avg_loss = total_loss / len(val_loader)
    avg_mae = total_mae / len(val_loader)
    return avg_loss, avg_mae

def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("=" * 80)
    print("CHESS EVALUATION MODEL TRAINING (V2 - SIMPLIFIED)")
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
    batch_size = 2048  # Larger batches for stability
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
    optimizer = optim.Adam(model.parameters(), lr=0.0001)  # Lower learning rate
    
    # Learning rate scheduler
    scheduler = optim.lr_scheduler.ReduceLROnPlateau(
        optimizer, mode='min', factor=0.5, patience=3  # More aggressive scheduling
    )
    
    # Training configuration
    num_epochs = 50
    early_stopping_patience = 8  # Less patience
    best_val_mae = float('inf')
    epochs_without_improvement = 0
    
    print("Starting training...")
    print("Lower learning rate (0.0001) for better convergence")
    print("Simpler model for better generalization")
    print("=" * 80)
    
    start_time = time.time()
    
    for epoch in range(num_epochs):
        epoch_start = time.time()
        
        # Train
        train_loss, train_mae = train_epoch(model, train_loader, criterion, optimizer, device)
        
        # Validate
        val_loss, val_mae = validate(model, val_loader, criterion, device)
        
        # Learning rate scheduling
        scheduler.step(val_mae)
        current_lr = optimizer.param_groups[0]['lr']
        
        epoch_time = time.time() - epoch_start
        
        print(f"Epoch {epoch+1:3d}/{num_epochs} | "
              f"Train Loss: {train_loss:.4f} | Train MAE: {train_mae:.4f} | "
              f"Val Loss: {val_loss:.4f} | Val MAE: {val_mae:.4f} | "
              f"LR: {current_lr:.6f} | Time: {epoch_time:.1f}s")
        
        # Early stopping
        if val_mae < best_val_mae:
            best_val_mae = val_mae
            epochs_without_improvement = 0
            
            # Save best model
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict(),
                'train_loss': train_loss,
                'val_loss': val_loss,
                'val_mae': val_mae,
            }, 'models/chess_eval_best.pt')
            
            print(f"  → New best model saved! (Val MAE: {val_mae:.4f} pawns)")
        else:
            epochs_without_improvement += 1
            
            if epochs_without_improvement >= early_stopping_patience:
                print(f"\nEarly stopping triggered after {epoch+1} epochs")
                print(f"Validation MAE stopped improving (best: {best_val_mae:.4f})")
                break
    
    total_time = time.time() - start_time
    
    print("=" * 80)
    print("TRAINING COMPLETE")
    print(f"Total time: {total_time/3600:.2f} hours")
    print(f"Best validation MAE: {best_val_mae:.4f} pawns")
    print()
    
    # Test on test set
    print("Evaluating on test set...")
    checkpoint = torch.load('models/chess_eval_best.pt')
    model.load_state_dict(checkpoint['model_state_dict'])
    
    test_loss, test_mae = validate(model, test_loader, criterion, device)
    print(f"Test Loss: {test_loss:.4f}")
    print(f"Test MAE: {test_mae:.4f} pawns")
    print()
    
    # Show if this is better than last attempt
    if test_mae < 1.2:
        print("✓ SUCCESS! This model is better than the previous attempt (1.2 MAE)")
    else:
        print("⚠ Model performance similar to or worse than previous attempt")
    
    print("=" * 80)

if __name__ == "__main__":
    main()
