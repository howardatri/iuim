@echo off
echo Compiling UserSVC...

if not exist "build" mkdir build

g++ -std=c++17 ^
    src/usersvc_main.cpp ^
    src/services/user_service.cpp ^
    src/services/user_handler.cpp ^
    src/services/user_controller.cpp ^
    src/utils/database_manager.cpp ^
    src/utils/logger.cpp ^
    -I./include ^
    -I../third_party ^
    -L../third_party/sqlite ^
    -lsqlite3 ^
    -lws2_32 ^
    -o build/usersvc.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    exit /b %ERRORLEVEL%
)

echo UserSVC compiled successfully!
echo Executable created at build/usersvc.exe