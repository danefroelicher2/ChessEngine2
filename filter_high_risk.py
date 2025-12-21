#!/usr/bin/env python3
"""
Filter out high-risk positions that are likely to timeout in Stockfish.

RISK SCORE CALCULATION:
- +2 points if >20 pieces (high complexity)
- +2 points if queen imbalance >= 2 (one side promoted multiple queens)
- +1 point if 4+ heavy pieces (queens + rooks)

Positions with risk_score >= 3 are filtered out.
"""

def calculate_risk_score(fen):
    """Calculate risk score for a position."""
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
        
        # Calculate risk score
        risk_score = 0
        
        if total_pieces > 20:
            risk_score += 2
        
        if queen_imbalance >= 2:
            risk_score += 2
        
        if heavy_pieces >= 4:
            risk_score += 1
        
        return risk_score, total_pieces, total_queens, heavy_pieces
        
    except:
        return 999, 0, 0, 0  # Invalid positions get max risk


def main():
    input_file = "data/self_play/positions_filtered.fen"
    output_file = "data/self_play/positions_low_risk.fen"
    
    print("=" * 70)
    print("HIGH-RISK POSITION FILTER")
    print("=" * 70)
    print(f"Input:  {input_file}")
    print(f"Output: {output_file}")
    print()
    print("Filtering out positions with risk_score >= 3")
    print()
    
    # Read positions
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions: {total:,}")
    print()
    
    # Filter by risk score
    print("Calculating risk scores...")
    low_risk = []
    high_risk = []
    risk_distribution = {}
    
    for fen in positions:
        risk_score, pieces, queens, heavy = calculate_risk_score(fen)
        
        risk_distribution[risk_score] = risk_distribution.get(risk_score, 0) + 1
        
        if risk_score < 3:
            low_risk.append(fen)
        else:
            high_risk.append(fen)
    
    # Write low-risk positions
    print(f"Writing low-risk positions to {output_file}...")
    with open(output_file, 'w') as f:
        for fen in low_risk:
            f.write(fen + '\n')
    
    # Print results
    print()
    print("=" * 70)
    print("FILTERING COMPLETE")
    print("=" * 70)
    print(f"Total positions:       {total:,}")
    print(f"Low risk (kept):       {len(low_risk):,} ({100*len(low_risk)/total:.1f}%)")
    print(f"High risk (filtered):  {len(high_risk):,} ({100*len(high_risk)/total:.1f}%)")
    print()
    
    print("Risk score distribution:")
    for score in sorted(risk_distribution.keys()):
        count = risk_distribution[score]
        pct = 100 * count / total
        status = "✓ KEPT" if score < 3 else "✗ FILTERED"
        print(f"  Score {score}: {count:6,} ({pct:5.1f}%) {status}")
    print()
    
    # Time estimate
    # Last run: 7,503 positions in 2 hours = 0.95 sec/position
    estimated_hours = (len(low_risk) * 0.95) / 3600
    
    print(f"Estimated labeling time: ~{estimated_hours:.1f} hours")
    print()
    print(f"Output saved to: {output_file}")
    print("=" * 70)
    print()
    print("NEXT STEP: Label with Stockfish")
    print("  python3 label_low_risk_positions.py")


if __name__ == '__main__':
    main()
