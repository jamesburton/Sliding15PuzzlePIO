#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>

struct PuzzleInfo {
    String filename;
    String displayName;
    int gridSize;     // 3, 4, or 5
    int difficulty;   // 0=Easy, 1=Medium, 2=Hard
};

class PuzzleManager {
private:
    std::vector<PuzzleInfo> easyPuzzles;
    std::vector<PuzzleInfo> mediumPuzzles;
    std::vector<PuzzleInfo> hardPuzzles;

public:
    bool init() {
        Serial.println("Initializing PuzzleManager...");

        if (!LittleFS.begin(true)) {  // format on fail
            Serial.println("ERROR: LittleFS mount failed!");
            return false;
        }

        Serial.println("LittleFS mounted successfully");

        // Check available space
        size_t totalBytes = LittleFS.totalBytes();
        size_t usedBytes = LittleFS.usedBytes();
        Serial.printf("LittleFS: %d / %d bytes used\n", usedBytes, totalBytes);

        // Easy puzzles (3x3) - RGB565 Format
        easyPuzzles = {
            {"/puzzles/easy/castle.rgb565", "Castle Sunset", 3, 0},
            {"/puzzles/easy/icecream.rgb565", "Ice Cream Park", 3, 0},
            {"/puzzles/easy/puppy.rgb565", "Puppy Car", 3, 0},
            {"/puzzles/easy/planet.rgb565", "Space Planet", 3, 0},
            {"/puzzles/easy/turtle_reef.rgb565", "Turtle Reef", 3, 0}
        };

        // Medium puzzles (4x4) - RGB565 Format
        mediumPuzzles = {
            {"/puzzles/medium/forest.rgb565", "Autumn Path", 4, 1},
            {"/puzzles/medium/market.rgb565", "Fantasy Market", 4, 1},
            {"/puzzles/medium/robot.rgb565", "Robot Workshop", 4, 1},
            {"/puzzles/medium/hangar.rgb565", "Sci-Fi Hangar", 4, 1},
            {"/puzzles/medium/beach.rgb565", "Beach Paradise", 4, 1}
        };

        // Hard puzzles (5x5) - RGB565 Format
        hardPuzzles = {
            {"/puzzles/hard/nebula.rgb565", "Cosmic Nebula", 5, 2},
            {"/puzzles/hard/cyberpunk.rgb565", "Cyberpunk City", 5, 2},
            {"/puzzles/hard/gears.rgb565", "Mechanical Gears", 5, 2},
            {"/puzzles/hard/androids.rgb565", "Android Pile", 5, 2},
            {"/puzzles/hard/library.rgb565", "Wizard Library", 5, 2}
        };

        Serial.println("Puzzle lists initialized:");
        Serial.printf("  Easy: %d puzzles (3x3)\n", easyPuzzles.size());
        Serial.printf("  Medium: %d puzzles (4x4)\n", mediumPuzzles.size());
        Serial.printf("  Hard: %d puzzles (5x5)\n", hardPuzzles.size());

        // Verify files exist
        Serial.println("\nVerifying puzzle files...");
        int foundCount = 0;
        int missingCount = 0;

        for (const auto& puzzle : easyPuzzles) {
            if (LittleFS.exists(puzzle.filename)) {
                foundCount++;
            } else {
                Serial.printf("  MISSING: %s\n", puzzle.filename.c_str());
                missingCount++;
            }
        }

        for (const auto& puzzle : mediumPuzzles) {
            if (LittleFS.exists(puzzle.filename)) {
                foundCount++;
            } else {
                Serial.printf("  MISSING: %s\n", puzzle.filename.c_str());
                missingCount++;
            }
        }

        for (const auto& puzzle : hardPuzzles) {
            if (LittleFS.exists(puzzle.filename)) {
                foundCount++;
            } else {
                Serial.printf("  MISSING: %s\n", puzzle.filename.c_str());
                missingCount++;
            }
        }

        Serial.printf("Files found: %d / %d\n", foundCount, foundCount + missingCount);

        if (missingCount > 0) {
            Serial.println("\nWARNING: Some puzzle files are missing!");
            Serial.println("Run: pio run --target uploadfs");
            return false;
        }

        Serial.println("All puzzle files verified ✓");
        return true;
    }

    const std::vector<PuzzleInfo>& getPuzzles(int difficulty) const {
        switch(difficulty) {
            case 0: return easyPuzzles;
            case 1: return mediumPuzzles;
            case 2: return hardPuzzles;
            default: return mediumPuzzles;
        }
    }

    int getPuzzleCount(int difficulty) const {
        return getPuzzles(difficulty).size();
    }

    const PuzzleInfo& getPuzzle(int difficulty, int index) const {
        const auto& puzzles = getPuzzles(difficulty);
        return puzzles[index % puzzles.size()];
    }

    bool fileExists(const String& filename) const {
        return LittleFS.exists(filename);
    }

    File openPuzzleFile(const String& filename) const {
        return LittleFS.open(filename, "r");
    }

    void listFiles() const {
        Serial.println("\nLittleFS Directory Listing:");
        listDir(LittleFS, "/", 3);
    }

private:
    void listDir(fs::FS &fs, const char* dirname, uint8_t levels) const {
        Serial.printf("Listing directory: %s\n", dirname);

        File root = fs.open(dirname);
        if (!root) {
            Serial.println("Failed to open directory");
            return;
        }
        if (!root.isDirectory()) {
            Serial.println("Not a directory");
            return;
        }

        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                Serial.print("  DIR : ");
                Serial.println(file.name());
                if (levels) {
                    listDir(fs, file.path(), levels - 1);
                }
            } else {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("\tSIZE: ");
                Serial.println(file.size());
            }
            file = root.openNextFile();
        }
    }
};
