@echo off
echo Compiling ServiceSVC...

if not exist "build" mkdir build

g++ -std=c++17 ^
    src/servicesvc_main.cpp ^
    src/services/service_handler.cpp ^
    src/utils/database_manager.cpp ^
    src/utils/logger.cpp ^
    -I./include ^
    -I../third_party ^
    -L../third_party/sqlite ^
    -lsqlite3 ^
    -lws2_32 ^
    -o build/servicesvc.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    exit /b %ERRORLEVEL%
)

echo ServiceSVC compiled successfully!
echo Executable created at build/servicesvc.exe