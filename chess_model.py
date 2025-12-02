#!/usr/bin/env python3
"""
Chess Position Evaluation Neural Network

A deep neural network for evaluating chess positions from board representations.
Input: 768-dimensional one-hot encoded board (12 piece types × 64 squares)
Output: Single evaluation score
"""

import torch
import torch.nn as nn


class ChessEvaluationNet(nn.Module):
    """
    Neural network for chess position evaluation.

    Architecture:
        Input: 768 features (12 piece types × 64 squares)
        Layer 1: Linear(768, 1024) → ReLU → BatchNorm1d → Dropout(0.1)
        Layer 2: Linear(1024, 512) → ReLU → BatchNorm1d → Dropout(0.1)
        Layer 3: Linear(512, 256) → ReLU → BatchNorm1d → Dropout(0.1)
        Layer 4: Linear(256, 128) → ReLU → BatchNorm1d
        Output: Linear(128, 1)
    """
    
    def __init__(self):
        super(ChessEvaluationNet, self).__init__()
        
        self.network = nn.Sequential(
            # Layerr 1: 768 -> 1024
            nn.Linear(768, 1024),
            nn.ReLU(),
            nn.BatchNorm1d(1024),
            nn.Dropout(0.1),
            
            # Layer 2: 1024 -> 512
            nn.Linear(1024, 512),
            nn.ReLU(),
            nn.BatchNorm1d(512),
            nn.Dropout(0.1),
            
            # Layer 3: 512 -> 256
            nn.Linear(512, 256),
            nn.ReLU(),
            nn.BatchNorm1d(256),
            nn.Dropout(0.1),
            
            # Layer 4: 256 -> 128
            nn.Linear(256, 128),
            nn.ReLU(),
            nn.BatchNorm1d(128),
            
            # Output layer
            nn.Linear(128, 1)
        )
    
    def forward(self, x):
        """
        Forward pass through the network.

        Args:
            x: Input tensor of shape (batch_size, 768)

        Returns:
            Output tensor of shape (batch_size, 1) with evaluation scores
        """
        return self.network(x)

if __name__ == "__main__":
    # Instantiate model
    model = ChessEvaluationNet()

    # Count parameters
    total_params = sum(p.numel() for p in model.parameters())
    trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)

    print("=" * 60)
    print("Chess Evaluation Network")
    print("=" * 60)
    print(f"Total parameters:     {total_params:,}")
    print(f"Trainable parameters: {trainable_params:,}")

    # Test with dummy input
    batch_size = 32
    input_features = 768

    print(f"\n{'='*60}")
    print("Testing with dummy input")
    print("=" * 60)
    print(f"Input shape: ({batch_size}, {input_features})")

    # Create dummy input
    dummy_input = torch.randn(batch_size, input_features)

    # Forward pass
    model.eval()
    with torch.no_grad():
        output = model(dummy_input)

    print(f"Output shape: {tuple(output.shape)}")
    print(f"\nOutput samples (first 5):")
    for i in range(min(5, batch_size)):
        print(f"  Position {i+1}: {output[i].item():.4f}")

    print(f"\n{'='*60}")
    print("Model Architecture")
    print("=" * 60)
    print(model)
