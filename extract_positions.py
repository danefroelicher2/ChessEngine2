
import chess
import chess.pgn
import sys
import json
from pathlib import Path

def should_use_game(game):
    """Filter for high-quality games"""
    white_elo = game.headers.get("WhiteElo", "0")
    black_elo = game.headers.get("BlackElo", "0")
    time_control = game.headers.get("TimeControl", "")
    
    try:
        white_elo = int(white_elo)
        black_elo = int(black_elo)
    except:
        return False
    
    # Both players rated 2200+, time control >= 180 seconds
    if white_elo < 2200 or black_elo < 2200:
        return False
    
    try:
        base_time = int(time_control.split('+')[0])
        if base_time < 180:
            return False
    except:
        return False
    
    return True

def extract_positions_from_game(game, positions_per_game=10):
    """Extract positions from a single game"""
    positions = []
    board = game.board()
    move_count = 0
    
    for move in game.mainline_moves():
        board.push(move)
        move_count += 1
        
        if move_count < 10:
            continue
        
        if move_count % 3 == 0:
            fen = board.fen()
            positions.append({
                'fen': fen,
                'move_number': move_count,
                'result': game.headers.get("Result", "*")
            })
        
        if len(positions) >= positions_per_game:
            break
    
    return positions

def main():
    pgn_file = sys.argv[1] if len(sys.argv) > 1 else "data/raw/lichess_db_standard_rated_2024-01.pgn"
    output_file = "data/processed/positions.jsonl"
    
    Path("data/processed").mkdir(parents=True, exist_ok=True)
    
    print(f"Reading games from: {pgn_file}")
    print(f"Output file: {output_file}")
    print("=" * 60)
    
    games_processed = 0
    games_used = 0
    positions_extracted = 0
    
    with open(pgn_file) as pgn:
        with open(output_file, 'w') as out:
            while True:
                game = chess.pgn.read_game(pgn)
                if game is None:
                    break
                
                games_processed += 1
                
                if games_processed % 1000 == 0:
                    print(f"Processed: {games_processed:,} games | "
                          f"Used: {games_used:,} games | "
                          f"Positions: {positions_extracted:,}")
                
                if not should_use_game(game):
                    continue
                
                games_used += 1
                positions = extract_positions_from_game(game)
                
                for pos in positions:
                    out.write(json.dumps(pos) + '\n')
                    positions_extracted += 1
                
                if positions_extracted >= 1_000_000:
                    print(f"\nReached target of 1M positions!")
                    break
    
    print("=" * 60)
    print(f"Final stats:")
    print(f"  Games processed: {games_processed:,}")
    print(f"  Games used: {games_used:,}")
    print(f"  Positions extracted: {positions_extracted:,}")
    print(f"  Output: {output_file}")

if __name__ == "__main__":
    main()
