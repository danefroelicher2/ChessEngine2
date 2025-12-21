#!/usr/bin/env python3
"""
Label comprehensively filtered positions with Stockfish.

These positions have been carefully filtered to maximize training diversity
while minimizing timeout risk:
- 16-28 pieces (endgame through opening)
- Kings on safe ranks (1-2 for white, 7-8 for black)

Expecting ~8% timeout rate, which is acceptable for the volume of data.
"""

import subprocess
import json
import time
from pathlib import Path

STOCKFISH_PATH = "/usr/games/stockfish"
INPUT_FILE = "data/self_play/positions_ready.fen"
OUTPUT_FILE = "data/self_play/positions_labeled.jsonl"
DEPTH = 16
TIMEOUT = 40


def get_stockfish_eval(fen, depth=16, timeout=40):
    try:
        process = subprocess.Popen(
            [STOCKFISH_PATH],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )

        process.stdin.write("uci\n")
        process.stdin.flush()

        start_time = time.time()
        while True:
            if time.time() - start_time > timeout:
                process.kill()
                return None

            line = process.stdout.readline().strip()
            if line == "uciok":
                break

        process.stdin.write(f"position fen {fen}\n")
        process.stdin.write(f"go depth {depth}\n")
        process.stdin.flush()

        last_eval = None
        start_time = time.time()

        while True:
            if time.time() - start_time > timeout:
                process.kill()
                return None

            line = process.stdout.readline().strip()

            if 'score cp' in line:
                try:
                    parts = line.split('score cp')
                    score_str = parts[1].strip().split()[0]
                    centipawns = int(score_str)
                    last_eval = centipawns / 100.0
                except:
                    pass

            elif 'score mate' in line:
                try:
                    parts = line.split('score mate')
                    mate_in = int(parts[1].strip().split()[0])
                    last_eval = 100.0 if mate_in > 0 else -100.0
                except:
                    pass
            
            if 'bestmove' in line:
                break
        
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
    print("STOCKFISH LABELING - FINAL RUN")
    print("=" * 70)
    print(f"Input:     {input_path}")
    print(f"Output:    {output_path}")
    print(f"Depth:     {DEPTH}")
    print(f"Timeout:   {TIMEOUT}s")
    print()
    
    if not input_path.exists():
        print(f"ERROR: Input file not found: {input_path}")
        print("Run filter_final_comprehensive.py first!")
        return
    
    with open(input_path, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions to label: {total:,}")
    print()
    
    estimated_hours = (total * 0.95) / 3600
    print(f"Estimated time: ~{estimated_hours:.1f} hours")
    print()
    print("Expecting ~8% timeout rate (acceptable for volume)")
    print("Starting labeling...")
    print()
    
    labeled = 0
    failed = 0
    start_time = time.time()
    
    with open(output_path, 'w') as out:
        for i, fen in enumerate(positions, 1):
            eval_score = get_stockfish_eval(fen, depth=DEPTH, timeout=TIMEOUT)
            
            if eval_score is None:
                failed += 1
                if failed <= 20:  # Only show first 20 failures
                    print(f"  FAIL #{failed}: {fen[:50]}...")
                elif failed % 100 == 0:  # Then show every 100th
                    print(f"  FAIL #{failed}")
                continue
            
            labeled += 1
            data = {
                'fen': fen,
                'evaluation': eval_score
            }
            out.write(json.dumps(data) + '\n')
            out.flush()
            
            if i % 100 == 0:
                elapsed = time.time() - start_time
                rate = labeled / elapsed if elapsed > 0 else 0
                remaining = total - i
                eta_seconds = remaining / rate if rate > 0 else 0
                eta_hours = eta_seconds / 3600
                fail_rate = 100 * failed / i
                
                print(f"Progress: {i}/{total} ({100*i/total:.1f}%)")
                print(f"  Labeled: {labeled:,} | Failed: {failed:,} | Fail rate: {fail_rate:.1f}%")
                print(f"  Rate:    {rate:.2f} pos/sec | Last eval: {eval_score:+.2f}")
                print(f"  ETA:     {eta_hours:.1f} hours")
                print()
    
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
    print()
    print("NEXT STEPS:")
    print("1. Combine with previous data:")
    print("   cat data/self_play/positions_with_evals_FINAL.jsonl \\")
    print("       data/self_play/positions_labeled.jsonl \\")
    print("       > data/training/combined_all.jsonl")
    print()
    print("2. Train model:")
    print("   python3 train_selfplay_combined.py")


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted by user. Partial results saved.")
    except Exception as e:
        print(f"\n\nERROR: {e}")
        raise
