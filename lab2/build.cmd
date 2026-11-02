@echo off
setlocal
cd /d "%~dp0"
where cmake >nul 2>nul
if errorlevel 1 if exist "C:\msys64\ucrt64\bin\cmake.exe" set "PATH=C:\msys64\ucrt64\bin;%PATH%"
cmake -S . -B build
if errorlevel 1 exit /b 1
cmake --build build --config Debug
if errorlevel 1 exit /b 1
ctest --test-dir build -C Debug --output-on-failure
exit /b %errorlevel%
