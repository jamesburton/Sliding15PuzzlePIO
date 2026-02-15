#include <Arduino.h>
#include <Wire.h>
#include "DisplayConfig.hpp"
#include "LGFX_Setup.hpp"
#include "PuzzleManager.hpp"

LGFX tft;
PuzzleManager puzzleManager;

// ==============================================================================
// ST7701S Manual Initialization (3-Wire SPI)
// ==============================================================================
void st7701_send(uint8_t data, bool is_cmd) {
    digitalWrite(PIN_SPI_CS, LOW);

    // 9-bit SPI: 1st bit is D/C (0=Cmd, 1=Data)
    digitalWrite(PIN_SPI_SCK, LOW);
    digitalWrite(PIN_SPI_SDA, is_cmd ? LOW : HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_SPI_SCK, HIGH);
    delayMicroseconds(1);

    for (int i = 0; i < 8; i++) {
        digitalWrite(PIN_SPI_SCK, LOW);
        if (data & 0x80) digitalWrite(PIN_SPI_SDA, HIGH);
        else             digitalWrite(PIN_SPI_SDA, LOW);
        delayMicroseconds(1);
        digitalWrite(PIN_SPI_SCK, HIGH);
        delayMicroseconds(1);
        data <<= 1;
    }
    digitalWrite(PIN_SPI_CS, HIGH);
    delayMicroseconds(1);
}

void st7701_write_command(uint8_t c) {
    st7701_send(c, true);
}

void st7701_write_data(uint8_t d) {
    st7701_send(d, false);
}

void run_init_sequence() {
    Serial.println("ST7701: Starting Manual Init...");
    pinMode(PIN_SPI_CS, OUTPUT);
    pinMode(PIN_SPI_SDA, OUTPUT);
    pinMode(PIN_SPI_SCK, OUTPUT);

    digitalWrite(PIN_SPI_CS, HIGH);
    digitalWrite(PIN_SPI_SCK, HIGH);
    digitalWrite(PIN_SPI_SDA, HIGH);

    delay(120);

    const size_t n = sizeof(ST7701_INIT_SEQUENCE);
    size_t i = 0;
    while (i < n) {
        const uint8_t cmd = ST7701_INIT_SEQUENCE[i++];
        if (cmd == 0x00 && i >= n) break;

        if (cmd == 0xFF) {
            st7701_write_command(cmd);
            if (i >= n) break;
            uint8_t len = ST7701_INIT_SEQUENCE[i++];
            for (uint8_t j = 0; j < len; j++) {
                if (i >= n) break;
                st7701_write_data(ST7701_INIT_SEQUENCE[i++]);
            }
            continue;
        }

        if (cmd == 0x00) break;

        st7701_write_command(cmd);
        if (i >= n) break;
        uint8_t len = ST7701_INIT_SEQUENCE[i++];

        for (uint8_t j = 0; j < len; j++) {
            if (i >= n) break;
            st7701_write_data(ST7701_INIT_SEQUENCE[i++]);
        }

        if (cmd == 0x11) delay(120);
        if (cmd == 0x29) delay(50);
        if (cmd == 0xFF) delay(10);
    }
    Serial.println("ST7701: Manual Init Done.");
}

