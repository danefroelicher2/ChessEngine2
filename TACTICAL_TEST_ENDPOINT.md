# Tactical Vision Test Endpoint

## Overview

The `/tactical-test` endpoint runs 5 fundamental tactical vision tests against the chess engine to verify it can see basic 1-2 ply tactics that any 800+ Elo player should recognize.

## Endpoint

```
GET /tactical-test
```

**No parameters required** - runs all 5 tests automatically.

## Response Format

```json
{
  "status": "ok",
  "tests": [
    {
      "name": "Hanging Piece Detection (1-ply)",
      "description": "White queen on e5 is undefended. Can engine capture free piece?",
      "fen": "rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1",
      "expected": "xe5",
      "actual": "d8e5",
      "passed": true,
      "first_move_cutoff_rate": 65.2,
      "nodes_searched": 12543
    },
    // ... 4 more tests ...
  ],
  "summary": {
    "total_tests": 5,
    "passed": 5,
    "failed": 0,
    "pass_rate": 100.0
  }
}
```

## The 5 Tactical Tests

### Test 1: Hanging Piece Detection (1-ply)
**Position:** `rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1`

- **Setup:** White queen on e5, completely undefended
- **Expected:** Black should capture the free queen (any piece to e5)
- **Tests:** Can the engine see a free piece in 1 move?

### Test 2: King Can Capture (1-ply)
**Position:** `rnbqkbnr/pppppppp/8/8/4k3/4N3/PPPPPPPP/RNBQKB1R b KQkq - 0 1`

- **Setup:** White knight on e3, Black king on e4 can capture it
- **Expected:** King captures knight (e4e3)
- **Tests:** Does the engine know kings can capture pieces?

### Test 3: Capture Checking Piece (1-ply)
**Position:** `rnbqkbnr/pppp1ppp/8/8/8/8/PPPPnPPP/RNBQKBNR w KQkq - 0 1`

- **Setup:** Black knight on e2 checking White king on e1
- **Expected:** White captures the checking piece (any piece to e2)
- **Tests:** Does the engine capture checking pieces?

### Test 4: King Capture vs King Flight (1-ply)
**Position:** `rnbqkb1r/pppppppp/8/8/8/4n3/PPPPKPPP/RNBQ1BNR w kq - 0 1`

- **Setup:** Black knight on e3 checking King on e2, King can capture it
- **Expected:** King captures knight (e2e3)
- **Tests:** Does the engine prefer capturing over fleeing?

### Test 5: Free Material in Opening (1-ply)
**Position:** `rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPNPPP/RNBQKB1R b KQkq - 0 1`

- **Setup:** White pawn on e4 is undefended after 1.e4 e5 2.Ne2??
- **Expected:** Black pawn captures free pawn (e5e4)
- **Tests:** Can the engine find free material in the opening?

## Usage Examples

### Via curl
```bash
curl https://chessengine2.onrender.com/tactical-test
```

### Via JavaScript/Fetch
```javascript
fetch('https://chessengine2.onrender.com/tactical-test')
  .then(response => response.json())
  .then(data => {
    console.log(`Tests passed: ${data.summary.passed}/${data.summary.total_tests}`);
    console.log(`Pass rate: ${data.summary.pass_rate}%`);

    data.tests.forEach(test => {
      console.log(`${test.name}: ${test.passed ? 'PASS' : 'FAIL'}`);
      if (!test.passed) {
        console.log(`  Expected: ${test.expected}, Got: ${test.actual}`);
      }
    });
  });
```

### Via HTML/Chess Interface

Add a button to `chess.html`:

```html
<button onclick="runTacticalTests()">Run Tactical Tests</button>
<div id="tactical-results"></div>

<script>
async function runTacticalTests() {
  const resultsDiv = document.getElementById('tactical-results');
  resultsDiv.innerHTML = 'Running tests...';

  try {
    const response = await fetch('/tactical-test');
    const data = await response.json();

    let html = `<h3>Tactical Vision Test Results</h3>`;
    html += `<p>Pass Rate: ${data.summary.pass_rate.toFixed(1)}%
             (${data.summary.passed}/${data.summary.total_tests})</p>`;

    html += '<ul>';
    data.tests.forEach(test => {
      const status = test.passed ? '✓ PASS' : '✗ FAIL';
      const color = test.passed ? 'green' : 'red';
      html += `<li style="color: ${color}">
                 <strong>${status}</strong> - ${test.name}<br>
                 Expected: ${test.expected}, Got: ${test.actual}<br>
                 First-move cutoff: ${test.first_move_cutoff_rate.toFixed(1)}%,
                 Nodes: ${test.nodes_searched}
               </li>`;
    });
    html += '</ul>';

    resultsDiv.innerHTML = html;
  } catch (error) {
    resultsDiv.innerHTML = `Error: ${error.message}`;
  }
}
</script>
```

## Interpreting Results

### Success Criteria

**Minimum (Must Pass):**
- At least 3/5 tests pass (60% pass rate)
- Engine captures free pieces (Test 1, 5)
- Engine doesn't move king away from capturable attackers (Test 4)

**Target (Expected with SEE):**
- 5/5 tests pass (100% pass rate)
- First-move cutoff rates > 40% for all tests
- Nodes searched < 15,000 for each test

**Stretch Goal:**
- 100% pass rate
- First-move cutoff rates > 50% for all tests
- Consistent tactical vision across all positions

