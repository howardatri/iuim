@echo off
echo Building FriendSVC...

if not exist "build" mkdir build

g++ -std=c++17 ^
    src/friendsvc_main.cpp ^
    src/services/friend_handler.cpp ^
    src/services/friend_controller.cpp ^
    src/utils/database_manager.cpp ^
    src/utils/logger.cpp ^
    -I./include ^
    -I../third_party ^
    -L../third_party/sqlite ^
    -lsqlite3 ^
    -lws2_32 ^
    -o build/friendsvc.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Executable created at build/friendsvc.exe
) else (
    echo Build failed with error code %ERRORLEVEL%
)