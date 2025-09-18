@echo off
echo Building UserSVC...

if not exist build mkdir build

g++ -std=c++17 -I. -o build/user_service.exe src/main.cpp src/services/user_service.cpp -lws2_32

if %ERRORLEVEL% == 0 (
    echo Build successful!
) else (
    echo Build failed!
)