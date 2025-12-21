#!/usr/bin/env python3
"""
Final filter: Remove positions with exposed kings.

EXPOSED KING = King not on its back rank
- White king should be on rank 1 (a1-h1)
- Black king should be on rank 8 (a8-h8)

If king is wandering in the middle, it's a "getting mated" tactical position
that causes Stockfish to timeout calculating 20+ move mate sequences.
"""

def has_safe_kings(fen):
    """Check if both kings are on their back ranks."""
    try:
        # FEN format: pieces/rank8/.../rank1 side castling ep halfmove fullmove
        board_part = fen.split()[0]
        ranks = board_part.split('/')
        
        # ranks[0] = rank 8 (black's back rank)
        # ranks[7] = rank 1 (white's back rank)
        
        rank_8 = ranks[0]  # Black's back rank
        rank_1 = ranks[7]  # White's back rank
        
        # Check if white king (K) is on rank 1
        white_king_safe = 'K' in rank_1
        
        # Check if black king (k) is on rank 8
        black_king_safe = 'k' in rank_8
        
        # Both kings must be on their back ranks
        if white_king_safe and black_king_safe:
            return True, "safe"
        elif not white_king_safe and not black_king_safe:
            return False, "both_kings_exposed"
        elif not white_king_safe:
            return False, "white_king_exposed"
        else:
            return False, "black_king_exposed"
        
    except:
        return False, "parse_error"


def main():
    input_file = "data/self_play/positions_middlegame.fen"
    output_file = "data/self_play/positions_final.fen"
    
    print("=" * 70)
    print("SAFE KINGS FILTER (Final Filter)")
    print("=" * 70)
    print(f"Input:  {input_file}")
    print(f"Output: {output_file}")
    print()
    print("Filtering out positions with exposed kings")
    print("(Kings not on their back ranks = tactical mate sequences)")
    print()
    
    # Read positions
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions: {total:,}")
    print()
    
    # Filter for safe kings
    print("Filtering...")
    safe_positions = []
    filtered_reasons = {}
    
    for fen in positions:
        is_safe, reason = has_safe_kings(fen)
        
        if is_safe:
            safe_positions.append(fen)
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
        print("Filter reasons:")
        for reason, count in sorted(filtered_reasons.items(), key=lambda x: -x[1]):
            print(f"  {reason:25s}: {count:,} ({100*count/total:.1f}%)")
    print()
    
    # Time estimate
    estimated_hours = (len(safe_positions) * 0.95) / 3600
    
    print(f"Estimated labeling time: ~{estimated_hours:.1f} hours")
    print()
    print(f"Output saved to: {output_file}")
    print("=" * 70)
    print()
    print("NEXT STEP: Label with Stockfish")
    print("  python3 label_final_positions.py")


if __name__ == '__main__':
    main()
