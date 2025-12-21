#!/usr/bin/env python3
"""
Filter for MIDDLE GAME positions only (18-22 pieces).

WHY THIS RANGE:
- <18 pieces: Tactical endgames that timeout and don't help learning
- 18-22 pieces: Middle game where positional patterns matter (BEST for NN training)
- >22 pieces: Too complex, Stockfish timeouts

This gives us the highest quality training data for the neural network.
"""

def is_middlegame(fen):
    """Check if position is in the middlegame sweet spot."""
    try:
        pieces = fen.split()[0]
        total_pieces = sum(c.isalpha() for c in pieces)
        
        # Sweet spot: 18-22 pieces
        if 18 <= total_pieces <= 22:
            return True, total_pieces
        else:
            return False, total_pieces
        
    except:
        return False, 0


def main():
    input_file = "data/self_play/positions_filtered.fen"
    output_file = "data/self_play/positions_middlegame.fen"
    
    print("=" * 70)
    print("MIDDLE GAME FILTER (18-22 pieces)")
    print("=" * 70)
    print(f"Input:  {input_file}")
    print(f"Output: {output_file}")
    print()
    print("Keeping only positions with 18-22 pieces")
    print("This is the optimal range for neural network training.")
    print()
    
    # Read positions
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions: {total:,}")
    print()
    
    # Filter to middlegame
    print("Filtering for middlegame positions...")
    middlegame_positions = []
    piece_distribution = {}
    
    for fen in positions:
        is_mg, piece_count = is_middlegame(fen)
        piece_distribution[piece_count] = piece_distribution.get(piece_count, 0) + 1
        
        if is_mg:
            middlegame_positions.append(fen)
    
    # Write middlegame positions
    print(f"Writing middlegame positions to {output_file}...")
    with open(output_file, 'w') as f:
        for fen in middlegame_positions:
            f.write(fen + '\n')
    
    # Print results
    print()
    print("=" * 70)
    print("FILTERING COMPLETE")
    print("=" * 70)
    print(f"Total positions:         {total:,}")
    print(f"Middlegame (kept):       {len(middlegame_positions):,} ({100*len(middlegame_positions)/total:.1f}%)")
    print(f"Not middlegame (filtered): {total - len(middlegame_positions):,} ({100*(total-len(middlegame_positions))/total:.1f}%)")
    print()
    
    # Show distribution
    print("Piece count distribution (showing kept range):")
    for count in range(10, 33):
        if count in piece_distribution:
            pct = 100 * piece_distribution[count] / total
            status = "✓ KEPT" if 18 <= count <= 22 else "✗ FILTERED"
            bar = '#' * int(pct / 2) if 18 <= count <= 22 else ''
            print(f"  {count:2d} pieces: {piece_distribution[count]:6,} ({pct:5.1f}%) {status} {bar}")
    print()
    
    # Time estimate
    estimated_hours = (len(middlegame_positions) * 0.95) / 3600
    
    print(f"Estimated labeling time: ~{estimated_hours:.1f} hours")
    print()
    print(f"Output saved to: {output_file}")
    print("=" * 70)
    print()
    print("WHY THIS IS BETTER:")
    print("  ✓ Middlegame positions teach positional patterns")
    print("  ✓ Classical search already handles endgame tactics")
    print("  ✓ These positions actually occur in real games")
    print("  ✓ Much lower timeout rate = faster labeling")
    print()
    print("NEXT STEP: Label with Stockfish")
    print("  python3 label_middlegame_positions.py")


if __name__ == '__main__':
    main()
