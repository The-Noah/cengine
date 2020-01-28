@echo off

set config=%1

if "%config%"=="" (
  set config=debug
)

premake5 gmake2
mingw32-make config=%config%