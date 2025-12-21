#!/usr/bin/env python3
import subprocess
import json
import time
from pathlib import Path

STOCKFISH_PATH = "/usr/games/stockfish"
INPUT_FILE = "data/self_play/positions_final_adjusted.fen"
OUTPUT_FILE = "data/self_play/positions_labeled_batch2.jsonl"
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
        
    except:
        try:
            process.kill()
        except:
            pass
        return None


def main():
    input_path = Path(INPUT_FILE)
    output_path = Path(OUTPUT_FILE)
    
    print("=" * 70)
    print("LABELING BATCH 2 - 16-26 PIECES")
    print("=" * 70)
    print(f"Input:  {input_path}")
    print(f"Output: {output_path}")
    print()
    
    if not input_path.exists():
        print(f"ERROR: {input_path} not found!")
        return
    
    with open(input_path, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Positions to label: {total:,}")
    print(f"Expected time: ~3.3 hours")
    print()
    
    labeled = 0
    failed = 0
    start_time = time.time()
    
    with open(output_path, 'w') as out:
        for i, fen in enumerate(positions, 1):
            eval_score = get_stockfish_eval(fen, depth=DEPTH, timeout=TIMEOUT)
            
            if eval_score is None:
                failed += 1
                if failed <= 20 or failed % 100 == 0:
                    print(f"  FAIL #{failed}")
                continue
            
            labeled += 1
            data = {'fen': fen, 'evaluation': eval_score}
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
                print(f"  Labeled: {labeled:,} | Failed: {failed:,} | Fail: {fail_rate:.1f}%")
                print(f"  Rate: {rate:.2f} pos/sec | ETA: {eta_hours:.1f}h")
                print()
    
    total_time = time.time() - start_time
    
    print()
    print("=" * 70)
    print("✅ BATCH 2 COMPLETE")
    print("=" * 70)
    print(f"Labeled: {labeled:,} | Failed: {failed:,}")
    print(f"Time: {total_time/3600:.2f} hours")
    print(f"Output: {output_path}")
    print()
    print("COMBINE ALL DATA:")
    print("  cat data/self_play/positions_with_evals_FINAL.jsonl \\")
    print("      data/self_play/positions_labeled.jsonl \\")
    print("      data/self_play/positions_labeled_batch2.jsonl \\")
    print("      > data/training/all_selfplay_combined.jsonl")


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nInterrupted. Partial results saved.")