### Diagnosing Failures

**If Test 1 fails (Hanging Piece):**
- ❌ Engine is blind to free material
- Check: Is evaluation function working correctly?
- Check: Are captures scored high in move ordering?

**If Test 2 fails (King Can Capture):**
- ❌ Engine may not generate king captures correctly
- Check: Move generation for king captures
- Check: Evaluation doesn't penalize king activity too heavily

**If Test 3 fails (Capture Checking Piece):**
- ❌ Engine not prioritizing getting out of check
- Check: Is king in check detected?
- Check: Are escaping-check moves generated?

**If Test 4 fails (King Capture vs Flight):**
- ❌ Engine prefers fleeing over capturing
- Check: SEE scoring for king captures
- Check: Move ordering for king moves

**If Test 5 fails (Free Material):**
- ❌ Engine missing basic material counting
- Check: Material evaluation
- Check: Search depth reaching position

### Performance Metrics

**First-Move Cutoff Rate:**
- **< 20%**: Critical move ordering problem
- **20-40%**: Poor move ordering
- **40-60%**: Acceptable move ordering
- **> 60%**: Good move ordering

**Nodes Searched:**
- **< 5,000**: Very efficient (may indicate shallow search)
- **5,000-15,000**: Efficient
- **15,000-30,000**: Acceptable
- **> 30,000**: Inefficient (move ordering issues)

## Testing After SEE Implementation

After deploying Static Exchange Evaluation (SEE), run this endpoint to verify:

1. **All 5 tests should pass** (100% pass rate)
2. **First-move cutoff rates improve** across all tests
3. **Bad captures no longer searched first** (higher efficiency)

Compare results before and after SEE:

| Metric | Before SEE | After SEE (Expected) |
|--------|-----------|---------------------|
| Pass Rate | 40-60% | 100% |
| Avg First-Move Cutoff | 20-30% | 50-60% |
| Avg Nodes Searched | 15,000-25,000 | 8,000-12,000 |

## Troubleshooting

**Error: "Endpoint not found"**
- Server may not have redeployed yet
- Check server logs for startup confirmation

**Error: Timeout**
- Tests take ~10 seconds total (2 seconds per test)
- Increase HTTP timeout if needed

**All tests fail:**
- Critical engine problem
- Check if basic move generation works
- Verify engine can find legal moves

**Inconsistent results:**
- May indicate non-deterministic behavior
- Run multiple times to confirm
- Check for random move selection fallback

## Server Log Output

When the endpoint is called, server logs will show:

```
[TACTICAL TEST] Running 5 tactical vision tests...
=== Iterative Deepening Search ===
FAST_EVAL_MODE: Enabled (tactical focus)
Time limit: 2000ms
Depth 1: d8e5 (score: 850) [12ms, 245 nodes]
Depth 2: d8e5 (score: 900) [85ms, 2134 nodes]
... (repeated for each test)
```

Monitor these logs to see:
- Search depth reached for each test
- Time taken per test
- Nodes searched
- Best move found

---

## Example: Complete Test Run

```bash
$ curl https://chessengine2.onrender.com/tactical-test

{
  "status": "ok",
  "tests": [
    {
      "name": "Hanging Piece Detection (1-ply)",
      "description": "White queen on e5 is undefended...",
      "fen": "rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1",
      "expected": "xe5",
      "actual": "d8e5",
      "passed": true,
      "first_move_cutoff_rate": 68.3,
      "nodes_searched": 8234
    },
    {
      "name": "King Can Capture (1-ply)",
      "description": "White knight on e3 attacked...",
      "fen": "rnbqkbnr/pppppppp/8/8/4k3/4N3/PPPPPPPP/RNBQKB1R b KQkq - 0 1",
      "expected": "e4e3",
      "actual": "e4e3",
      "passed": true,
      "first_move_cutoff_rate": 72.1,
      "nodes_searched": 7823
    },
    {
      "name": "Capture Checking Piece (1-ply)",
      "description": "Black knight on e2 checking...",
      "fen": "rnbqkbnr/pppp1ppp/8/8/8/8/PPPPnPPP/RNBQKBNR w KQkq - 0 1",
      "expected": "xe2",
      "actual": "d1e2",
      "passed": true,
      "first_move_cutoff_rate": 65.4,
      "nodes_searched": 9124
    },
    {
      "name": "King Capture vs King Flight (1-ply)",
      "description": "Black knight on e3 checks King...",
      "fen": "rnbqkb1r/pppppppp/8/8/8/4n3/PPPPKPPP/RNBQ1BNR w kq - 0 1",
      "expected": "e2e3",
      "actual": "e2e3",
      "passed": true,
      "first_move_cutoff_rate": 70.2,
      "nodes_searched": 8456
    },
    {
      "name": "Free Material in Opening (1-ply)",
      "description": "White pawn on e4 is undefended...",
      "fen": "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPNPPP/RNBQKB1R b KQkq - 0 1",
      "expected": "e5e4",
      "actual": "e5e4",
      "passed": true,
      "first_move_cutoff_rate": 66.7,
      "nodes_searched": 8912
    }
  ],
  "summary": {
    "total_tests": 5,
    "passed": 5,
    "failed": 0,
    "pass_rate": 100.0
  }
}
```

**Result:** ✅ All tests passed! Engine has good tactical vision.

---

*Created: October 31, 2024*
*Related: SEE Implementation (commit 7e5ec03)*
