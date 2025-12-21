#!/usr/bin/env python3
"""
Analyze filtered positions to identify patterns that might cause Stockfish timeouts.

We want to find characteristics that predict failures BEFORE we waste 40 seconds on them.
"""

def analyze_position(fen):
    """
    Extract characteristics from a FEN position that might cause timeouts.
    """
    try:
        parts = fen.split()
        pieces = parts[0]
        
        # Count pieces by type
        white_pawns = pieces.count('P')
        black_pawns = pieces.count('p')
        white_knights = pieces.count('N')
        black_knights = pieces.count('n')
        white_bishops = pieces.count('B')
        black_bishops = pieces.count('b')
        white_rooks = pieces.count('R')
        black_rooks = pieces.count('r')
        white_queens = pieces.count('Q')
        black_queens = pieces.count('q')
        
        total_pieces = sum(c.isalpha() for c in pieces)
        total_queens = white_queens + black_queens
        total_rooks = white_rooks + black_rooks
        total_minor = white_knights + black_knights + white_bishops + black_bishops
        total_pawns = white_pawns + black_pawns
        
        # Calculate "complexity" metrics
        # More pieces = more possible moves = harder for Stockfish
        piece_count = total_pieces
        
        # Queens + Rooks = "heavy pieces" (more tactical complexity)
        heavy_pieces = total_queens + total_rooks
        
        # Low material but not endgame (weird positions)
        is_low_material = total_pieces < 10
        
        # Very high material (pawn promotions)
        is_high_material = total_pieces > 20
        
        # Imbalanced queens (one side promoted multiple queens)
        queen_imbalance = abs(white_queens - black_queens)
        has_queen_imbalance = queen_imbalance >= 2
        
        # Too many of one piece type (unusual)
        max_rooks = max(white_rooks, black_rooks)
        max_bishops = max(white_bishops, black_bishops)
        max_knights = max(white_knights, black_knights)
        
        unusual_pieces = (max_rooks >= 3 or max_bishops >= 3 or max_knights >= 3)
        
        return {
            'fen': fen,
            'total_pieces': piece_count,
            'total_queens': total_queens,
            'total_rooks': total_rooks,
            'total_minor': total_minor,
            'total_pawns': total_pawns,
            'heavy_pieces': heavy_pieces,
            'is_low_material': is_low_material,
            'is_high_material': is_high_material,
            'queen_imbalance': queen_imbalance,
            'has_queen_imbalance': has_queen_imbalance,
            'unusual_pieces': unusual_pieces,
            'white_queens': white_queens,
            'black_queens': black_queens,
        }
    except:
        return None


