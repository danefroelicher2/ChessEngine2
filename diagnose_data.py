import json
import numpy as np

print("Analyzing training data...")
print("=" * 60)

evaluations = []
with open('data/processed/positions_with_evals.jsonl') as f:
    for line in f:
        data = json.loads(line)
        evaluations.append(data['evaluation'])

evaluations = np.array(evaluations)

print(f"Total positions: {len(evaluations):,}")
print(f"\nEvaluation statistics:")
print(f"  Mean: {np.mean(evaluations):.2f}")
print(f"  Median: {np.median(evaluations):.2f}")
print(f"  Std dev: {np.std(evaluations):.2f}")
print(f"  Min: {np.min(evaluations):.2f}")
print(f"  Max: {np.max(evaluations):.2f}")

print(f"\nDistribution:")
print(f"  Between -10 and -5: {np.sum((evaluations <= -5) & (evaluations >= -10)):,}")
print(f"  Between -5 and -2: {np.sum((evaluations < -2) & (evaluations > -5)):,}")
print(f"  Between -2 and -0.5: {np.sum((evaluations < -0.5) & (evaluations >= -2)):,}")
print(f"  Between -0.5 and +0.5: {np.sum((evaluations >= -0.5) & (evaluations <= 0.5)):,}")
print(f"  Between +0.5 and +2: {np.sum((evaluations > 0.5) & (evaluations <= 2)):,}")
print(f"  Between +2 and +5: {np.sum((evaluations > 2) & (evaluations < 5)):,}")
print(f"  Between +5 and +10: {np.sum((evaluations >= 5) & (evaluations <= 10)):,}")

print(f"\nExtreme values (>5 or <-5): {np.sum(np.abs(evaluations) > 5):,}")
print(f"Percentage of extreme values: {100 * np.sum(np.abs(evaluations) > 5) / len(evaluations):.1f}%")
