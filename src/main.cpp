#include <Arduino.h>
#include <Wire.h>
#include "DisplayConfig.hpp"
#include "LGFX_Setup.hpp"
#include "PuzzleManager.hpp"
#include "SlidingPuzzle.hpp"

// ==============================================================================
// Sound Configuration (Optional)
// ==============================================================================
// Uncomment the line below to enable sound effects
// Hardware Required: Passive buzzer connected to BUZZER_PIN
// #define ENABLE_SOUND

#ifdef ENABLE_SOUND
  #define BUZZER_PIN 2        // GPIO2 (shared with relay, can be changed)
  #define BUZZER_CHANNEL 0    // LEDC channel for PWM
  #define BUZZER_RESOLUTION 8 // 8-bit resolution (0-255)
#endif

// ==============================================================================
// Game State
// ==============================================================================
enum GameState { MAIN_MENU, PUZZLE_SELECT, PLAYING, WIN_SCREEN };

LGFX tft;
PuzzleManager puzzleManager;
SlidingPuzzle* puzzle = nullptr;

GameState gameState = MAIN_MENU;
int selectedDifficulty = 0;
int selectedPuzzle = 0;

// Image buffer in PSRAM (480x480 RGB565 = 460800 bytes)
uint16_t* puzzleImageBuffer = nullptr;

// Timer tracking
unsigned long gameStartTime = 0;
unsigned long gameEndTime = 0;
bool timerRunning = false;
int lastDisplayedSeconds = -1;
int lastDisplayedMoves = -1;

// Touch debouncing
bool lastTouchState = false;
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE_MS = 250;

// Animation state
bool isAnimating = false;
unsigned long animStartTime = 0;
int animFromPos = -1;
int animToPos = -1;
int animTileNum = 0;
const unsigned long ANIM_DURATION_MS = 180;

// Touch feedback state
int flashTile = -1;
unsigned long flashStartTime = 0;
uint16_t flashColor = 0;
const unsigned long FLASH_DURATION_MS = 100;

// UI Constants
const int STATUS_BAR_HEIGHT = 40;
const int BUTTON_BAR_HEIGHT = 50;
const int GAME_AREA_Y = STATUS_BAR_HEIGHT;
const int GAME_AREA_SIZE = 480 - STATUS_BAR_HEIGHT - BUTTON_BAR_HEIGHT;
// Grid draws within GAME_AREA_Y to GAME_AREA_Y + GAME_AREA_SIZE

// Forward declarations
void showMainMenu();
void showPuzzleSelect(int difficulty);
void showWinScreen();
void startGame(int difficulty, int puzzleIndex);

#ifdef ENABLE_SOUND
void initSound();
void playTone(int frequency, int duration);
void playSlideSound();
void playErrorSound();
void playWinSound();
#endif

// Colors
const uint16_t COL_BG        = 0x1082;  // Dark gray
const uint16_t COL_MENU_BG   = 0x000F;  // Dark blue
const uint16_t COL_BTN       = 0x2945;  // Button gray
const uint16_t COL_BTN_EASY  = 0x07E0;  // Green
const uint16_t COL_BTN_MED   = 0xFFE0;  // Yellow
const uint16_t COL_BTN_HARD  = 0xF800;  // Red
const uint16_t COL_BTN_SEL   = 0x04FF;  // Cyan
const uint16_t COL_WHITE     = 0xFFFF;
const uint16_t COL_BLACK     = 0x0000;
const uint16_t COL_GRID_LINE = 0x4208;  // Grid separator
const uint16_t COL_EMPTY     = 0x2104;  // Empty tile
const uint16_t COL_WIN_BG    = 0x0320;  // Dark green
const uint16_t COL_GOLD      = 0xFEA0;  // Gold
const uint16_t COL_FLASH_VALID   = 0xFFFF;  // White highlight for valid tile
const uint16_t COL_FLASH_INVALID = 0xF800;  // Red flash for invalid tile

// ==============================================================================
// Sound Functions (Optional PWM Buzzer Support)
// ==============================================================================
#ifdef ENABLE_SOUND

// Initialize LEDC PWM for sound generation
// Call this once in setup()
void initSound() {
    Serial.println("Initializing PWM buzzer...");

    // Configure LEDC timer
    ledcSetup(BUZZER_CHANNEL, 1000, BUZZER_RESOLUTION);  // 1kHz default, 8-bit resolution

    // Attach pin to channel
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    // Start silent
    ledcWrite(BUZZER_CHANNEL, 0);

    Serial.printf("Buzzer ready on GPIO%d (LEDC channel %d)\n", BUZZER_PIN, BUZZER_CHANNEL);
}

