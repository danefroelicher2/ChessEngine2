#!/usr/bin/env python3
"""
Label filtered chess positions with Stockfish evaluations.

INPUT:  data/self_play/positions_filtered.fen (realistic positions only)
OUTPUT: data/self_play/positions_labeled.jsonl (positions + evaluations)

STOCKFISH SETTINGS:
- Depth: 16 (same as last successful run)
- Timeout: 40 seconds per position
- Takes LAST evaluation before "bestmove" (most accurate)
"""

import subprocess
import json
import time
from pathlib import Path

# Configuration
STOCKFISH_PATH = "/usr/games/stockfish"
INPUT_FILE = "data/self_play/positions_filtered.fen"
OUTPUT_FILE = "data/self_play/positions_labeled.jsonl"
DEPTH = 16
TIMEOUT = 40


def get_stockfish_eval(fen, depth=16, timeout=40):
    """
    Get Stockfish evaluation for a FEN position.
    
    Returns:
        float: Evaluation in pawns (positive = white better)
        None: If timeout or error
    """
    try:
        # Start Stockfish
        process = subprocess.Popen(
            [STOCKFISH_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )

        # Initialize UCI
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

        # Read lines until "bestmove"
        # Keep track of LAST evaluation (most accurate)
        last_eval = None
        start_time = time.time()

        while True:
            if time.time() - start_time > timeout:
                process.kill()
                return None

            line = process.stdout.readline().strip()

            # Parse centipawn scores
            if 'score cp' in line:
                try:
                    parts = line.split('score cp')
                    score_str = parts[1].strip().split()[0]
                    centipawns = int(score_str)
                    last_eval = centipawns / 100.0  # Convert to pawns
                except:
                    pass

            # Parse mate scores
            elif 'score mate' in line:
                try:
                    parts = line.split('score mate')
                    mate_in = int(parts[1].strip().split()[0])
                    last_eval = 100.0 if mate_in > 0 else -100.0
                except:
                    pass
            
            # Stop when analysis complete
            if 'bestmove' in line:
                break
        
        # Clean shutdown
        process.stdin.write("quit\n")
        process.stdin.flush()
        process.wait(timeout=5)
        
        return last_eval
        
    except Exception as e:
        try:
            process.kill()
        except:
            pass
        return None


def main():
    input_path = Path(INPUT_FILE)
    output_path = Path(OUTPUT_FILE)
    
    print("=" * 70)
    print("STOCKFISH LABELING")
    print("=" * 70)
    print(f"Input:     {input_path}")
    print(f"Output:    {output_path}")
    print(f"Stockfish: {STOCKFISH_PATH}")
    print(f"Depth:     {DEPTH}")
    print(f"Timeout:   {TIMEOUT}s per position")
    print()
    
    # Verify input exists
    if not input_path.exists():
        print(f"ERROR: Input file not found: {input_path}")
        print("Run filter_positions.py first!")
        return
    
    # Read positions
    print("Reading filtered positions...")
    with open(input_path, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions to label: {total:,}")
    print()
    
    # Estimate time (based on last run: 0.95 sec per position)
    estimated_hours = (total * 0.95) / 3600
    print(f"Estimated time: ~{estimated_hours:.1f} hours")
    print()
    
    print("Starting labeling...")
    print("Press Ctrl+C to cancel")
    print()
    
    # Start labeling
    labeled = 0
    failed = 0
    start_time = time.time()
    
    with open(output_path, 'w') as out:
        for i, fen in enumerate(positions, 1):
            # Get evaluation
            eval_score = get_stockfish_eval(fen, depth=DEPTH, timeout=TIMEOUT)
            
            if eval_score is None:
                failed += 1
                print(f"  FAIL #{failed}: Timeout - {fen[:50]}...")
                continue
            
            # Save labeled position
            labeled += 1
            data = {
                'fen': fen,
                'evaluation': eval_score
            }
            out.write(json.dumps(data) + '\n')
            out.flush()
            
            # Progress every 100 positions
            if i % 100 == 0:
                elapsed = time.time() - start_time
                rate = labeled / elapsed if elapsed > 0 else 0
                remaining = total - i
                eta_seconds = remaining / rate if rate > 0 else 0
                eta_hours = eta_seconds / 3600
                
                print(f"Progress: {i}/{total} ({100*i/total:.1f}%)")
                print(f"  Labeled: {labeled:,} | Failed: {failed:,}")
                print(f"  Rate:    {rate:.2f} pos/sec | Last eval: {eval_score:+.2f}")
                print(f"  ETA:     {eta_hours:.1f} hours")
                print()
    
    # Final stats
    total_time = time.time() - start_time
    
    print()
    print("=" * 70)
    print("✅ LABELING COMPLETE")
    print("=" * 70)
    print(f"Total positions:      {total:,}")
    print(f"Successfully labeled: {labeled:,} ({100*labeled/total:.1f}%)")
    print(f"Failed (timeout):     {failed:,} ({100*failed/total:.1f}%)")
    print()
    print(f"Total time: {total_time/3600:.2f} hours")
    print(f"Average:    {total_time/labeled:.2f} sec/position")
    print()
    print(f"Output: {output_path}")
    print("=" * 70)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user. Partial results saved.")
    except Exception as e:
        print(f"\n\nERROR: {e}")
        raise
