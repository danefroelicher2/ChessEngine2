import torch
import torch.onnx

# Load the trained model
class ChessEvaluationNet(torch.nn.Module):
    def __init__(self):
        super(ChessEvaluationNet, self).__init__()
        self.fc1 = torch.nn.Linear(768, 512)
        self.bn1 = torch.nn.BatchNorm1d(512)
        self.dropout1 = torch.nn.Dropout(0.2)
        self.fc2 = torch.nn.Linear(512, 256)
        self.bn2 = torch.nn.BatchNorm1d(256)
        self.dropout2 = torch.nn.Dropout(0.2)
        self.fc3 = torch.nn.Linear(256, 128)
        self.bn3 = torch.nn.BatchNorm1d(128)
        self.dropout3 = torch.nn.Dropout(0.2)
        self.fc4 = torch.nn.Linear(128, 1)
        
    def forward(self, x):
        x = torch.relu(self.bn1(self.fc1(x)))
        x = self.dropout1(x)
        x = torch.relu(self.bn2(self.fc2(x)))
        x = self.dropout2(x)
        x = torch.relu(self.bn3(self.fc3(x)))
        x = self.dropout3(x)
        x = self.fc4(x)
        return x.squeeze()

print("Loading trained model...")
model = ChessEvaluationNet()
checkpoint = torch.load('models/chess_eval_best.pt')
model.load_state_dict(checkpoint['model_state_dict'])
model.eval()

print(f"Model info:")
print(f"  Best epoch: {checkpoint['epoch']}")
print(f"  Val MAE: {checkpoint['val_mae']:.4f} pawns")
print()

# Create dummy input
dummy_input = torch.randn(1, 768)

print("Exporting to ONNX...")
torch.onnx.export(
    model,
    dummy_input,
    "models/chess_eval_hybrid.onnx",
    export_params=True,
    opset_version=11,
    input_names=['board_input'],
    output_names=['evaluation'],
    dynamic_axes={'board_input': {0: 'batch_size'}}
)

print("✓ Model exported to: models/chess_eval_hybrid.onnx")
print()
print("Next steps:")
print("1. Update engine.cpp to use hybrid evaluation")
print("2. Upload chess_eval_hybrid.onnx to Render")
print("3. Test on live site!")
