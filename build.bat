@echo off

set config=%1

if "%config%"=="" (
  set config=debug
)

premake5 gmake
mingw32-make config=%config%