// ==============================================================================
// Demo: Display a puzzle image
// ==============================================================================
void displayPuzzleDemo(int difficulty, int puzzleIndex) {
    const PuzzleInfo& puzzle = puzzleManager.getPuzzle(difficulty, puzzleIndex);

    Serial.printf("\n=== Displaying Puzzle ===\n");
    Serial.printf("Name: %s\n", puzzle.displayName.c_str());
    Serial.printf("File: %s\n", puzzle.filename.c_str());
    Serial.printf("Grid: %dx%d\n", puzzle.gridSize, puzzle.gridSize);

    // Check file exists
    if (!LittleFS.exists(puzzle.filename)) {
        Serial.println("ERROR: File does not exist!");
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setTextDatum(textdatum_t::middle_center);
        tft.drawString("FILE NOT FOUND", 240, 240);
        return;
    }

    // Open RGB565 file
    File file = LittleFS.open(puzzle.filename, "r");
    if (!file) {
        Serial.println("ERROR: Failed to open file!");
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setTextDatum(textdatum_t::middle_center);
        tft.drawString("OPEN ERROR", 240, 240);
        return;
    }

    size_t fileSize = file.size();
    Serial.printf("File size: %d bytes\n", fileSize);

    // Verify size (480x480x2 bytes = 460,800 bytes)
    if (fileSize != 460800) {
        Serial.printf("ERROR: Invalid file size! Expected 460800, got %d\n", fileSize);
        file.close();
        tft.fillScreen(TFT_ORANGE);
        tft.setTextColor(TFT_BLACK);
        tft.setTextSize(2);
        tft.setTextDatum(textdatum_t::middle_center);
        tft.drawString("INVALID FILE SIZE", 240, 240);
        return;
    }

    // Read and display RGB565 data
    Serial.println("Loading RGB565 image...");
    uint16_t* buffer = (uint16_t*)ps_malloc(480 * 2 * sizeof(uint16_t)); // 1 row buffer
    if (!buffer) {
        Serial.println("ERROR: Failed to allocate buffer!");
        file.close();
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setTextDatum(textdatum_t::middle_center);
        tft.drawString("OUT OF MEMORY", 240, 240);
        return;
    }

    tft.startWrite();
    tft.setAddrWindow(0, 0, 480, 480);

    for (int y = 0; y < 480; y++) {
        size_t bytesRead = file.read((uint8_t*)buffer, 480 * 2);
        if (bytesRead != 480 * 2) {
            Serial.printf("ERROR: Read error at row %d\n", y);
            break;
        }
        tft.writePixels(buffer, 480);
    }

    tft.endWrite();
    file.close();
    free(buffer);

    Serial.println("Puzzle displayed successfully!");

    // Draw label overlay
    tft.fillRect(0, 0, 480, 35, TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setTextDatum(textdatum_t::top_left);
    tft.drawString(puzzle.displayName, 5, 8);

    const char* diffNames[] = {"EASY", "MEDIUM", "HARD"};
    tft.setTextDatum(textdatum_t::top_right);
    tft.drawString(diffNames[difficulty], 475, 8);
}

// ==============================================================================
// Setup
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  Sliding 15-Puzzle Demo");
    Serial.println("  ESP32-4848S040C_I");
    Serial.println("========================================");

    // Check PSRAM
    Serial.printf("PSRAM Size: %d bytes (%.2f MB)\n",
                  ESP.getPsramSize(),
                  ESP.getPsramSize() / 1024.0 / 1024.0);
    Serial.printf("Free PSRAM: %d bytes (%.2f MB)\n",
                  ESP.getFreePsram(),
                  ESP.getFreePsram() / 1024.0 / 1024.0);

    // 1. Run Manual Init for ST7701S
    run_init_sequence();

    // 2. Initialize LGFX
    Serial.println("Initializing TFT...");
    tft.init();
    tft.setBrightness(255);
    tft.fillScreen(TFT_BLACK);

    // 3. Initialize PuzzleManager
    Serial.println("\nInitializing PuzzleManager...");
    if (!puzzleManager.init()) {
        Serial.println("\nERROR: PuzzleManager initialization failed!");
        Serial.println("Make sure to upload filesystem:");
        Serial.println("  pio run --target uploadfs");

        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setTextDatum(textdatum_t::middle_center);
        tft.drawString("FILESYSTEM ERROR", 240, 200);
        tft.setTextSize(1);
        tft.drawString("Run: pio run --target uploadfs", 240, 240);
        while(1) delay(1000);
    }

    // 4. List files (for debugging)
    puzzleManager.listFiles();

    // 5. Display welcome screen
    tft.fillScreen(TFT_NAVY);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(4);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.drawString("SLIDING", 240, 180);
    tft.drawString("PUZZLE", 240, 230);

    tft.setTextSize(2);
    tft.drawString("15 Puzzles Loaded!", 240, 300);

    tft.setTextSize(1);
    tft.drawString("Touch to start demo...", 240, 400);

    Serial.println("\n========================================");
    Serial.println("Setup Complete!");
    Serial.println("Touch screen to cycle through puzzles");
    Serial.println("========================================\n");
}

// ==============================================================================
// Loop - Demo Mode (cycles through puzzles on touch)
// ==============================================================================
int currentDifficulty = 1;  // Start with Medium
int currentPuzzleIndex = 0;
bool waitingForTouch = true;
bool lastTouchState = false;

void loop() {
    int32_t x, y;
    bool touching = tft.getTouch(&x, &y);

    // Detect touch press (not hold)
    if (touching && !lastTouchState) {
        Serial.printf("\nTouch detected at (%d, %d)\n", x, y);

        if (waitingForTouch) {
            waitingForTouch = false;
        }

        // Cycle to next puzzle
        currentPuzzleIndex++;
        if (currentPuzzleIndex >= puzzleManager.getPuzzleCount(currentDifficulty)) {
            currentPuzzleIndex = 0;
            currentDifficulty = (currentDifficulty + 1) % 3;
        }

        displayPuzzleDemo(currentDifficulty, currentPuzzleIndex);

        delay(300);  // Debounce
    }

    lastTouchState = touching;
    delay(10);
}
