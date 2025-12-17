import torch
import torch.onnx
from chess_model import ChessEvaluationNet

print("Loading self-play trained model...")
model = ChessEvaluationNet()
checkpoint = torch.load('models/chess_eval_selfplay.pt')
model.load_state_dict(checkpoint['model_state_dict'])
model.eval()

print("Exporting to ONNX...")
dummy_input = torch.randn(1, 768)

torch.onnx.export(
    model,
    dummy_input,
    "models/chess_eval_selfplay.onnx",
    export_params=True,
    opset_version=14,
    input_names=['input'],
    output_names=['evaluation'],
    dynamic_axes={'input': {0: 'batch_size'}, 'evaluation': {0: 'batch_size'}}
)

print("✅ Model exported to: models/chess_eval_selfplay.onnx")
print(f"   Validation MAE: {checkpoint['val_mae']:.4f} pawns")
