import chess
import chess.engine
import json
from pathlib import Path
import time

def main():
    input_file = "data/processed/positions.jsonl"
    output_file = "data/processed/labeled_positions.jsonl"
    
    print("=" * 80)
    print("STOCKFISH POSITION LABELING WITH AGGRESSIVE FILTERING")
    print("=" * 80)
    print(f"Input: {input_file}")
    print(f"Output: {output_file}")
    print(f"Stockfish depth: 15")
    print(f"Filter: Skip positions evaluated between -0.3 and +0.3")
    print("=" * 80)
    
    # Initialize Stockfish
    engine = chess.engine.SimpleEngine.popen_uci("stockfish")
    
    positions_processed = 0
    positions_kept = 0
    positions_filtered = 0
    start_time = time.time()
    
    with open(input_file) as infile:
        with open(output_file, 'w') as outfile:
            for line in infile:
                data = json.loads(line)
                fen = data['fen']
                
                try:
                    # Create board from FEN
                    board = chess.Board(fen)
                    
                    # Analyze with Stockfish depth 15
                    info = engine.analyse(board, chess.engine.Limit(depth=15))
                    
                    # Extract evaluation (in centipawns)
                    score = info["score"].relative
                    
                    # Convert to pawns
                    if score.is_mate():
                        mate_in = score.mate()
                        eval_score = 10.0 if mate_in > 0 else -10.0
                    else:
                        eval_score = score.score() / 100.0
                    
                    # Clamp between -10 and +10
                    eval_score = max(-10.0, min(10.0, eval_score))
                    
                    positions_processed += 1
                    
                    # AGGRESSIVE FILTER: Skip boring positions
                    if -0.3 <= eval_score <= 0.3:
                        positions_filtered += 1
                    else:
                        # Keep this position
                        data['evaluation'] = eval_score
                        outfile.write(json.dumps(data) + '\n')
                        positions_kept += 1
                    
                    # Progress update every 1000 positions
                    if positions_processed % 1000 == 0:
                        elapsed = time.time() - start_time
                        rate = positions_processed / elapsed
                        eta_seconds = (2_500_000 - positions_processed) / rate
                        eta_hours = eta_seconds / 3600
                        
                        print(f"Processed: {positions_processed:,} / 2,500,000 "
                              f"({positions_processed/25000:.1f}%) | "
                              f"Kept: {positions_kept:,} | "
                              f"Filtered: {positions_filtered:,} | "
                              f"Rate: {rate:.1f} pos/sec | "
                              f"ETA: {eta_hours:.1f} hours")
                
                except Exception as e:
                    # Skip positions that cause errors
                    print(f"Error on position {positions_processed}: {e}")
                    continue
    
    engine.quit()
    
    elapsed = time.time() - start_time
    print("=" * 80)
    print(f"LABELING COMPLETE")
    print(f"Time taken: {elapsed/3600:.1f} hours")
    print(f"Positions processed: {positions_processed:,}")
    print(f"Positions kept: {positions_kept:,}")
    print(f"Positions filtered: {positions_filtered:,}")
    print(f"Filter rate: {positions_filtered/positions_processed*100:.1f}%")
    print(f"Output: {output_file}")
    print("=" * 80)

if __name__ == "__main__":
    main()
