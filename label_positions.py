#!/usr/bin/env python3
"""
Label chess positions with Stockfish evaluations.
Reads FEN positions and outputs JSONL with evaluations.
"""

import subprocess
import json
import time
from pathlib import Path

def get_stockfish_eval(fen, depth=14):
    """
    Get Stockfish evaluation for a FEN position.
    Returns evaluation in pawns (e.g., 1.5 = +1.5 pawns for white)
    """
    try:
        # Start Stockfish
        process = subprocess.Popen(
            ['/usr/games/stockfish'],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Send commands
        commands = f"position fen {fen}\ngo depth {depth}\n"
        output, _ = process.communicate(commands, timeout=30)
        process.kill()
        
        # Parse evaluation from output
        for line in output.split('\n'):
            if 'score cp' in line:
                # Centipawn score
                parts = line.split('score cp')
                if len(parts) > 1:
                    score_str = parts[1].strip().split()[0]
                    centipawns = int(score_str)
                    return centipawns / 100.0  # Convert to pawns
            elif 'score mate' in line:
                # Mate score
                parts = line.split('score mate')
                if len(parts) > 1:
                    mate_in = int(parts[1].strip().split()[0])
                    return 100.0 if mate_in > 0 else -100.0
        
        return 0.0  # Default if parsing fails
        
    except Exception as e:
        print(f"Error evaluating position: {e}")
        return 0.0

def main():
    input_file = Path('data/self_play/positions.fen')
    output_file = Path('data/self_play/positions_with_evals.jsonl')
    
    print(f"Reading positions from: {input_file}")
    print(f"Output will be saved to: {output_file}")
    
    # Read all positions
    with open(input_file, 'r') as f:
        positions = [line.strip() for line in f if line.strip()]
    
    total = len(positions)
    print(f"Total positions to label: {total}")
    print(f"Stockfish depth: 14")
    print(f"Estimated time: ~{total * 1.0 / 60:.1f} minutes\n")
    
    # Label positions
    start_time = time.time()
    
    with open(output_file, 'w') as out:
        for i, fen in enumerate(positions, 1):
            eval_score = get_stockfish_eval(fen, depth=14)
            
            # Write to JSONL
            data = {
                'fen': fen,
                'evaluation': eval_score
            }
            out.write(json.dumps(data) + '\n')
            
            # Progress update every 100 positions
            if i % 100 == 0:
                elapsed = time.time() - start_time
                rate = i / elapsed
                remaining = (total - i) / rate
                print(f"Progress: {i}/{total} ({100*i/total:.1f}%) | "
                      f"Rate: {rate:.1f} pos/sec | "
                      f"ETA: {remaining/60:.1f} min")
    
    total_time = time.time() - start_time
    print(f"\n✅ COMPLETE!")
    print(f"Total time: {total_time/60:.1f} minutes")
    print(f"Average: {total_time/total:.2f} sec/position")
    print(f"Output saved to: {output_file}")

if __name__ == '__main__':
    main()
