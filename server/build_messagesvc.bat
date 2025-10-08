@echo off
echo Building MsgSVC...

if not exist "build" mkdir build

g++ -std=c++17 ^
    src/messagesvc_main.cpp ^
    src/services/message_handler.cpp ^
    src/utils/database_manager.cpp ^
    src/utils/logger.cpp ^
    -I./include ^
    -I../third_party ^
    -L../third_party/sqlite ^
    -lsqlite3 ^
    -lws2_32 ^
    -o build/messagesvc.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Executable created at build/messagesvc.exe
) else (
    echo Build failed with error code %ERRORLEVEL%
)