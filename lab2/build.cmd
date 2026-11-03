@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
where mingw32-make >nul 2>nul
if errorlevel 1 (
    make -B
) else (
    mingw32-make -B
)
if errorlevel 1 exit /b 1
build\main.exe
exit /b %errorlevel%
