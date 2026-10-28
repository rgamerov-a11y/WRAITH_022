@echo off
setlocal

where cmake >nul 2>nul
if errorlevel 1 (
  echo ERROR: CMake was not found. Install CMake and Visual Studio 2022 C++ tools.
  exit /b 1
)

cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b %errorlevel%

cmake --build build --config Release --target WRAITH_VST3 --parallel
if errorlevel 1 exit /b %errorlevel%

echo.
echo Build complete. Looking for WRAITH.vst3...
if exist "build\WRAITH_artefacts\Release\VST3\WRAITH.vst3" (
  echo Found: build\WRAITH_artefacts\Release\VST3\WRAITH.vst3
) else (
  echo Build finished but the expected plugin bundle path was not found.
  echo Search the build folder for WRAITH.vst3.
  exit /b 2
)
