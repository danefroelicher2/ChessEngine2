#!/usr/bin/env python3
"""
Filter out positions that cause Stockfish timeouts.

FILTERS OUT:
1. High complexity (>20 pieces + other issues)
2. Low-piece tactical endgames (<14 pieces) - These are super tactical and cause timeouts
3. Queen imbalance >= 2
"""

def calculate_risk(fen):
    """Calculate if position is safe to label."""
    try:
        parts = fen.split()
        pieces = parts[0]
        
        # Count pieces
        white_queens = pieces.count('Q')
        black_queens = pieces.count('q')
        white_rooks = pieces.count('R')
        black_rooks = pieces.count('r')
        
        total_pieces = sum(c.isalpha() for c in pieces)
        total_queens = white_queens + black_queens
        total_rooks = white_rooks + black_rooks
        heavy_pieces = total_queens + total_rooks
        queen_imbalance = abs(white_queens - black_queens)
        
        reasons = []
        
        # FILTER 1: Low-piece endgames (super tactical, cause timeouts)
        if total_pieces < 14:
            reasons.append("low_pieces")
        
        # FILTER 2: High complexity (old logic)
        risk_score = 0
        if total_pieces > 20:
            risk_score += 2
        if queen_imbalance >= 2:
            risk_score += 2
        if heavy_pieces >= 4:
            risk_score += 1
        
        if risk_score >= 3:
            reasons.append("high_risk")
        
        is_safe = len(reasons) == 0
        return is_safe, reasons, total_pieces
        
    except:
        return False, ["parse_error"], 0


def main():
    input_file = "data/self_play/positions_filtered.fen"
    output_file = "data/self_play/positions_safe.fen"
    
    print("=" * 70)
    print("BALANCED RISK FILTER")
    print("=" * 70)
    print(f"Input:  {input_file}")
    print(f"Output: {output_file}")
    print()
    print("Filtering out:")
    print("  - Low-piece tactical endgames (<14 pieces)")
    print("  - High-risk complex positions (score >= 3)")
    print()
    
    # Read positions
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions: {total:,}")
    print()
    
    # Filter
    print("Filtering...")
    safe_positions = []
    filtered_reasons = {}
    
    for fen in positions:
        is_safe, reasons, pieces = calculate_risk(fen)
        
        if is_safe:
            safe_positions.append(fen)
        else:
            for reason in reasons:
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
            print(f"  {reason:20s}: {count:,} ({100*count/total:.1f}%)")
    print()
    
    # Time estimate
    estimated_hours = (len(safe_positions) * 0.95) / 3600
    
    print(f"Estimated labeling time: ~{estimated_hours:.1f} hours")
    print()
    print(f"Output saved to: {output_file}")
    print("=" * 70)
    print()
    print("NEXT STEP: Label with Stockfish")
    print("  python3 label_safe_positions.py")


if __name__ == '__main__':
    main()
