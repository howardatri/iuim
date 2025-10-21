@echo off
echo Building Enhanced GroupSVC...

if not exist "build" mkdir build

g++ -std=c++17 ^
    src/groupsvc_main.cpp ^
    src/services/group_handler.cpp ^
    src/services/group_controller.cpp ^
    src/services/group_service.cpp ^
    src/services/group_repository.cpp ^
    src/services/group_factory.cpp ^
    src/base/basegroupZHY.cpp ^
    src/models/qqgroupZHY.cpp ^
    src/models/wechatgroupZHY.cpp ^
    src/models/weibogroupZHY.cpp ^
    src/utils/database_manager.cpp ^
    src/utils/group_db_manager.cpp ^
    src/utils/logger.cpp ^
    -I./include ^
    -I../third_party ^
    -L../third_party/sqlite ^
    -lsqlite3 ^
    -lws2_32 ^
    -o build/groupsvc.exe

if %ERRORLEVEL% EQU 0 (
    echo Enhanced GroupSVC build successful! Executable created at build/groupsvc.exe
) else (
    echo Enhanced GroupSVC build failed with error code %ERRORLEVEL%
)