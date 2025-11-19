import torch
import time

print("=" * 60)
print("PyTorch GPU Test")
print("=" * 60)

# 1. Basic GPU Info
print("\n1. GPU Detection:")
print(f"   CUDA available: {torch.cuda.is_available()}")
if torch.cuda.is_available():
    print(f"   GPU Device: {torch.cuda.get_device_name(0)}")
    print(f"   Compute Capability: {torch.cuda.get_device_capability(0)}")
    print(f"   CUDA Version: {torch.version.cuda}")
    print(f"   PyTorch Version: {torch.__version__}")
else:
    print("   ERROR: CUDA not available!")
    exit(1)

# 2. Memory Info
print("\n2. GPU Memory:")
print(f"   Total Memory: {torch.cuda.get_device_properties(0).total_memory / 1e9:.2f} GB")
print(f"   Allocated: {torch.cuda.memory_allocated(0) / 1e9:.4f} GB")
print(f"   Cached: {torch.cuda.memory_reserved(0) / 1e9:.4f} GB")

# 3. Simple Tensor Operations
print("\n3. Tensor Operations Test:")
try:
    x = torch.randn(1000, 1000, device='cuda')
    y = torch.randn(1000, 1000, device='cuda')
    z = torch.matmul(x, y)
    
    print("   ✓ Successfully created and multiplied 1000x1000 matrices on GPU")
    print(f"   Result shape: {z.shape}")
    print(f"   Memory allocated: {torch.cuda.memory_allocated(0) / 1e6:.2f} MB")
except Exception as e:
    print(f"   ✗ ERROR: {e}")
    exit(1)

# 4. Performance Comparison (GPU vs CPU)
print("\n4. Performance Test (GPU vs CPU):")
size = 5000

print("   Testing CPU...")
x_cpu = torch.randn(size, size)
y_cpu = torch.randn(size, size)
start = time.time()
z_cpu = torch.matmul(x_cpu, y_cpu)
cpu_time = time.time() - start
print(f"   CPU Time: {cpu_time:.4f} seconds")

print("   Testing GPU...")
x_gpu = torch.randn(size, size, device='cuda')
y_gpu = torch.randn(size, size, device='cuda')
torch.cuda.synchronize()
start = time.time()
z_gpu = torch.matmul(x_gpu, y_gpu)
torch.cuda.synchronize()
gpu_time = time.time() - start
print(f"   GPU Time: {gpu_time:.4f} seconds")
print(f"   Speedup: {cpu_time / gpu_time:.2f}x faster on GPU")

# 5. Neural Network Test
print("\n5. Neural Network Test:")
try:
    model = torch.nn.Sequential(
        torch.nn.Linear(100, 256),
        torch.nn.ReLU(),
        torch.nn.Linear(256, 256),
        torch.nn.ReLU(),
        torch.nn.Linear(256, 1)
    ).to('cuda')
    
    test_input = torch.randn(32, 100, device='cuda')
    output = model(test_input)
    
    print("   ✓ Successfully created neural network on GPU")
    print(f"   Model parameters: {sum(p.numel() for p in model.parameters())}")
    print(f"   Output shape: {output.shape}")
    
    loss = output.sum()
    loss.backward()
    
    print("   ✓ Successfully computed gradients (backward pass)")
    
except Exception as e:
    print(f"   ✗ ERROR: {e}")
    exit(1)

# 6. Memory Stress Test
print("\n6. Memory Stress Test:")
try:
    large_tensor = torch.randn(10000, 10000, device='cuda')
    print(f"   ✓ Allocated 10000x10000 tensor ({large_tensor.element_size() * large_tensor.nelement() / 1e9:.2f} GB)")
    print(f"   Total GPU memory used: {torch.cuda.memory_allocated(0) / 1e9:.2f} GB")
    
    del large_tensor
    torch.cuda.empty_cache()
    print("   ✓ Successfully freed memory")
    
except Exception as e:
    print(f"   ✗ WARNING: {e}")

print("\n" + "=" * 60)
print("GPU Test Complete! ✓")
print("Your GPU is ready for training.")
print("=" * 60)
