import torch
import torch.onnx
from chess_model import ChessEvaluationNet

print("Loading trained model...")
model = ChessEvaluationNet()
checkpoint = torch.load('models/chess_eval_best.pt')
model.load_state_dict(checkpoint['model_state_dict'])
model.eval()

print(f"Model loaded successfully!")
print(f"  - Best epoch: {checkpoint['epoch']}")
print(f"  - Validation MAE: {checkpoint['val_mae']:.4f} pawns")

# Create dummy input (batch_size=1, features=768)
dummy_input = torch.randn(1, 768)

print("\nExporting to ONNX format...")
torch.onnx.export(
    model,
    dummy_input,
    "models/chess_eval.onnx",
    export_params=True,
    opset_version=11,
    input_names=['board_input'],
    output_names=['evaluation'],
    dynamic_axes={'board_input': {0: 'batch_size'}}
)

print("✓ Model exported successfully!")
print("  Location: models/chess_eval.onnx")

# Verify the export
import onnx
onnx_model = onnx.load("models/chess_eval.onnx")
onnx.checker.check_model(onnx_model)
print("✓ ONNX model validated")
