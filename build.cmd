@echo off
setlocal
cd /d "%~dp0"

echo === Updating project from Git ===
git pull --ff-only origin main
if errorlevel 1 exit /b 1

echo === Configuring CMake ===
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
if errorlevel 1 exit /b 1

echo === Building project ===
cmake --build build
if errorlevel 1 exit /b 1

echo === Running Hello World ===
build\hello.exe
if errorlevel 1 exit /b 1

echo === Done ===