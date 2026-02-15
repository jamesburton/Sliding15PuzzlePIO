# Quick Start Guide
## Get Your Puzzle Game Running in 5 Minutes!

---

## Prerequisites

- [x] ESP32-4848S040C_I board
- [x] USB cable
- [x] PlatformIO installed

---

## 3 Simple Steps

### Step 1: Upload Puzzle Images (1 minute)

```bash
cd C:\Development\PlatformIO\Sliding15PuzzlePIO
pio run --target uploadfs
```

**Wait for:** `Wrote 8388608 bytes...`

⚠️ **CRITICAL:** This MUST be done before uploading firmware!

---

### Step 2: Upload Firmware (1 minute)

```bash
pio run --target upload
```

**Wait for:** `Hard resetting via RTS pin...`

---

### Step 3: Test! (30 seconds)

```bash
pio device monitor
```

**Look for:**
```
All puzzle files verified ✓
Setup Complete!
```

**Touch the screen** - Puzzles should cycle!

---

## Troubleshooting

### "Serial port not found"
- Reconnect USB cable
- Try different USB port
- Install drivers: CH340 or CP2102

### "LittleFS mount failed"
```bash
# Re-upload filesystem
pio run --target uploadfs
```

### "Files found: 0 / 15"
**You forgot Step 1!**
```bash
pio run --target uploadfs  # ← Run this first!
```

### Display blank
- Check power (needs 5V, 1A+)
- Press RESET button
- Check serial output for errors

---

## Success Indicators

✅ **Serial Monitor Shows:**
```
Puzzle lists initialized:
  Easy: 5 puzzles (3x3)
  Medium: 5 puzzles (4x4)
  Hard: 5 puzzles (5x5)
Files found: 15 / 15
All puzzle files verified ✓
```

✅ **Display Shows:**
- Welcome screen: "SLIDING PUZZLE"
- "15 Puzzles Loaded!"
- "Touch to start demo..."

✅ **Touch Works:**
- Each touch cycles to next puzzle
- 15 different images total
- Smooth transitions

---

## What's Next?

**Current Status:** Demo Mode (cycles through images)

**To Implement Full Game:**
1. See `INSTRUCTIONS.md` for game logic
2. Implement tile splitting and shuffling
3. Add touch-to-move mechanics
4. Create menu system

---

## One-Command Test

```bash
cd C:\Development\PlatformIO\Sliding15PuzzlePIO && pio run --target uploadfs && pio run --target upload && pio device monitor
```

**This does all 3 steps in one command!**

---

## Files Overview

```
Sliding15PuzzlePIO/
├── data/puzzles/      ← 15 puzzle images (will upload to ESP32)
├── src/main.cpp       ← Demo application (working now!)
├── platformio.ini     ← Build configuration (ready)
└── README.md          ← Full documentation
```

---

## Expected Timeline

| Step | Time | Command |
|------|------|---------|
| Upload Images | 2 min | `pio run --target uploadfs` |
| Upload Firmware | 1 min | `pio run --target upload` |
| Test | 30 sec | Touch screen! |
| **Total** | **~4 min** | |

---

## Support

**Check Serial Output:**
```bash
pio device monitor
```

Look for error messages and verify:
- PSRAM detected (~8 MB)
- LittleFS mounted
- Files found: 15 / 15
- ST7701 init complete

**Re-upload if Issues:**
```bash
# Clean build
pio run --target clean

# Upload filesystem again
pio run --target uploadfs

# Upload firmware again
pio run --target upload
```

---

## Demo Controls

- **Touch:** Cycle to next puzzle
- **Auto-cycles through:** Easy → Medium → Hard → Easy...
- **Total puzzles:** 15
- **Display:** Shows puzzle name and difficulty

---

## Ready? Let's Go!

```bash
cd C:\Development\PlatformIO\Sliding15PuzzlePIO
pio run --target uploadfs
pio run --target upload
pio device monitor
```

**Touch the screen and enjoy 15 beautiful puzzles!** 🎉

---

*Questions? See README.md for full documentation.*
