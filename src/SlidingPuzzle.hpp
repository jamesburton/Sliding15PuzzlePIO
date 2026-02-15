#pragma once

#include <Arduino.h>
#include <vector>
#include <algorithm>

class SlidingPuzzle {
private:
    std::vector<int> tiles;     // Tile positions (0 = empty)
    int gridSize;                // 3, 4, or 5
    int emptyPos;                // Position of empty tile
    int moveCount;
    unsigned long startTime;
    bool gameWon;
    bool gameStarted;

    // Helper: Get position from row/col
    int pos(int row, int col) const {
        return row * gridSize + col;
    }

    // Helper: Get row from position
    int row(int p) const {
        return p / gridSize;
    }

    // Helper: Get col from position
    int col(int p) const {
        return p % gridSize;
    }

    // Check if puzzle is currently solved
    bool checkWinCondition() {
        for (int i = 0; i < tiles.size() - 1; i++) {
            if (tiles[i] != i + 1) return false;
        }
        return tiles[tiles.size() - 1] == 0;  // Empty tile at end
    }

    // Count inversions (for solvability check)
    int countInversions() const {
        int inv = 0;
        for (int i = 0; i < tiles.size() - 1; i++) {
            if (tiles[i] == 0) continue;
            for (int j = i + 1; j < tiles.size(); j++) {
                if (tiles[j] == 0) continue;
                if (tiles[i] > tiles[j]) inv++;
            }
        }
        return inv;
    }

    // Check if current configuration is solvable
    bool isSolvable() const {
        int inv = countInversions();

        if (gridSize % 2 == 1) {
            // Odd grid: solvable if inversions are even
            return (inv % 2 == 0);
        } else {
            // Even grid: solvable if (inversions + empty row from bottom) is odd
            int emptyRow = row(emptyPos);
            int emptyRowFromBottom = gridSize - emptyRow;
            return ((inv + emptyRowFromBottom) % 2 == 1);
        }
    }

public:
    SlidingPuzzle(int size = 3) : gridSize(size), moveCount(0), gameWon(false), gameStarted(false) {
        reset();
    }

    // Initialize puzzle in solved state
    void reset() {
        int totalTiles = gridSize * gridSize;
        tiles.clear();
        tiles.reserve(totalTiles);

        // Fill tiles 1 to N-1, then 0 (empty)
        for (int i = 1; i < totalTiles; i++) {
            tiles.push_back(i);
        }
        tiles.push_back(0);  // Empty tile at end

        emptyPos = totalTiles - 1;
        moveCount = 0;
        gameWon = false;
        gameStarted = false;
        startTime = 0;
    }

    // Shuffle puzzle with guaranteed solvable configuration
    void shuffle(int numMoves = 100) {
        // Use random moves from solved state to ensure solvability
        for (int i = 0; i < numMoves; i++) {
            std::vector<int> validMoves;

            int emptyRow = row(emptyPos);
            int emptyCol = col(emptyPos);

            // Check all 4 directions
            if (emptyRow > 0) validMoves.push_back(pos(emptyRow - 1, emptyCol));  // Up
            if (emptyRow < gridSize - 1) validMoves.push_back(pos(emptyRow + 1, emptyCol));  // Down
            if (emptyCol > 0) validMoves.push_back(pos(emptyRow, emptyCol - 1));  // Left
            if (emptyCol < gridSize - 1) validMoves.push_back(pos(emptyRow, emptyCol + 1));  // Right

            if (!validMoves.empty()) {
                int randomMove = validMoves[random(validMoves.size())];
                // Swap with empty (no move count increase during shuffle)
                std::swap(tiles[emptyPos], tiles[randomMove]);
                emptyPos = randomMove;
            }
        }

        // Reset counters after shuffle
        moveCount = 0;
        gameWon = false;
        gameStarted = false;
    }

    // Check if a tile at given position can move
    bool canMove(int tilePos) const {
        if (tilePos < 0 || tilePos >= tiles.size()) return false;
        if (tiles[tilePos] == 0) return false;  // Can't move empty tile

        int tileRow = row(tilePos);
        int tileCol = col(tilePos);
        int emptyRow = row(emptyPos);
        int emptyCol = col(emptyPos);

        // Check if adjacent to empty
        bool adjacent = (abs(tileRow - emptyRow) == 1 && tileCol == emptyCol) ||
                       (abs(tileCol - emptyCol) == 1 && tileRow == emptyRow);

        return adjacent;
    }

    // Move tile at position (if valid)
    bool moveTile(int tilePos) {
        if (!canMove(tilePos)) return false;

        // Start timer on first move
        if (!gameStarted) {
            gameStarted = true;
            startTime = millis();
        }

        // Swap tile with empty
        std::swap(tiles[emptyPos], tiles[tilePos]);
        emptyPos = tilePos;
        moveCount++;

        // Check win condition
        if (checkWinCondition()) {
            gameWon = true;
        }

        return true;
    }

    // Get tile number at position (0 = empty)
    int getTile(int pos) const {
        if (pos < 0 || pos >= tiles.size()) return -1;
        return tiles[pos];
    }

    // Get tile number at row/col
    int getTile(int r, int c) const {
        return getTile(pos(r, c));
    }

    // Getters
    int getGridSize() const { return gridSize; }
    int getMoveCount() const { return moveCount; }
    int getEmptyPos() const { return emptyPos; }
    bool isWon() const { return gameWon; }
    bool hasStarted() const { return gameStarted; }

    unsigned long getElapsedTime() const {
        if (!gameStarted) return 0;
        if (gameWon) return 0;  // Could store final time
        return (millis() - startTime) / 1000;  // Return seconds
    }

    // Debug: Print board state
    void printBoard() const {
        Serial.println("Board:");
        for (int r = 0; r < gridSize; r++) {
            for (int c = 0; c < gridSize; c++) {
                int tile = getTile(r, c);
                if (tile == 0) {
                    Serial.print("[  ] ");
                } else {
                    Serial.printf("[%2d] ", tile);
                }
            }
            Serial.println();
        }
        Serial.printf("Moves: %d, Empty: %d, Won: %s\n",
                     moveCount, emptyPos, gameWon ? "YES" : "NO");
    }
};
