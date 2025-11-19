import chess
import chess.engine
import json
from pathlib import Path

def main():
    input_file = "data/processed/positions.jsonl"
    output_file = "data/processed/positions_with_evals.jsonl"
    
    print(f"Reading positions from: {input_file}")
    print(f"Output file: {output_file}")
    print("=" * 60)
    
    # Initialize Stockfishh
    engine = chess.engine.SimpleEngine.popen_uci("/usr/games/stockfish")
    
    positions_processed = 0
    
    with open(input_file) as infile:
        with open(output_file, 'w') as outfile:
            for line in infile:
                data = json.loads(line)
                fen = data['fen']
                
                try:
                    # Create board from FEN
                    board = chess.Board(fen)
                    
                    # Analyze position with Stockfish (depth 12 for speed)
                    info = engine.analyse(board, chess.engine.Limit(depth=12))
                    
                    # Extract evaluation (in centipawns)
                    score = info["score"].relative
                    
                    # Convert to pawns (divide by 100)
                    if score.is_mate():
                        # Handle mate scores
                        mate_in = score.mate()
                        eval_score = 10.0 if mate_in > 0 else -10.0
                    else:
                        eval_score = score.score() / 100.0
                    
                    # Clamp between -10 and +10
                    eval_score = max(-10.0, min(10.0, eval_score))
                    
                    # Add evaluation to data
                    data['evaluation'] = eval_score
                    
                    # Write to output
                    outfile.write(json.dumps(data) + '\n')
                    
                    positions_processed += 1
                    
                    # Progress update every 1000 positions
                    if positions_processed % 1000 == 0:
                        print(f"Processed: {positions_processed:,} / 1,000,000 positions "
                              f"({positions_processed/10000:.1f}%)")
                
                except Exception as e:
                    # Skip positions that cause errors
                    print(f"Error on position {positions_processed}: {e}")
                    continue
    
    engine.quit()
    
    print("=" * 60)
    print(f"Complete! Processed {positions_processed:,} positions")
    print(f"Output: {output_file}")

if __name__ == "__main__":
    main()