// Play a tone at specified frequency for specified duration
// Non-blocking - uses delay() but duration is short (50-200ms)
void playTone(int frequency, int duration) {
    if (frequency > 0) {
        ledcWriteTone(BUZZER_CHANNEL, frequency);
        ledcWrite(BUZZER_CHANNEL, 128);  // 50% duty cycle for volume
        delay(duration);
    }
    ledcWrite(BUZZER_CHANNEL, 0);  // Stop tone
}

// Quick beep for valid tile slide
void playSlideSound() {
    playTone(500, 50);  // 500Hz for 50ms
}

// Low buzz for invalid move attempt
void playErrorSound() {
    playTone(200, 100);  // 200Hz for 100ms
}

// Ascending melody for puzzle completion: C-E-G
void playWinSound() {
    playTone(262, 200);  // C (262Hz)
    delay(50);           // Short gap
    playTone(330, 200);  // E (330Hz)
    delay(50);           // Short gap
    playTone(392, 200);  // G (392Hz)
}

#endif  // ENABLE_SOUND

// ==============================================================================
// ST7701S Manual Initialization (3-Wire SPI)
// ==============================================================================
void st7701_send(uint8_t data, bool is_cmd) {
    digitalWrite(PIN_SPI_CS, LOW);
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

void st7701_write_command(uint8_t c) { st7701_send(c, true); }
void st7701_write_data(uint8_t d) { st7701_send(d, false); }

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
// Helper: Draw a rounded-corner button
// ==============================================================================
void drawButton(int x, int y, int w, int h, uint16_t color, const char* label, uint16_t textColor = COL_WHITE) {
    tft.fillRoundRect(x, y, w, h, 8, color);
    tft.drawRoundRect(x, y, w, h, 8, COL_WHITE);
    tft.setTextColor(textColor);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.drawString(label, x + w / 2, y + h / 2);
}

// ==============================================================================
// Helper: Check if point is inside rectangle
// ==============================================================================
bool inRect(int tx, int ty, int rx, int ry, int rw, int rh) {
    return tx >= rx && tx < rx + rw && ty >= ry && ty < ry + rh;
}

// ==============================================================================
// Load puzzle image into PSRAM buffer
// ==============================================================================
bool loadPuzzleImage(const String& filename) {
    if (puzzleImageBuffer) {
        free(puzzleImageBuffer);
        puzzleImageBuffer = nullptr;
    }

    puzzleImageBuffer = (uint16_t*)ps_malloc(480 * 480 * sizeof(uint16_t));
    if (!puzzleImageBuffer) {
        Serial.println("ERROR: Failed to allocate PSRAM for image!");
        return false;
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.printf("ERROR: Failed to open %s\n", filename.c_str());
        free(puzzleImageBuffer);
        puzzleImageBuffer = nullptr;
        return false;
    }

    size_t fileSize = file.size();
    if (fileSize != 460800) {
        Serial.printf("ERROR: Invalid file size %d\n", fileSize);
        file.close();
        free(puzzleImageBuffer);
        puzzleImageBuffer = nullptr;
        return false;
    }

    // Read entire image into buffer
    size_t bytesRead = file.read((uint8_t*)puzzleImageBuffer, 460800);
    file.close();

    if (bytesRead != 460800) {
        Serial.printf("ERROR: Only read %d bytes\n", bytesRead);
        free(puzzleImageBuffer);
        puzzleImageBuffer = nullptr;
        return false;
    }

    Serial.println("Puzzle image loaded into PSRAM");
    return true;
}

// ==============================================================================
// Get elapsed game time in seconds
// ==============================================================================
unsigned long getGameSeconds() {
    if (!timerRunning && gameStartTime == 0) return 0;
    unsigned long endT = timerRunning ? millis() : gameEndTime;
    return (endT - gameStartTime) / 1000;
}

// ==============================================================================
// Format time as MM:SS
// ==============================================================================
String formatTime(unsigned long seconds) {
    int mins = seconds / 60;
    int secs = seconds % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
    return String(buf);
}

// ==============================================================================
// MAIN MENU
// ==============================================================================
void showMainMenu() {
    gameState = MAIN_MENU;
    tft.fillScreen(COL_MENU_BG);

    // Title
    tft.setTextColor(COL_WHITE);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setTextSize(4);
    tft.drawString("SLIDING", 240, 100);
    tft.drawString("PUZZLE", 240, 155);

    tft.setTextSize(2);
    tft.setTextColor(0xBDF7);
    tft.drawString("Select Difficulty", 240, 220);

    // Difficulty buttons
    int btnW = 300, btnH = 60;
    int btnX = (480 - btnW) / 2;

    tft.setTextSize(3);
    drawButton(btnX, 260, btnW, btnH, COL_BTN_EASY, "EASY  (3x3)", COL_BLACK);
    drawButton(btnX, 340, btnW, btnH, COL_BTN_MED,  "MEDIUM (4x4)", COL_BLACK);
    drawButton(btnX, 420, btnW, btnH, COL_BTN_HARD, "HARD  (5x5)", COL_WHITE);
}

// ==============================================================================
// PUZZLE SELECT
// ==============================================================================
void showPuzzleSelect(int difficulty) {
    gameState = PUZZLE_SELECT;
    selectedDifficulty = difficulty;

    tft.fillScreen(COL_MENU_BG);

    const char* diffNames[] = {"Easy (3x3)", "Medium (4x4)", "Hard (5x5)"};
    const uint16_t diffColors[] = {COL_BTN_EASY, COL_BTN_MED, COL_BTN_HARD};

    // Title
    tft.setTextColor(diffColors[difficulty]);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setTextSize(3);
    tft.drawString(diffNames[difficulty], 240, 30);

    tft.setTextSize(2);
    tft.setTextColor(0xBDF7);
    tft.drawString("Choose a Puzzle", 240, 65);

    // Show 5 puzzle buttons
    const auto& puzzles = puzzleManager.getPuzzles(difficulty);
    int btnW = 420, btnH = 55;
    int btnX = (480 - btnW) / 2;
    int startY = 95;

    tft.setTextSize(2);
    for (int i = 0; i < (int)puzzles.size() && i < 5; i++) {
        int btnY = startY + i * 65;
        char label[64];
        snprintf(label, sizeof(label), "%d. %s", i + 1, puzzles[i].displayName.c_str());
        drawButton(btnX, btnY, btnW, btnH, COL_BTN, label);
    }

    // Back button
    tft.setTextSize(2);
    drawButton(10, 430, 120, 40, 0x8000, "< Back");
}

// ==============================================================================
// Draw flash feedback overlay on a tile
// ==============================================================================
void drawFlashFeedback(int gridPos, int gridSize, int tileSize, int offsetX, int offsetY, uint16_t color) {
    int row = gridPos / gridSize;
    int col = gridPos % gridSize;
    int x = offsetX + col * tileSize;
    int y = offsetY + row * tileSize;

    // Draw a thick border for valid tile (white/yellow)
    if (color == COL_FLASH_VALID) {
        // Draw multiple rectangles for thick border
        for (int i = 0; i < 3; i++) {
            tft.drawRect(x + i, y + i, tileSize - (i * 2), tileSize - (i * 2), color);
        }
    } else {
        // Draw red semi-transparent overlay for invalid tile
        // Since we can't do true transparency in RGB565, we'll draw a red border + corners
        tft.drawRect(x, y, tileSize, tileSize, color);
        tft.drawRect(x + 1, y + 1, tileSize - 2, tileSize - 2, color);

        // Draw diagonal lines in corners for stronger effect
        for (int i = 0; i < 10; i++) {
            tft.drawLine(x + i, y, x, y + i, color);
            tft.drawLine(x + tileSize - 1 - i, y, x + tileSize - 1, y + i, color);
            tft.drawLine(x + i, y + tileSize - 1, x, y + tileSize - 1 - i, color);
            tft.drawLine(x + tileSize - 1 - i, y + tileSize - 1, x + tileSize - 1, y + tileSize - 1 - i, color);
        }
    }
}

// ==============================================================================
// Draw a single tile at its grid position (or custom pixel position)
// ==============================================================================
void drawTile(int tileNum, int gridPos, int gridSize, int tileSize, int offsetX, int offsetY, int customX = -1, int customY = -1) {
    int destX, destY;

    if (customX >= 0 && customY >= 0) {
        // Use custom pixel coordinates (for animation)
        destX = customX;
        destY = customY;
    } else {
        // Use grid position
        int destRow = gridPos / gridSize;
        int destCol = gridPos % gridSize;
        destX = offsetX + destCol * tileSize;
        destY = offsetY + destRow * tileSize;
    }

    if (tileNum == 0) {
        // Empty tile
        tft.fillRect(destX, destY, tileSize, tileSize, COL_EMPTY);
        return;
    }

    if (!puzzleImageBuffer) {
        // Fallback: numbered tile
        tft.fillRect(destX, destY, tileSize, tileSize, COL_BTN);
        tft.drawRect(destX, destY, tileSize, tileSize, COL_WHITE);
        tft.setTextColor(COL_WHITE);
        tft.setTextDatum(textdatum_t::middle_center);
        tft.setTextSize(2);
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", tileNum);
        tft.drawString(buf, destX + tileSize / 2, destY + tileSize / 2);
        return;
    }

    // Calculate source region from the original image
    // Tile N corresponds to position N-1 in the solved puzzle
    int srcRow = (tileNum - 1) / gridSize;
    int srcCol = (tileNum - 1) % gridSize;

    // Source coordinates in the 480x480 image
    // Scale: each tile maps to (480/gridSize) pixels in the source image
    int imgTileSize = 480 / gridSize;
    int srcX = srcCol * imgTileSize;
    int srcY = srcRow * imgTileSize;

    // Draw tile line by line, scaling from image to screen tile size
    // If tileSize == imgTileSize, it's 1:1. Otherwise we need to scale.
    if (tileSize == imgTileSize) {
        // Direct copy, row by row
        for (int row = 0; row < tileSize; row++) {
            tft.pushImage(destX, destY + row, tileSize, 1,
                          &puzzleImageBuffer[(srcY + row) * 480 + srcX]);
        }
    } else {
        // Scale: use nearest-neighbor
        uint16_t* lineBuffer = (uint16_t*)malloc(tileSize * sizeof(uint16_t));
        if (lineBuffer) {
            for (int dy = 0; dy < tileSize; dy++) {
                int sy = srcY + (dy * imgTileSize) / tileSize;
                for (int dx = 0; dx < tileSize; dx++) {
                    int sx = srcX + (dx * imgTileSize) / tileSize;
                    lineBuffer[dx] = puzzleImageBuffer[sy * 480 + sx];
                }
                tft.pushImage(destX, destY + dy, tileSize, 1, lineBuffer);
            }
            free(lineBuffer);
        }
    }

    // Draw thin grid border on tile
    tft.drawRect(destX, destY, tileSize, tileSize, COL_GRID_LINE);
}

// ==============================================================================
// Draw status bar
// ==============================================================================
void drawStatusBar() {
    int moves = puzzle ? puzzle->getMoveCount() : 0;
    unsigned long secs = getGameSeconds();

    // Only redraw if changed
    if ((int)secs == lastDisplayedSeconds && moves == lastDisplayedMoves) return;
    lastDisplayedSeconds = (int)secs;
    lastDisplayedMoves = moves;

    tft.fillRect(0, 0, 480, STATUS_BAR_HEIGHT, COL_BLACK);
    tft.setTextColor(COL_WHITE);
    tft.setTextDatum(textdatum_t::middle_left);
    tft.setTextSize(2);

    char buf[32];
    snprintf(buf, sizeof(buf), "Moves: %d", moves);
    tft.drawString(buf, 10, STATUS_BAR_HEIGHT / 2);

    String timeStr = "Time: " + formatTime(secs);
    tft.setTextDatum(textdatum_t::middle_right);
    tft.drawString(timeStr.c_str(), 470, STATUS_BAR_HEIGHT / 2);
}

// ==============================================================================
// Draw button bar (Restart / Back)
// ==============================================================================
void drawButtonBar() {
    int barY = 480 - BUTTON_BAR_HEIGHT;
    tft.fillRect(0, barY, 480, BUTTON_BAR_HEIGHT, COL_BLACK);

    tft.setTextSize(2);
    drawButton(10, barY + 5, 140, 40, 0x8000, "< Back");
    drawButton(330, barY + 5, 140, 40, COL_BTN_MED, "Restart", COL_BLACK);
}

// ==============================================================================
// Draw full game screen
// ==============================================================================
void drawGameScreen(bool skipAnimatingTile = false) {
    if (!puzzle) return;

    int gridSize = puzzle->getGridSize();
    int tileSize = GAME_AREA_SIZE / gridSize;
    int offsetX = (480 - tileSize * gridSize) / 2;
    int offsetY = GAME_AREA_Y + (GAME_AREA_SIZE - tileSize * gridSize) / 2;

    // Clear game area
    tft.fillRect(0, GAME_AREA_Y, 480, GAME_AREA_SIZE, COL_BG);

    // Draw all tiles
    int totalTiles = gridSize * gridSize;
    for (int pos = 0; pos < totalTiles; pos++) {
        // Skip the animating tile if requested
        if (skipAnimatingTile && isAnimating && pos == animToPos) {
            continue;
        }

        int tileNum = puzzle->getTile(pos);
        drawTile(tileNum, pos, gridSize, tileSize, offsetX, offsetY);
    }

    drawStatusBar();
    drawButtonBar();
}

// ==============================================================================
// Redraw only the two tiles that changed (+ status bar)
// ==============================================================================
void redrawMovedTiles(int pos1, int pos2) {
    if (!puzzle) return;

    int gridSize = puzzle->getGridSize();
    int tileSize = GAME_AREA_SIZE / gridSize;
    int offsetX = (480 - tileSize * gridSize) / 2;
    int offsetY = GAME_AREA_Y + (GAME_AREA_SIZE - tileSize * gridSize) / 2;

    drawTile(puzzle->getTile(pos1), pos1, gridSize, tileSize, offsetX, offsetY);
    drawTile(puzzle->getTile(pos2), pos2, gridSize, tileSize, offsetX, offsetY);

    // Force status bar update
    lastDisplayedMoves = -1;
    drawStatusBar();
}

// ==============================================================================
// START GAME
// ==============================================================================
void startGame(int difficulty, int puzzleIndex) {
    gameState = PLAYING;
    selectedDifficulty = difficulty;
    selectedPuzzle = puzzleIndex;

    const PuzzleInfo& info = puzzleManager.getPuzzle(difficulty, puzzleIndex);
    Serial.printf("Starting game: %s (%dx%d)\n", info.displayName.c_str(), info.gridSize, info.gridSize);

    // Load image
    tft.fillScreen(COL_BLACK);
    tft.setTextColor(COL_WHITE);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setTextSize(2);
    tft.drawString("Loading...", 240, 240);

    if (!loadPuzzleImage(info.filename)) {
        tft.fillScreen(0xF800);
        tft.drawString("Failed to load image!", 240, 240);
        delay(2000);
        showMainMenu();
        return;
    }

    // Create puzzle
    if (puzzle) delete puzzle;
    puzzle = new SlidingPuzzle(info.gridSize);

    // Shuffle based on difficulty
    int shuffleMoves = (info.gridSize == 3) ? 50 : (info.gridSize == 4) ? 150 : 300;
    puzzle->shuffle(shuffleMoves);

    // Reset timer state
    gameStartTime = 0;
    gameEndTime = 0;
    timerRunning = false;
    lastDisplayedSeconds = -1;
    lastDisplayedMoves = -1;

    drawGameScreen();

    Serial.println("Game started!");
    puzzle->printBoard();
}

// ==============================================================================
// Start tile animation
// ==============================================================================
void startTileAnimation(int fromPos, int toPos, int tileNum) {
    isAnimating = true;
    animStartTime = millis();
    animFromPos = fromPos;
    animToPos = toPos;
    animTileNum = tileNum;
}

// ==============================================================================
// Update animation state and draw animating tile
// Returns true if animation is complete
// ==============================================================================
bool updateAnimation() {
    if (!isAnimating || !puzzle) return true;

    unsigned long now = millis();
    unsigned long elapsed = now - animStartTime;

    if (elapsed >= ANIM_DURATION_MS) {
        // Animation complete
        isAnimating = false;
        return true;
    }

    // Calculate interpolation factor (0.0 to 1.0)
    float t = (float)elapsed / (float)ANIM_DURATION_MS;

    int gridSize = puzzle->getGridSize();
    int tileSize = GAME_AREA_SIZE / gridSize;
    int offsetX = (480 - tileSize * gridSize) / 2;
    int offsetY = GAME_AREA_Y + (GAME_AREA_SIZE - tileSize * gridSize) / 2;

    // Calculate from/to positions in pixels
    int fromRow = animFromPos / gridSize;
    int fromCol = animFromPos % gridSize;
    int fromX = offsetX + fromCol * tileSize;
    int fromY = offsetY + fromRow * tileSize;

    int toRow = animToPos / gridSize;
    int toCol = animToPos % gridSize;
    int toX = offsetX + toCol * tileSize;
    int toY = offsetY + toRow * tileSize;

    // Linear interpolation
    int currentX = fromX + (int)((toX - fromX) * t);
    int currentY = fromY + (int)((toY - fromY) * t);

    // Clear previous position trail (draw empty tile behind)
    // We need to clear the path between from and to
    if (fromX == toX) {
        // Vertical movement
        int y1 = min(fromY, toY);
        int y2 = max(fromY, toY) + tileSize;
        tft.fillRect(fromX, y1, tileSize, y2 - y1, COL_EMPTY);
    } else {
        // Horizontal movement
        int x1 = min(fromX, toX);
        int x2 = max(fromX, toX) + tileSize;
        tft.fillRect(x1, fromY, x2 - x1, tileSize, COL_EMPTY);
    }

    // Draw the animating tile at interpolated position
    drawTile(animTileNum, animToPos, gridSize, tileSize, offsetX, offsetY, currentX, currentY);

    return false;
}

// ==============================================================================
// Handle touch during gameplay
// ==============================================================================
void handleGameTouch(int x, int y) {
    if (!puzzle) return;

    // Block touch input during animation
    if (isAnimating) {
        Serial.println("Touch blocked: animation in progress");
        return;
    }

    int gridSize = puzzle->getGridSize();
    int tileSize = GAME_AREA_SIZE / gridSize;
    int offsetX = (480 - tileSize * gridSize) / 2;
    int offsetY = GAME_AREA_Y + (GAME_AREA_SIZE - tileSize * gridSize) / 2;

    // Check button bar
    int barY = 480 - BUTTON_BAR_HEIGHT;
    if (y >= barY) {
        if (inRect(x, y, 10, barY + 5, 140, 40)) {
            // Back button
            Serial.println("Back to puzzle select");
            if (puzzleImageBuffer) { free(puzzleImageBuffer); puzzleImageBuffer = nullptr; }
            if (puzzle) { delete puzzle; puzzle = nullptr; }
            showPuzzleSelect(selectedDifficulty);
            return;
        }
        if (inRect(x, y, 330, barY + 5, 140, 40)) {
            // Restart button
            Serial.println("Restarting puzzle");
            int shuffleMoves = (gridSize == 3) ? 50 : (gridSize == 4) ? 150 : 300;
            puzzle->reset();
            puzzle->shuffle(shuffleMoves);
            gameStartTime = 0;
            gameEndTime = 0;
            timerRunning = false;
            lastDisplayedSeconds = -1;
            lastDisplayedMoves = -1;
            drawGameScreen();
            return;
        }
        return;
    }

    // Check if touch is in grid area
    int gridX = x - offsetX;
    int gridY = y - offsetY;
    if (gridX < 0 || gridY < 0 || gridX >= tileSize * gridSize || gridY >= tileSize * gridSize) {
        return;
    }

    int col = gridX / tileSize;
    int row = gridY / tileSize;
    int tilePos = row * gridSize + col;

    Serial.printf("Touch grid [%d,%d] pos=%d tile=%d\n", row, col, tilePos, puzzle->getTile(tilePos));

    if (puzzle->canMove(tilePos)) {
        int oldEmptyPos = puzzle->getEmptyPos();
        int tileNum = puzzle->getTile(tilePos);

        // Show valid tile feedback (bright border)
        flashTile = tilePos;
        flashStartTime = millis();
        flashColor = COL_FLASH_VALID;
        drawFlashFeedback(tilePos, gridSize, tileSize, offsetX, offsetY, flashColor);

        #ifdef ENABLE_SOUND
        playSlideSound();
        #endif

        // Start timer on first move
        if (!timerRunning && gameStartTime == 0) {
            gameStartTime = millis();
            timerRunning = true;
        }

        // Start animation before moving tile in puzzle state
        startTileAnimation(tilePos, oldEmptyPos, tileNum);

        // Move tile in puzzle state
        puzzle->moveTile(tilePos);

        // Note: Win check will happen after animation completes in loop()
    } else {
        // Invalid tile - show red flash
        Serial.println("Invalid move - tile can't move");
        flashTile = tilePos;
        flashStartTime = millis();
        flashColor = COL_FLASH_INVALID;
        drawFlashFeedback(tilePos, gridSize, tileSize, offsetX, offsetY, flashColor);

        #ifdef ENABLE_SOUND
        playErrorSound();
        #endif
    }
}

// ==============================================================================
// WIN SCREEN
// ==============================================================================
void showWinScreen() {
    gameState = WIN_SCREEN;

    unsigned long secs = getGameSeconds();
    int moves = puzzle ? puzzle->getMoveCount() : 0;

    tft.fillScreen(COL_WIN_BG);

    // Show completed puzzle image briefly behind
    if (puzzleImageBuffer) {
        // Draw small centered preview of solved image
        int previewSize = 200;
        int px = (480 - previewSize) / 2;
        int py = 30;
        uint16_t* lineBuffer = (uint16_t*)malloc(previewSize * sizeof(uint16_t));
        if (lineBuffer) {
            for (int dy = 0; dy < previewSize; dy++) {
                int sy = (dy * 480) / previewSize;
                for (int dx = 0; dx < previewSize; dx++) {
                    int sx = (dx * 480) / previewSize;
                    lineBuffer[dx] = puzzleImageBuffer[sy * 480 + sx];
                }
                tft.pushImage(px, py + dy, previewSize, 1, lineBuffer);
            }
            free(lineBuffer);
        }
        tft.drawRect(px - 1, py - 1, previewSize + 2, previewSize + 2, COL_GOLD);
        tft.drawRect(px - 2, py - 2, previewSize + 4, previewSize + 4, COL_GOLD);
    }

    // "You Win!" text
    tft.setTextColor(COL_GOLD);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setTextSize(4);
    tft.drawString("YOU WIN!", 240, 260);

    // Stats
    tft.setTextSize(2);
    tft.setTextColor(COL_WHITE);
    char buf[64];
    snprintf(buf, sizeof(buf), "Moves: %d    Time: %s", moves, formatTime(secs).c_str());
    tft.drawString(buf, 240, 310);

    // Difficulty label
    const char* diffNames[] = {"Easy (3x3)", "Medium (4x4)", "Hard (5x5)"};
    tft.setTextSize(2);
    tft.setTextColor(0xBDF7);
    tft.drawString(diffNames[selectedDifficulty], 240, 345);

    // Buttons
    tft.setTextSize(2);
    drawButton(50, 400, 170, 50, COL_BTN_MED, "Play Again", COL_BLACK);
    drawButton(260, 400, 170, 50, COL_BTN, "Menu");
}

// ==============================================================================
// Handle touch on win screen
// ==============================================================================
void handleWinTouch(int x, int y) {
    if (inRect(x, y, 50, 400, 170, 50)) {
        // Play Again - same puzzle
        startGame(selectedDifficulty, selectedPuzzle);
    } else if (inRect(x, y, 260, 400, 170, 50)) {
        // Menu
        if (puzzleImageBuffer) { free(puzzleImageBuffer); puzzleImageBuffer = nullptr; }
        if (puzzle) { delete puzzle; puzzle = nullptr; }
        showMainMenu();
    }
}

// ==============================================================================
// Handle touch on main menu
// ==============================================================================
void handleMenuTouch(int x, int y) {
    int btnW = 300, btnH = 60;
    int btnX = (480 - btnW) / 2;

    if (inRect(x, y, btnX, 260, btnW, btnH)) {
        showPuzzleSelect(0);  // Easy
    } else if (inRect(x, y, btnX, 340, btnW, btnH)) {
        showPuzzleSelect(1);  // Medium
    } else if (inRect(x, y, btnX, 420, btnW, btnH)) {
        showPuzzleSelect(2);  // Hard
    }
}

// ==============================================================================
// Handle touch on puzzle select
// ==============================================================================
void handlePuzzleSelectTouch(int x, int y) {
    // Back button
    if (inRect(x, y, 10, 430, 120, 40)) {
        showMainMenu();
        return;
    }

    // Puzzle buttons
    int btnW = 420, btnH = 55;
    int btnX = (480 - btnW) / 2;
    int startY = 95;

    const auto& puzzles = puzzleManager.getPuzzles(selectedDifficulty);
    for (int i = 0; i < (int)puzzles.size() && i < 5; i++) {
        int btnY = startY + i * 65;
        if (inRect(x, y, btnX, btnY, btnW, btnH)) {
            startGame(selectedDifficulty, i);
            return;
        }
    }
}

// ==============================================================================
// Setup
// ==============================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n========================================");
    Serial.println("  Sliding Puzzle Game");
    Serial.println("  ESP32-4848S040C_I");
    Serial.println("========================================");

    Serial.printf("PSRAM Size: %d bytes (%.2f MB)\n",
                  ESP.getPsramSize(), ESP.getPsramSize() / 1024.0 / 1024.0);
    Serial.printf("Free PSRAM: %d bytes (%.2f MB)\n",
                  ESP.getFreePsram(), ESP.getFreePsram() / 1024.0 / 1024.0);

    // 1. ST7701S init
    run_init_sequence();

    // 2. LGFX init
    Serial.println("Initializing TFT...");
    tft.init();
    tft.setBrightness(255);
    tft.fillScreen(TFT_BLACK);

    // Seed random
    randomSeed(analogRead(0) ^ millis());

    // 3. PuzzleManager init
    Serial.println("Initializing PuzzleManager...");
    if (!puzzleManager.init()) {
        Serial.println("ERROR: PuzzleManager initialization failed!");
        tft.fillScreen(TFT_RED);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.setTextDatum(textdatum_t::middle_center);
        tft.drawString("FILESYSTEM ERROR", 240, 200);
        tft.setTextSize(1);
        tft.drawString("Run: pio run --target uploadfs", 240, 240);
        while (1) delay(1000);
    }

    puzzleManager.listFiles();

    // 4. Initialize sound (if enabled)
    #ifdef ENABLE_SOUND
    initSound();
    #endif

    // 5. Show main menu
    showMainMenu();

    Serial.println("Setup Complete!");
}

