#!/usr/bin/env python3
"""
Extract only NEW positions (not already trained on).
"""

# New positions are in these two files
new_batch1 = "data/self_play/positions_labeled.jsonl"
new_batch2 = "data/self_play/positions_labeled_batch2.jsonl"
output = "data/training/new_positions_only.jsonl"

with open(output, 'w') as out:
    # Copy batch 1
    with open(new_batch1, 'r') as f:
        for line in f:
            out.write(line)
    
    # Copy batch 2
    with open(new_batch2, 'r') as f:
        for line in f:
            out.write(line)

print(f"New positions extracted to: {output}")

# Verify count
with open(output, 'r') as f:
    count = sum(1 for line in f)
    print(f"Total new positions: {count:,}")
