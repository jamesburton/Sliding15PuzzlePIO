# Sliding Puzzle Game - Implementation Plan

## MVP v1.0 - Playable Game

### Core Features

**1. Menu System**
- Main menu: Difficulty selection (Easy/Medium/Hard)
- Puzzle selection: Choose from 5 puzzles per difficulty
- Clean UI with touch buttons

**2. Game Mechanics**
- Shuffle algorithm (solvable configurations only)
- Touch input to slide tiles
- Valid move detection (only empty-adjacent tiles)
- Smooth tile movements

**3. Game State**
- Move counter
- Timer (MM:SS format)
- Win detection (all tiles in correct position)

**4. UI Components**
- Game board with tiles
- Status bar (moves, time)
- Control buttons (Restart, Back to menu)
- Win screen

**5. Core Classes**
```
SlidingPuzzle
  - int[] tiles        // Tile positions
  - int gridSize       // 3, 4, or 5
  - int emptyPos       // Empty tile position
  - int moves          // Move count
  - unsigned long startTime
  - bool isWon

  + shuffle()          // Solvable shuffle
  + canMove(pos)       // Check if tile can slide
  + moveTile(pos)      // Slide tile to empty
  + isCompleted()      // Check win condition
  + reset()            // Restart puzzle
```

### Implementation Phases

**Phase 1: Core Engine** (Commit: v1.0-engine)
- SlidingPuzzle class
- Shuffle algorithm (solvable)
- Move validation and execution
- Win detection

**Phase 2: UI Framework** (Commit: v1.0-ui)
- Menu screens (main, difficulty, puzzle selection)
- Touch button class
- Screen navigation

**Phase 3: Game Display** (Commit: v1.0-display)
- Draw puzzle grid
- Draw tiles with image slices
- Status bar (moves, timer)
- Control buttons

**Phase 4: Game Integration** (Commit: v1.0-playable)
- Connect UI to game engine
- Touch input for tile movement
- Win screen
- Restart/back functionality

### Technical Details

**Tile Rendering:**
```cpp
// Each tile displays a portion of the full 480x480 image
int tileSize = 480 / gridSize;
int srcX = (tileNum % gridSize) * tileSize;
int srcY = (tileNum / gridSize) * tileSize;
// Draw image slice at tile position
```

**Shuffle Algorithm:**
```cpp
// Use random moves from solved state
// Ensures solvable configuration
for (int i = 0; i < moves; i++) {
    // Make random valid move
}
```

**Touch to Tile:**
```cpp
int col = touchX / tileSize;
int row = touchY / tileSize;
int tilePos = row * gridSize + col;
```

### Post-MVP Enhancements

**v1.1 - Polish**
- Smooth tile animations
- Sound effects (optional)
- Visual feedback for touches

**v1.2 - Features**
- Best time/moves tracking
- Preview of solved image
- Difficulty hints

**v1.3 - Advanced**
- Auto-solve hint system
- Multiple save slots
- Statistics screen

## Development Timeline

1. ✅ v0.9: RGB565 fix, image display
2. 🚧 v1.0: Playable MVP
   - Engine: ~30 min
   - UI: ~30 min
   - Display: ~30 min
   - Integration: ~30 min
3. ⏳ v1.1-1.3: Future enhancements

## Testing Checklist

- [ ] All 15 puzzles load correctly
- [ ] 3x3 puzzle is solvable
- [ ] 4x4 puzzle is solvable
- [ ] 5x5 puzzle is solvable
- [ ] Move counter accurate
- [ ] Timer works correctly
- [ ] Win detection works
- [ ] Restart preserves puzzle choice
- [ ] Back to menu works
- [ ] Touch response is accurate

## Commit Strategy

Commit after each phase with working, testable code:
1. v1.0-engine: Core game logic
2. v1.0-ui: Menu system
3. v1.0-display: Visual rendering
4. v1.0: Final integration and MVP complete
