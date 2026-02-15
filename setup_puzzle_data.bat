@echo off
REM Setup Puzzle Images for LittleFS Upload
REM This script copies puzzle images to the data directory for ESP32 upload

echo ================================================
echo   Sliding Puzzle Image Setup
echo   ESP32-4848S040C_I
echo ================================================
echo.

SET SOURCE_DIR=images\sliding_puzzle_images_with_originals_480x480_final
SET DATA_DIR=data\puzzles

echo Creating data directory structure...
if not exist data mkdir data
if not exist %DATA_DIR% mkdir %DATA_DIR%
if not exist %DATA_DIR%\easy mkdir %DATA_DIR%\easy
if not exist %DATA_DIR%\medium mkdir %DATA_DIR%\medium
if not exist %DATA_DIR%\hard mkdir %DATA_DIR%\hard

echo.
echo Copying puzzle images...
echo.

echo [1/3] Copying EASY puzzles (3x3 grid)...
xcopy "%SOURCE_DIR%\Easy\*.png" "%DATA_DIR%\easy\" /Y /Q
if errorlevel 1 (
    echo ERROR: Failed to copy Easy puzzles
    pause
    exit /b 1
)
echo   ✓ Easy puzzles copied

echo.
echo [2/3] Copying MEDIUM puzzles (4x4 grid)...
xcopy "%SOURCE_DIR%\Medium\*.png" "%DATA_DIR%\medium\" /Y /Q
if errorlevel 1 (
    echo ERROR: Failed to copy Medium puzzles
    pause
    exit /b 1
)
echo   ✓ Medium puzzles copied

echo.
echo [3/3] Copying HARD puzzles (5x5 grid)...
xcopy "%SOURCE_DIR%\Hard\*.png" "%DATA_DIR%\hard\" /Y /Q
if errorlevel 1 (
    echo ERROR: Failed to copy Hard puzzles
    pause
    exit /b 1
)
echo   ✓ Hard puzzles copied

echo.
echo ================================================
echo   SUCCESS! Data directory prepared
echo ================================================
echo.
echo Data directory structure:
tree /F data\puzzles
echo.
echo ================================================
echo   Next Steps:
echo ================================================
echo.
echo 1. Build the project:
echo    pio run
echo.
echo 2. Upload filesystem to ESP32:
echo    pio run --target uploadfs
echo.
echo 3. Upload firmware:
echo    pio run --target upload
echo.
echo 4. Monitor serial output:
echo    pio device monitor
echo.
pause
