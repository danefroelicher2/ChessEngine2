#!/usr/bin/env python3
"""
FINAL COMPREHENSIVE FILTER

After extensive testing, we've identified timeout patterns:
1. Very low piece count (<16) with exposed kings = tactical mate sequences
2. Very high piece count (>28) = too many variations
3. Exposed kings (not on back 2 ranks) = tactical complications

KEEPS:
- 16-28 pieces (realistic endgame through early opening)
- Kings on ranks 1-2 (white) or 7-8 (black) - includes castled + slightly advanced

This gives maximum training diversity while filtering timeout risks.
"""

def is_safe_position(fen):
    """Check if position is safe to label."""
    try:
        parts = fen.split()
        board_part = parts[0]
        ranks = board_part.split('/')
        
        # Count total pieces
        total_pieces = sum(c.isalpha() for c in board_part)
        
        # FILTER 1: Piece count range (16-28)
        if total_pieces < 16 or total_pieces > 28:
            if total_pieces < 16:
                return False, "too_few_pieces", total_pieces
            else:
                return False, "too_many_pieces", total_pieces
        
        # FILTER 2: King safety
        # White king (K) should be on rank 1 or 2
        # Black king (k) should be on rank 8 or 7
        
        rank_1 = ranks[7]  # White's back rank
        rank_2 = ranks[6]  # White's second rank
        rank_7 = ranks[1]  # Black's second rank  
        rank_8 = ranks[0]  # Black's back rank
        
        white_king_safe = ('K' in rank_1) or ('K' in rank_2)
        black_king_safe = ('k' in rank_8) or ('k' in rank_7)
        
        if not white_king_safe:
            return False, "white_king_exposed", total_pieces
        if not black_king_safe:
            return False, "black_king_exposed", total_pieces
        
        # Position passed all filters
        return True, "safe", total_pieces
        
    except:
        return False, "parse_error", 0


def main():
    input_file = "data/self_play/positions_filtered.fen"  # After 3+ queen filter
    output_file = "data/self_play/positions_ready.fen"
    
    print("=" * 70)
    print("FINAL COMPREHENSIVE FILTER")
    print("=" * 70)
    print(f"Input:  {input_file}")
    print(f"Output: {output_file}")
    print()
    print("Filter criteria:")
    print("  ✓ Keep: 16-28 pieces (diverse game phases)")
    print("  ✓ Keep: Kings on ranks 1-2 (white) or 7-8 (black)")
    print("  ✗ Skip: <16 pieces (tactical endgames)")
    print("  ✗ Skip: >28 pieces (too complex)")
    print("  ✗ Skip: Exposed kings (tactical mate sequences)")
    print()
    
    # Read positions
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions: {total:,}")
    print()
    
    # Filter positions
    print("Filtering...")
    safe_positions = []
    filtered_reasons = {}
    piece_distribution = {}
    
    for fen in positions:
        is_safe, reason, pieces = is_safe_position(fen)
        
        if is_safe:
            safe_positions.append(fen)
            piece_distribution[pieces] = piece_distribution.get(pieces, 0) + 1
        else:
            filtered_reasons[reason] = filtered_reasons.get(reason, 0) + 1
    
    # Write safe positions
    print(f"Writing safe positions to {output_file}...")
    with open(output_file, 'w') as f:
        for fen in safe_positions:
            f.write(fen + '\n')
    
    # Print results
    print()
    print("=" * 70)
    print("FILTERING COMPLETE")
    print("=" * 70)
    print(f"Total positions:      {total:,}")
    print(f"Safe (kept):          {len(safe_positions):,} ({100*len(safe_positions)/total:.1f}%)")
    print(f"Filtered out:         {total - len(safe_positions):,} ({100*(total-len(safe_positions))/total:.1f}%)")
    print()
    
    if filtered_reasons:
        print("Filtered reasons:")
        for reason, count in sorted(filtered_reasons.items(), key=lambda x: -x[1]):
            print(f"  {reason:25s}: {count:,} ({100*count/total:.1f}%)")
    print()
    
    # Show piece distribution of kept positions
    if piece_distribution:
        print("Piece distribution (kept positions):")
        for pieces in sorted(piece_distribution.keys()):
            count = piece_distribution[pieces]
            pct = 100 * count / len(safe_positions)
            bar = '#' * int(pct / 2)
            print(f"  {pieces:2d} pieces: {count:5,} ({pct:5.1f}%) {bar}")
    print()
    
    # Time estimate
    # Expecting ~5-10% timeout rate with this filter
    expected_success_rate = 0.92  # 92% success, 8% timeout
    expected_labeled = int(len(safe_positions) * expected_success_rate)
    estimated_hours = (len(safe_positions) * 0.95) / 3600
    
    print(f"Expected labeled positions: ~{expected_labeled:,} (assuming 8% timeout rate)")
    print(f"Estimated labeling time: ~{estimated_hours:.1f} hours")
    print()
    print(f"Output saved to: {output_file}")
    print("=" * 70)
    print()
    print("This is our FINAL filter - balanced for diversity and success rate.")
    print()
    print("NEXT STEP: Label with Stockfish")
    print("  python3 label_ready_positions.py")


if __name__ == '__main__':
    main()
