#!/usr/bin/env python3
"""
Extract chess positions from compressed Lichess database using streaming decompression.
Reads .zst files directly without creating intermediate decompressed files.
"""

import zstandard as zstd
import chess.pgn
import json
import io
from pathlib import Path
from typing import Dict, Optional, List


def parse_time_control(tc_str: str) -> Optional[int]:
    """
    Parse time control string to get base time in seconds.
    Format examples: "180+0", "300+3", "600"
    Returns base time in seconds, or None if invalid.
    """
    try:
        if '+' in tc_str:
            base_time = int(tc_str.split('+')[0])
        else:
            base_time = int(tc_str)
        return base_time
    except (ValueError, AttributeError):
        return None


def extract_positions_from_game(game: chess.pgn.Game, result: str) -> List[Dict]:
    """
    Extract positions from a game according to filtering rules:
    - Skip first 10 moves
    - Extract every 3rd position after that
    """
    positions = []
    board = game.board()
    move_number = 0

    for move in game.mainline_moves():
        board.push(move)
        move_number += 1

        # Skip first 10 moves (opening theory)
        if move_number <= 10:
            continue

        # Extract every 3rd position after move 10
        if (move_number - 10) % 3 == 0:
            positions.append({
                "fen": board.fen(),
                "move_number": move_number,
                "result": result
            })

    return positions


def should_process_game(game: chess.pgn.Game) -> bool:
    """
    Check if game meets filtering criteria:
    - Both players rated 2000+
    - Time control >= 180 seconds
    """
    headers = game.headers

    # Check Elo ratings
    try:
        white_elo = int(headers.get("WhiteElo", 0))
        black_elo = int(headers.get("BlackElo", 0))

        if white_elo < 2000 or black_elo < 2000:
            return False
    except ValueError:
        return False

    # Check time control
    time_control = headers.get("TimeControl", "")
    base_time = parse_time_control(time_control)

    if base_time is None or base_time < 180:
        return False

    return True


def main():
    # File paths
    input_file = Path("data/raw/lichess_db_standard_rated_2024-01.pgn.zst")
    output_file = Path("data/processed/positions.jsonl")

    # Ensure output directory exists
    output_file.parent.mkdir(parents=True, exist_ok=True)

    # Check if input file exists
    if not input_file.exists():
        print(f"Error: Input file not found: {input_file}")
        print(f"Please ensure the Lichess database file is downloaded to: {input_file}")
        return

    # Statistics
    games_processed = 0
    games_filtered = 0
    positions_extracted = 0
    errors_encountered = 0
    target_positions = 500_000

    print(f"Starting extraction from: {input_file}")
    print(f"Target: {target_positions:,} positions")
    print(f"Output: {output_file}")
    print("-" * 60)

    # Open compressed file with streaming decompression
    with open(input_file, 'rb') as compressed_file:
        # Create decompression context
        dctx = zstd.ZstdDecompressor()

        # Create streaming reader
        with dctx.stream_reader(compressed_file) as reader:
            # Wrap in TextIOWrapper for line-by-line reading
            text_stream = io.TextIOWrapper(reader, encoding='utf-8', errors='ignore')

            # Open output file
            with open(output_file, 'w') as out_f:

                while positions_extracted < target_positions:
                    try:
                        # Parse next game from stream
                        game = chess.pgn.read_game(text_stream)

                        # Check if we've reached end of file
                        if game is None:
                            print("\nReached end of database file")
                            break

                        games_processed += 1

                        # Check if game meets criteria
                        if not should_process_game(game):
                            continue

                        games_filtered += 1

                        # Get game result
                        result = game.headers.get("Result", "*")

                        # Extract positions
                        positions = extract_positions_from_game(game, result)

                        # Write positions to output file
                        for position in positions:
                            json.dump(position, out_f)
                            out_f.write('\n')
                            positions_extracted += 1

                            if positions_extracted >= target_positions:
                                break

                        # Print progress every 1,000 games
                        if games_processed % 1000 == 0:
                            print(f"Games processed: {games_processed:,} | "
                                  f"Games filtered: {games_filtered:,} | "
                                  f"Positions: {positions_extracted:,} | "
                                  f"Errors: {errors_encountered}")

                    except Exception as e:
                        errors_encountered += 1
                        # Skip malformed games
                        if errors_encountered % 100 == 0:
                            print(f"Warning: {errors_encountered} errors encountered (continuing...)")
                        continue

    # Final statistics
    print("-" * 60)
    print("EXTRACTION COMPLETE")
    print(f"Total games processed: {games_processed:,}")
    print(f"Games meeting criteria: {games_filtered:,}")
    print(f"Positions extracted: {positions_extracted:,}")
    print(f"Errors encountered: {errors_encountered}")
    print(f"Output written to: {output_file}")

    # Calculate acceptance rate
    if games_processed > 0:
        acceptance_rate = (games_filtered / games_processed) * 100
        print(f"Acceptance rate: {acceptance_rate:.2f}%")


if __name__ == "__main__":
    main()
