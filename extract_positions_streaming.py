import chess
import chess.pgn
import zstandard as zstd
import json
from pathlib import Path
import io

def should_use_game(game):
    """Filter: 2050+ Elo, 3+ minute games"""
    white_elo = game.headers.get("WhiteElo", "0")
    black_elo = game.headers.get("BlackElo", "0")
    time_control = game.headers.get("TimeControl", "")
    
    try:
        white_elo = int(white_elo)
        black_elo = int(black_elo)
    except:
        return False
    
    # Both players 2050+ Elo
    if white_elo < 2050 or black_elo < 2050:
        return False
    
    # 180+ seconds (3+ minutes)
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
    input_file = "data/raw/lichess_db_standard_rated_2024-01.pgn.zst"
    output_file = "data/processed/positions.jsonl"
    target_positions = 300_000  # 300K positions
    
    print(f"Reading from: {input_file}")
    print(f"Output: {output_file}")
    print(f"Target: {target_positions:,} positions")
    print(f"Filter: 2050+ Elo, 3+ minute games")
    print("=" * 60)
    
    games_processed = 0
    games_used = 0
    positions_extracted = 0
    
    with open(input_file, 'rb') as compressed:
        dctx = zstd.ZstdDecompressor()
        with dctx.stream_reader(compressed) as stream_reader:
            text_stream = io.TextIOWrapper(stream_reader, encoding='utf-8')
            
            with open(output_file, 'w') as out:
                while positions_extracted < target_positions:
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
                        
                        if positions_extracted >= target_positions:
                            break
    
    print("=" * 60)
    print(f"Final stats:")
    print(f"  Games processed: {games_processed:,}")
    print(f"  Games used: {games_used:,}")
    print(f"  Positions extracted: {positions_extracted:,}")

if __name__ == "__main__":
    main()
