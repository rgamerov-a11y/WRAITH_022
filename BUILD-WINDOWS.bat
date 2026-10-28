@echo off
setlocal
echo === WRAITH 0.4 Windows x64 build ===
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto failed
cmake --build build --config Release --target WRAITH_VST3 --parallel
if errorlevel 1 goto failed
echo.
echo Build finished. Find WRAITH.vst3 in build\WRAITH_artefacts\Release\VST3
goto end
:failed
echo.
echo Build failed. Read the error messages above.
exit /b 1
:end
endlocal
