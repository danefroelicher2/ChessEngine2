Part 1: Core Search Architecture 

Section 1.1:  NegaMax with Alpha-Beta Pruning - The Core Algorithm
The foundation of the classical engine is NegaMax. Instead of alternating between maximizing and minimizing perspectives, NegaMax always maximizes from the current player's viewpoint by negating the opponent's score. Next, we have Alpha-beta pruning which represents the best score we've guaranteed so far (lower bound), while beta is the opponent's best alternative (upper bound). If we find a move scoring >= beta, the opponent will avoid this position, so we can stop searching this branch entirely. This cuts the search tree by 50-75%, letting the engine search 2-3 plies deeper.

Section 1.2:  Quiescence Search - Avoiding Tactical Blindness
Moving further, we implemented quiescence search in order to avoid the horizon effect. The horizon effect is when the engine doesn't search far enough depth wise during tactical sequences. Why? Because it sees a good move and plays it - without seeing what comes soon after that move. This can lead to bad play by the engine. To negate this I implemented quiescence search that extends beyond the nominal depth, examining only tactical moves until the position becomes quiet. With this, I integrated delta pruning. This ensures that captures are skipped that can't improve the position. For example, If the current position evaluates to -200, and we can capture a pawn worth +100, the best we could hope for is -100. But if alpha is already +50 (we've found a line guaranteeing us +50), this capture is futile - it can't possibly beat what we already have. So we skip it.

Section 1.3:  Transposition Tables - Eliminating Redundant Work 
Another piece to the puzzle was implementing a transposition table. Chess positions can transpose - reach the same position through different move orders. A transposition table is a massive hash table (1M+ entries) that caches position evaluations to avoid redundant work. The table uses Zobrist hashing - a technique where each piece on each square has a random 64-bit number. The position's hash is the XOR of all these numbers, making hash updates O(1) during move make/unmake.


Part 2:  Move Ordering 

Alpha-beta pruning's effectiveness depends entirely on examining good moves first. With optimal move ordering, the first move causes a beta cutoff 50-60% of the time. With poor ordering, this drops to 10-20%, forcing the engine to search 3-5x more positions. In order to improve upon this I implemented a multi-tier move ordering system: 
	1. Transposition Table Move (highest priority) 
		○ If we've seen this position before, try the best move from that search first.
	2. Static Exchange Evaluation (SEE) for Captures 
		○ Not all captures are good. SEE simulates the complete capture sequence to determine net material .
	3. Killer Moves
		○ This heuristic (did I say this right) remembers quiet moves that caused beta cutoffs at the same depth in sibling branches. If Nf3 caused a cutoff in one line, it's likely strong in similar positions. Each depth stores 2 killer moves. 
	4. History Heuristic 
		○ Tracks how often each move (from-square to to-square) has caused beta cutoffs across the entire search. Squaring the depth gives more weight to stronger moves that work deeper in the tree.

Part 3:  Position Evaluation 

Once the search reaches a leaf node, the evaluation function determines "how good is this position?" This is where we teach the engine what makes a chess position strong or weak. For example, a knight on the rim is worth maybe 280, whereas a centralized knight is worth 360.

Pawn Structure 
	- Doubled pawns:  -10 penalty
	- Isolated pawns:  -15 penalty 
	- Passed pawns:  +20-50 depending on how advanced 

King Safety 
	- Castled: +20 bonus
	- Pawn shield: +5 per pawn in front of king
	- Becomes less important in the endgame when king should activate 

Center Control provides a bonus for pieces controlling central squares -- encouraging piece activity. 

Mobility counts pseudo-legal moves for each piece. More moves = more options = better position. This is expensive to compute, so I made it optional with a compile-time flag.


Part 4:  Performance Metrics 

Part 4.1:  Raw Search Performance
The engine achieves approximately 12,000-15,000 nodes per second on consumer hardware after optimizations. For context, modern engines at this strength level typically reach 50,000-500,000 nodes/second, so there's significant room for improvement. However, raw speed isn't everything - what matters more is how efficiently those nodes are being searched.
	
Part 4.2:  Move Ordering Effectiveness
The critical metric for search efficiency is the first-move cutoff rate - how often does the first move examined at a node cause a beta cutoff, allowing us to skip searching all remaining moves?

Here's how my engine performs across different position types:

Position Type:       First-Move Cutoff     Nodes/Second  
Opening                             60.7%                     15,000
Early middlegame             42%                       13,500
Tactical complexity           13.7%                    6,384
Quiet endgame                  58%                       14,800

That third row was the smoking gun. In tactically sharp positions with multiple captures available, move ordering completely broke down.

Part 4.3:  The Static Exchange Evaluation (SEE) Impact
The problem was clear: the engine was trying captures in the wrong order. Before SEE, all captures were scored equally high (1,000,000+ points) using simple MVV-LVA (Most Valuable Victim - Least Valuable Attacker). This meant:
	• Knight takes queen = 1,089,680 points 
	• Knight takes pawn but hangs the knight = 1,009,680 points (terrible)
Both captures scored over 1 million, so the bad capture was still examined before killer moves (800,000-900,000 points). In the tactical test position, this caused catastrophic move ordering. 

This single optimization essentially doubled search efficiency in tactical positions by ensuring the engine examines promising moves before wasting time on captures that lose material.

Part 4.4 Overall Playing Strength 
The classical engine plays at approximately 1500 ELO based on testing against rated opponents. Key strengths:
	• Solid tactical vision (rarely hangs pieces)
	• Reasonable positional understanding from PST
	• Good king safety in opening/middlegame
	• Decent endgame technique with passed pawn evaluation
Weaknesses:
	• Occasionally misses deep combinations (6+ moves)
	• Positional evaluation could be more sophisticated
	• Limited opening book knowledge
	• Search speed still below top engines at this level


Part 5: Neural Network Training Pipeline
After building a solid classical engine, the next challenge was teaching a neural network to evaluate chess positions. This journey involved experimenting with different data sources, discovering the power of self-play training, and ultimately achieving a model that could compete with the hand-coded evaluation.

Section 5.1: Initial Approach - Lichess Master Games
The first approach used games from strong players. I downloaded the Lichess database from January 2024 and applied strict filters:

Player strength: Both players rated 2050+ ELO
Time control: 180+ seconds (avoiding blitz mistakes)
Game completion: Only finished games

From approximately 10 million games, I filtered down to 103,115 high-quality games and extracted positions, skipping the opening moves and sampling every third position to avoid correlation.
Result: 1,902,741 positions
Each position was labeled with Stockfish at depth 12, providing evaluation scores in centipawns (+68 = white ahead 0.68 pawns, -150 = black winning 1.5 pawns). This labeling process took 36.5 hours, keeping 76% of positions after filtering out drawn or boring positions.
Section 5.2: The First Model - What Went Wrong
I designed a neural network with:

Input: 768 features  (board state encoding)
Hidden layers: 1024 → 512 → 256 → 128 neurons with ReLU, batch normalization, and 0.1 dropout
Output: Single evaluation value (pawns)
Parameters: ~1.48 million

Training on 1.9M Lichess positions achieved 2.15 pawn mean absolute error (MAE)—seemingly reasonable. But when integrated into the engine, the neural network played significantly weaker than the classical engine. It made positional errors, missed tactics, and played uninspired chess.
The problem: The dataset was too balanced. 43% of positions were near-equal (±0.5 pawns), so the network learned to evaluate quiet positions but never learned what truly winning, losing, or tactically sharp positions looked like.

Section 5.3: The Self-Play Solution
The breakthrough came from training on positions generated by my own engine rather than external databases.
The self-play pipeline:

Game Generation: A C++ program automated games between my hybrid engine (70% classical + 30% neural network) and the pure classical engine, playing to completion.
Position Filtering: Removed unrealistic positions (3+ queens, impossible board states) and opening theory (first 10 moves).
Stockfish Labeling: Labeled positions at depth 18 (vs. depth 12 for Lichess), capturing more tactical nuance.
Retraining: Neural network learned from positions it would actually encounter during play.

Results: 646 self-play games generated 12,776 training positions after filtering.
New model performance: 0.35 pawn MAE
This was a 6x improvement over the Lichess model. More importantly, the neural network finally played competent chess—achieving draws against the pure classical engine in head-to-head matches.

Section 5.4: Why Self-Play Worked
Domain Alignment: Training on my engine's own games meant the neural network learned positions it would actually face, with the same search depth and tactical priorities.
Quality Over Quantity: 12,776 carefully filtered self-play positions outperformed 1.9 million Lichess positions. The self-play data was perfectly aligned with the engine's playing style.
Deeper Analysis: Stockfish depth 18 (vs. 12) provided more accurate evaluations, especially in complex tactical sequences where shallow analysis misses key variations.

Section 5.5: Production Integration
The trained model exports to ONNX format and integrates into the C++ engine via ONNX Runtime. The live deployment offers three evaluation modes:

Classical: Pure hand-coded evaluation (1700 ELO)
Neural Network: 100% ML-based evaluation
Hybrid: Weighted blend (default 70% classical / 30% neural)

The hybrid approach combines classical tactical reliability with neural network positional understanding. Users can toggle between modes at the live URL to compare playing styles.
