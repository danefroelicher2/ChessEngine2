import chess
import chess.pgn
import zstandard as zstd
import json
from pathlib import Path
import io

def should_use_game(game):
    """Filter: 2200+ Elo, 4+ minute games, finished games only"""
    white_elo = game.headers.get("WhiteElo", "0")
    black_elo = game.headers.get("BlackElo", "0")
    time_control = game.headers.get("TimeControl", "")
    result = game.headers.get("Result", "*")
    
    # Skip unfinished games
    if result == "*":
        return False
    
    try:
        white_elo = int(white_elo)
        black_elo = int(black_elo)
    except:
        return False
    
    # Both players 2200+ Elo
    if white_elo < 2200 or black_elo < 2200:
        return False
    
    # 240+ seconds (4+ minutes)
    try:
        base_time = int(time_control.split('+')[0])
        if base_time < 240:
            return False
    except:
        return False
    
    return True

def extract_positions_from_game(game, positions_per_game=15):
    """Extract positions from a single game"""
    positions = []
    board = game.board()
    move_count = 0
    
    for move in game.mainline_moves():
        board.push(move)
        move_count += 1
        
        # Skip early opening (first 15 moves)
        if move_count < 15:
            continue
        
        # Sample every 5 moves for diversity
        if move_count % 5 == 0:
            fen = board.fen()
            positions.append({
                'fen': fen,
                'move_number': move_count,
                'result': game.headers.get("Result", "*")
            })
        
        if len(positions) >= positions_per_game:
            break
    
    return positions

def process_month(input_file, output_file, target_remaining, positions_so_far):
    """Process a single month's data"""
    print(f"\nProcessing: {input_file}")
    print(f"Positions so far: {positions_so_far:,}")
    print(f"Target remaining: {target_remaining:,}")
    print("=" * 60)
    
    games_processed = 0
    games_used = 0
    positions_extracted = 0
    
    with open(input_file, 'rb') as compressed:
        dctx = zstd.ZstdDecompressor()
        with dctx.stream_reader(compressed) as stream_reader:
            text_stream = io.TextIOWrapper(stream_reader, encoding='utf-8')
            
            with open(output_file, 'a') as out:  # APPEND mode
                while positions_extracted < target_remaining:
                    game = chess.pgn.read_game(text_stream)
                    
                    if game is None:
                        print("\nReached end of file")
                        break
                    
                    games_processed += 1
                    
                    if games_processed % 1000 == 0:
                        print(f"Processed: {games_processed:,} games | "
                              f"Used: {games_used:,} | "
                              f"Positions: {positions_extracted:,}")
                    
                    if not should_use_game(game):
                        continue
                    
                    games_used += 1
                    positions = extract_positions_from_game(game)
                    
                    for pos in positions:
                        out.write(json.dumps(pos) + '\n')
                        positions_extracted += 1
                        
                        if positions_extracted >= target_remaining:
                            break
    
    print(f"Month complete: Extracted {positions_extracted:,} positions")
    return positions_extracted

def main():
    output_file = "data/processed/positions.jsonl"
    target_positions = 2_500_000
    
    # Delete existing file to start fresh
    Path(output_file).unlink(missing_ok=True)
    
    months = [
        "data/raw/lichess_db_standard_rated_2024-01.pgn.zst",
        "data/raw/lichess_db_standard_rated_2024-02.pgn.zst",
        "data/raw/lichess_db_standard_rated_2024-03.pgn.zst"
    ]
    
    print("=" * 60)
    print("MULTI-MONTH POSITION EXTRACTION")
    print("=" * 60)
    print(f"Target: {target_positions:,} positions")
    print(f"Filter: 2200+ Elo, 240+ second games")
    print(f"Output: {output_file}")
    print("=" * 60)
    
    total_extracted = 0
    
    for month_file in months:
        remaining = target_positions - total_extracted
        if remaining <= 0:
            print(f"\n✓ Target reached! Skipping remaining files.")
            break
        
        extracted = process_month(month_file, output_file, remaining, total_extracted)
        total_extracted += extracted
        
        if total_extracted >= target_positions:
            print(f"\n✓ Target reached: {total_extracted:,} positions")
            break
    
    print("=" * 60)
    print(f"EXTRACTION COMPLETE")
    print(f"Total positions: {total_extracted:,}")
    print(f"Output: {output_file}")
    print("=" * 60)

if __name__ == "__main__":
    main()
