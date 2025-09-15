@echo off
chcp 65001>nul
echo Compiling IUIM server...

if not exist "build\bin" (
    mkdir "build\bin"
)

g++ -std=c++11 ^
    src\main.cpp ^
    src\base\UserBase.cpp ^
    src\base\PlatformManager.cpp ^
    src\utils\NetworkServer.cpp ^
    -I include ^
    -I ..\third_party ^
    -L ..\third_party\sqlite ^
    -lsqlite3 ^
    -lws2_32 ^
    -o build\bin\iuim_server.exe

if %ERRORLEVEL% == 0 (
    echo Compilation successful!
    echo Executable at: build\bin\iuim_server.exe
) else (
    echo Compilation failed!
)