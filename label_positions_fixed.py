#!/usr/bin/env python3
"""
Label chess FEN positions with Stockfish evaluations.
FINAL VERSION - Ready for overnight run on all 13,082 positions.

Key features:
- Pre-filters problematic positions (3+ queens)
- Properly waits for Stockfish depth 16 analysis
- Takes LAST evaluation before "bestmove"
- Handles timeouts gracefully
- Saves ~1 hour by skipping unrealistic positions
"""

import subprocess
import json
import time
import os
from pathlib import Path

STOCKFISH_PATH = "/usr/games/stockfish"
INPUT_FILE = "data/self_play/positions_fixed.fen"
OUTPUT_FILE = "data/self_play/positions_with_evals_FINAL.jsonl"
DEPTH = 16
PROGRESS_INTERVAL = 100  # Show progress every 100 positions


def is_valid_position(fen):
    """
    Quick validation to skip problematic positions.
    Returns False for positions that will likely timeout.
    
    Filters out:
    - Positions with 3+ queens (unrealistic, causes 40-second timeouts)
    - Positions with >32 total pieces (invalid)
    """
    try:
        # Extract piece placement (first part of FEN)
        pieces = fen.split()[0]
        
        # Count queens
        white_queens = pieces.count('Q')
        black_queens = pieces.count('q')
        total_queens = white_queens + black_queens
        
        # Skip positions with 3+ queens (unrealistic, causes timeouts)
        if total_queens >= 3:
            return False
        
        # Count total pieces (sanity check)
        total_pieces = sum(c.isalpha() for c in pieces)
        if total_pieces > 32:  # Can't have more than 32 pieces
            return False
            
        return True
        
    except:
        return False  # Invalid FEN format


def get_stockfish_eval(fen, depth=16, timeout=30):
    """
    Get Stockfish evaluation for a FEN position.

    Args:
        fen: FEN string of the position
        depth: Search depth (default 16)
        timeout: Max time to wait in seconds (default 30)

    Returns:
        float: Evaluation in pawns (positive = white better, negative = black better)
               Returns None if evaluation fails or times out
    """
    try:
        # Start Stockfish process
        process = subprocess.Popen(
            [STOCKFISH_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )

        # Send UCI commands
        process.stdin.write("uci\n")
        process.stdin.flush()

        # Wait for uciok
        start_time = time.time()
        while True:
            if time.time() - start_time > timeout:
                process.kill()
                return None

            line = process.stdout.readline().strip()
            if line == "uciok":
                break

        # Set position and analyze
        process.stdin.write(f"position fen {fen}\n")
        process.stdin.write(f"go depth {depth}\n")
        process.stdin.flush()

        # Read lines until we see "bestmove"
        # Keep track of the LAST evaluation seen
        last_eval = None
        start_time = time.time()

        while True:
            if time.time() - start_time > timeout:
                process.kill()
                return None

            line = process.stdout.readline().strip()

            # Check if this line contains a score
            if "score cp" in line:
                # Parse centipawn score
                parts = line.split()
                try:
                    cp_index = parts.index("cp") + 1
                    centipawns = int(parts[cp_index])
                    # Convert centipawns to pawns
                    last_eval = centipawns / 100.0
                except (ValueError, IndexError):
                    pass

            elif "score mate" in line:
                # Parse mate score
                parts = line.split()
                try:
                    mate_index = parts.index("mate") + 1
                    mate_in = int(parts[mate_index])
                    # Mate scores: use large values
                    if mate_in > 0:
                        last_eval = 100.0 + (10.0 * (10 - abs(mate_in)))
                    else:
                        last_eval = -100.0 - (10.0 * (10 - abs(mate_in)))
                except (ValueError, IndexError):
                    pass

            # Check if we've reached the bestmove
            if line.startswith("bestmove"):
                break

        # Clean up
        process.stdin.write("quit\n")
        process.stdin.flush()
        process.wait(timeout=2)

        return last_eval

    except Exception as e:
        print(f"Error evaluating position: {e}")
        try:
            process.kill()
        except:
            pass
        return None


def main():
    print(f"Stockfish Chess Position Labeler - FINAL VERSION")
    print(f"=" * 60)
    print(f"Stockfish path: {STOCKFISH_PATH}")
    print(f"Input file: {INPUT_FILE}")
    print(f"Output file: {OUTPUT_FILE}")
    print(f"Depth: {DEPTH}")
    print(f"=" * 60)

    # Check if Stockfish exists
    if not os.path.exists(STOCKFISH_PATH):
        print(f"ERROR: Stockfish not found at {STOCKFISH_PATH}")
        return

    # Read input positions
    if not os.path.exists(INPUT_FILE):
        print(f"ERROR: Input file not found: {INPUT_FILE}")
        return

    print(f"\nReading positions from {INPUT_FILE}...")
    with open(INPUT_FILE, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]



    total_positions = len(positions)
    print(f"Loaded {total_positions} positions")
    print(f"Estimated time: ~{total_positions * 4.5 / 3600:.1f} hours")

    # Create output directory if needed
    Path(OUTPUT_FILE).parent.mkdir(parents=True, exist_ok=True)

    # Process positions
    print(f"\nProcessing positions (depth {DEPTH})...")
    print(f"Progress updates every {PROGRESS_INTERVAL} positions\n")

    start_time = time.time()
    processed = 0
    skipped = 0
    failed = 0

    with open(OUTPUT_FILE, 'w') as out_f:
        for i, fen in enumerate(positions, 1):
            # Skip obviously problematic positions (saves ~40 seconds each)
            if not is_valid_position(fen):
                skipped += 1
                if skipped <= 10:  # Only print first 10 skips
                    print(f"SKIPPED position {i}: Unrealistic (3+ queens) - {fen[:50]}...")
                continue
            
            # Get evaluation
            eval_score = get_stockfish_eval(fen, depth=DEPTH, timeout=30)

            if eval_score is not None:
                # Write to output file
                data = {
                    "fen": fen,
                    "evaluation": eval_score
                }
                out_f.write(json.dumps(data) + '\n')
                out_f.flush()
                processed += 1

                # Show progress
                if i % PROGRESS_INTERVAL == 0:
                    elapsed = time.time() - start_time
                    rate = processed / elapsed
                    remaining_positions = total_positions - i
                    eta_seconds = remaining_positions / rate if rate > 0 else 0
                    print(f"Progress: {i}/{total_positions} ({100*i/total_positions:.1f}%) | "
                          f"Processed: {processed} | Skipped: {skipped} | Failed: {failed} | "
                          f"Last eval: {eval_score:+.2f} | "
                          f"Rate: {rate:.2f} pos/sec | "
                          f"ETA: {eta_seconds/3600:.1f} hours")
            else:
                failed += 1
                if failed <= 10:  # Only print first 10 failures
                    print(f"WARNING: Failed to evaluate position {i}: {fen[:50]}...")

    # Final summary
    elapsed = time.time() - start_time
    print(f"\n{'=' * 60}")
    print(f"✅ COMPLETED!")
    print(f"Total positions in file: {total_positions}")
    print(f"Successfully processed: {processed}")
    print(f"Skipped (unrealistic): {skipped}")
    print(f"Failed (timeout): {failed}")
    print(f"Success rate: {100*processed/total_positions:.1f}%")
    print(f"Total time: {elapsed/3600:.2f} hours")
    print(f"Average time per processed position: {elapsed/processed:.2f} seconds")
    print(f"Output saved to: {OUTPUT_FILE}")
print("=" * 60)

if __name__ == "__main__":
    main()