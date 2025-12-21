#!/usr/bin/env python3
"""
ADJUSTED FILTER - Cut out 27-28 pieces (too complex)

Based on labeling results:
- 16-26 pieces: 6.5% fail rate (excellent)
- 27-28 pieces: 35%+ fail rate (unacceptable)

Cutting to 16-26 pieces only.
"""

def is_safe_position(fen):
    try:
        parts = fen.split()
        board_part = parts[0]
        ranks = board_part.split('/')
        
        total_pieces = sum(c.isalpha() for c in board_part)
        
        # ADJUSTED: 16-26 pieces only (cut out 27-28)
        if total_pieces < 16 or total_pieces > 26:
            return False, "out_of_range", total_pieces
        
        # King safety check
        rank_1 = ranks[7]
        rank_2 = ranks[6]
        rank_7 = ranks[1]
        rank_8 = ranks[0]
        
        white_king_safe = ('K' in rank_1) or ('K' in rank_2)
        black_king_safe = ('k' in rank_8) or ('k' in rank_7)
        
        if not white_king_safe or not black_king_safe:
            return False, "exposed_king", total_pieces
        
        return True, "safe", total_pieces
        
    except:
        return False, "parse_error", 0


def main():
    input_file = "data/self_play/positions_filtered.fen"
    output_file = "data/self_play/positions_final_adjusted.fen"
    
    print("=" * 70)
    print("ADJUSTED FILTER - 16-26 PIECES ONLY")
    print("=" * 70)
    print("Cutting out 27-28 piece positions (causing 35%+ fail rate)")
    print()
    
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions: {total:,}")
    print()
    
    safe_positions = []
    piece_dist = {}
    
    for fen in positions:
        is_safe, reason, pieces = is_safe_position(fen)
        
        if is_safe:
            safe_positions.append(fen)
            piece_dist[pieces] = piece_dist.get(pieces, 0) + 1
    
    with open(output_file, 'w') as f:
        for fen in safe_positions:
            f.write(fen + '\n')
    
    print("=" * 70)
    print("RESULTS")
    print("=" * 70)
    print(f"Kept:     {len(safe_positions):,} ({100*len(safe_positions)/total:.1f}%)")
    print(f"Filtered: {total - len(safe_positions):,}")
    print()
    
    print("Piece distribution:")
    for p in sorted(piece_dist.keys()):
        print(f"  {p} pieces: {piece_dist[p]:,}")
    print()
    
    # Time estimate with 6.5% fail rate
    expected_labeled = int(len(safe_positions) * 0.935)
    hours = (len(safe_positions) * 0.95) / 3600
    
    print(f"Expected labeled: ~{expected_labeled:,} (6.5% fail rate)")
    print(f"Estimated time: ~{hours:.1f} hours")
    print(f"Output: {output_file}")
    print("=" * 70)


if __name__ == '__main__':
    main()