def main():
    input_file = "data/self_play/positions_filtered.fen"
    
    print("=" * 70)
    print("POSITION ANALYSIS")
    print("=" * 70)
    print(f"Analyzing: {input_file}")
    print()
    
    # Read positions
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    print(f"Total positions: {len(positions):,}")
    print()
    print("Analyzing characteristics...")
    print()
    
    # Analyze all positions
    analyses = []
    for fen in positions:
        analysis = analyze_position(fen)
        if analysis:
            analyses.append(analysis)
    
    # Statistics
    total = len(analyses)
    
    # Piece count distribution
    piece_counts = {}
    for a in analyses:
        count = a['total_pieces']
        piece_counts[count] = piece_counts.get(count, 0) + 1
    
    print("PIECE COUNT DISTRIBUTION:")
    for count in sorted(piece_counts.keys()):
        pct = 100 * piece_counts[count] / total
        bar = '#' * int(pct / 2)
        print(f"  {count:2d} pieces: {piece_counts[count]:6,} ({pct:5.1f}%) {bar}")
    print()
    
    # Queens distribution
    queen_counts = {}
    for a in analyses:
        count = a['total_queens']
        queen_counts[count] = queen_counts.get(count, 0) + 1
    
    print("QUEEN COUNT DISTRIBUTION:")
    for count in sorted(queen_counts.keys()):
        pct = 100 * queen_counts[count] / total
        print(f"  {count} queens: {queen_counts[count]:6,} ({pct:5.1f}%)")
    print()
    
    # Risky characteristics
    high_material = sum(1 for a in analyses if a['is_high_material'])
    low_material = sum(1 for a in analyses if a['is_low_material'])
    queen_imbalance = sum(1 for a in analyses if a['has_queen_imbalance'])
    unusual = sum(1 for a in analyses if a['unusual_pieces'])
    heavy_piece_count = [a['heavy_pieces'] for a in analyses]
    
    print("POTENTIAL TIMEOUT RISKS:")
    print(f"  High material (>20 pieces):    {high_material:6,} ({100*high_material/total:5.1f}%)")
    print(f"  Low material (<10 pieces):     {low_material:6,} ({100*low_material/total:5.1f}%)")
    print(f"  Queen imbalance (diff >= 2):   {queen_imbalance:6,} ({100*queen_imbalance/total:5.1f}%)")
    print(f"  Unusual pieces (3+ R/B/N):     {unusual:6,} ({100*unusual/total:5.1f}%)")
    print()
    
    # Heavy pieces (queens + rooks)
    heavy_dist = {}
    for a in analyses:
        count = a['heavy_pieces']
        heavy_dist[count] = heavy_dist.get(count, 0) + 1
    
    print("HEAVY PIECES (Queens + Rooks):")
    for count in sorted(heavy_dist.keys()):
        pct = 100 * heavy_dist[count] / total
        print(f"  {count} heavy: {heavy_dist[count]:6,} ({pct:5.1f}%)")
    print()
    
    # Find extreme positions
    print("EXTREME POSITIONS (most likely to timeout):")
    print()
    
    # Sort by "risk score" - multiple risky factors
    for a in analyses:
        risk_score = 0
        if a['is_high_material']: risk_score += 2
        if a['has_queen_imbalance']: risk_score += 2
        if a['unusual_pieces']: risk_score += 1
        if a['heavy_pieces'] >= 4: risk_score += 1
        a['risk_score'] = risk_score
    
    # Show highest risk positions
    high_risk = sorted([a for a in analyses if a['risk_score'] >= 3], 
                       key=lambda x: -x['risk_score'])
    
    print(f"High risk positions (score >= 3): {len(high_risk):,} ({100*len(high_risk)/total:.1f}%)")
    if high_risk:
        print("\nTop 10 highest risk:")
        for i, a in enumerate(high_risk[:10], 1):
            print(f"  {i}. Risk={a['risk_score']} | Pieces={a['total_pieces']} | "
                  f"Queens={a['total_queens']} | Heavy={a['heavy_pieces']}")
            print(f"     {a['fen'][:60]}...")
        print()
    
    print("=" * 70)
    print("RECOMMENDATIONS:")
    print("=" * 70)
    
    if high_material > total * 0.05:
        print(f"⚠️  {100*high_material/total:.1f}% have >20 pieces (high complexity)")
        print("   Consider filtering: total_pieces > 22")
        print()
    
    if queen_imbalance > total * 0.05:
        print(f"⚠️  {100*queen_imbalance/total:.1f}% have large queen imbalance")
        print("   Consider filtering: abs(white_queens - black_queens) >= 2")
        print()
    
    if unusual > total * 0.05:
        print(f"⚠️  {100*unusual/total:.1f}% have unusual piece counts (3+ of a type)")
        print("   Consider filtering: max(rooks/bishops/knights) >= 3")
        print()
    
    if len(high_risk) > total * 0.05:
        print(f"⚠️  {100*len(high_risk)/total:.1f}% are high risk (multiple factors)")
        print("   Consider filtering positions with risk_score >= 3")
        print()
    
    print("=" * 70)


if __name__ == '__main__':
    main()