// ==============================================================================
// Loop
// ==============================================================================
void loop() {
    int32_t x, y;
    bool touching = tft.getTouch(&x, &y);
    unsigned long now = millis();

    // Update animation if in progress
    if (gameState == PLAYING && isAnimating) {
        bool animComplete = updateAnimation();

        if (animComplete) {
            // Animation finished - redraw tiles at final positions
            isAnimating = false;

            // Redraw both positions to ensure clean final state
            // animFromPos now has empty tile, animToPos now has the moved tile
            redrawMovedTiles(animFromPos, animToPos);

            // Check for win condition
            if (puzzle && puzzle->isWon()) {
                timerRunning = false;
                gameEndTime = millis();
                Serial.println("PUZZLE SOLVED!");

                #ifdef ENABLE_SOUND
                playWinSound();
                #endif

                delay(500);  // Brief pause before win screen
                showWinScreen();
            }
        }
    }

    // Clear flash feedback after duration
    if (gameState == PLAYING && flashTile >= 0 && (now - flashStartTime >= FLASH_DURATION_MS)) {
        // Redraw the tile to clear flash effect
        if (puzzle && !isAnimating) {
            int gridSize = puzzle->getGridSize();
            int tileSize = GAME_AREA_SIZE / gridSize;
            int offsetX = (480 - tileSize * gridSize) / 2;
            int offsetY = GAME_AREA_Y + (GAME_AREA_SIZE - tileSize * gridSize) / 2;

            // Only redraw if the tile is still in the same position (not animating)
            drawTile(puzzle->getTile(flashTile), flashTile, gridSize, tileSize, offsetX, offsetY);
        }
        flashTile = -1;
    }

    // Touch press detection with debounce (not during animation)
    if (touching && !lastTouchState && (now - lastTouchTime > TOUCH_DEBOUNCE_MS)) {
        lastTouchTime = now;
        Serial.printf("Touch at (%d, %d) state=%d\n", x, y, (int)gameState);

        switch (gameState) {
            case MAIN_MENU:
                handleMenuTouch(x, y);
                break;
            case PUZZLE_SELECT:
                handlePuzzleSelectTouch(x, y);
                break;
            case PLAYING:
                handleGameTouch(x, y);
                break;
            case WIN_SCREEN:
                handleWinTouch(x, y);
                break;
        }
    }

    lastTouchState = touching;

    // Update timer display during gameplay (but not during animation to avoid flicker)
    if (gameState == PLAYING && timerRunning && !isAnimating) {
        drawStatusBar();
    }

    delay(10);
}
