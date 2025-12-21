#!/usr/bin/env python3
"""
Filter out unrealistic chess positions before Stockfish labeling.

FILTERS OUT:
- Positions with 3+ queens (unrealistic pawn promotions)
- Positions with >32 pieces (invalid)

WHY: These positions waste Stockfish time (40s timeouts) and hurt training quality
"""

def is_valid_position(fen):
    """
    Check if position is realistic for training.
    Returns (is_valid, reason)
    """
    try:
        # Get piece placement (first part of FEN)
        pieces = fen.split()[0]
        
        # Count queens
        white_queens = pieces.count('Q')
        black_queens = pieces.count('q')
        total_queens = white_queens + black_queens
        
        # FILTER 1: Skip positions with 3+ queens
        if total_queens >= 3:
            return False, f"{total_queens}_queens"
        
        # Count total pieces
        total_pieces = sum(c.isalpha() for c in pieces)
        
        # FILTER 2: Skip positions with >32 pieces (impossible)
        if total_pieces > 32:
            return False, "too_many_pieces"
        
        return True, "valid"
        
    except Exception as e:
        return False, "parse_error"


def main():
    input_file = "data/self_play/positions.fen"
    output_file = "data/self_play/positions_filtered.fen"
    
    print("=" * 60)
    print("FILTERING POSITIONS")
    print("=" * 60)
    print(f"Input:  {input_file}")
    print(f"Output: {output_file}")
    print()
    
    # Read all positions
    print("Reading positions...")
    with open(input_file, 'r') as f:
        all_positions = [line.strip() for line in f if line.strip()]
    
    total = len(all_positions)
    print(f"Total positions: {total:,}")
    print()
    
    # Filter positions
    print("Filtering...")
    valid_positions = []
    skipped_counts = {}
    
    for fen in all_positions:
        is_valid, reason = is_valid_position(fen)
        
        if is_valid:
            valid_positions.append(fen)
        else:
            # Track skip reasons
            skipped_counts[reason] = skipped_counts.get(reason, 0) + 1
    
    # Write filtered positions
    print(f"Writing filtered positions to {output_file}...")
    with open(output_file, 'w') as f:
        for fen in valid_positions:
            f.write(fen + '\n')
    
    # Print results
    print()
    print("=" * 60)
    print("FILTERING COMPLETE")
    print("=" * 60)
    print(f"Total positions:     {total:,}")
    print(f"Valid (kept):        {len(valid_positions):,} ({100*len(valid_positions)/total:.1f}%)")
    print(f"Invalid (filtered):  {total - len(valid_positions):,} ({100*(total-len(valid_positions))/total:.1f}%)")
    print()
    
    if skipped_counts:
        print("Skip reasons:")
        for reason, count in sorted(skipped_counts.items(), key=lambda x: -x[1]):
            print(f"  {reason:20s}: {count:,} ({100*count/total:.1f}%)")
    
    print()
    print(f"Output saved to: {output_file}")
    print("=" * 60)
    print()
    print("NEXT STEP: Label with Stockfish")
    print("  python3 label_filtered_positions.py")


if __name__ == '__main__':
    main()
