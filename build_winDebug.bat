@echo off
set SCRIPT_DIR=%~dp0
set SRC_DIR=%~dp0
cd %SRC_DIR%
rd /s /q %SRC_DIR%\output\win\Debug
rd /s /q %SRC_DIR%\build
rem "%1 is used by github action for not copy the vendors"
cmake -B build %SRC_DIR%  -DCMAKE_GENERATOR_TOOLSET=v142,host=x64  ^
                 -G"Visual Studio 16 2019" -A "x64"        ^
                 -DCMAKE_SYSTEM_VERSION=10.0.18362.0       ^
                 -Djinja2cpp_DIR=..\Jinja2Cpp\output\lib\jinja2cpp ^
				 %1 ^
				 %2 ^
				 %3 ^
				 %4 ^
				 %5 ^
                 -DCMAKE_BUILD_TYPE=Debug
cmake --build %SRC_DIR%/build --target clean
cmake --build %SRC_DIR%/build --config Debug --target install -j 12
pause