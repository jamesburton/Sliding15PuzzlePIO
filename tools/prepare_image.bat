@echo off
REM Windows batch file for easy image preparation
REM Usage: prepare_image.bat your_photo.jpg

echo ================================================
echo   Puzzle Image Preparation Tool
echo   ESP32-4848S040C_I Sliding Puzzle Game
echo ================================================
echo.

if "%~1"=="" (
    echo ERROR: No input file specified
    echo.
    echo Usage: prepare_image.bat ^<input_image^> [output_name] [options]
    echo.
    echo Examples:
    echo   prepare_image.bat photo.jpg
    echo   prepare_image.bat photo.jpg my_puzzle
    echo   prepare_image.bat photo.jpg --tiles 5
    echo.
    pause
    exit /b 1
)

if not exist "%~1" (
    echo ERROR: File not found: %~1
    echo.
    pause
    exit /b 1
)

REM Check if Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found!
    echo Please install Python from https://www.python.org/
    echo.
    pause
    exit /b 1
)

REM Check if required packages are installed
python -c "import PIL" >nul 2>&1
if errorlevel 1 (
    echo Installing required packages...
    pip install -r requirements.txt
    if errorlevel 1 (
        echo ERROR: Failed to install required packages
        pause
        exit /b 1
    )
)

echo Processing: %~1
echo.

REM Run the Python script with all arguments
python prepare_puzzle_image.py %*

echo.
echo ================================================
echo Done! Check the 'output' folder for results.
echo ================================================
pause
