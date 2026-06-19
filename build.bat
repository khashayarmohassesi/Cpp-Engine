@echo off

mkdir ..\build
pushd ..\build
cl -Zi  "..\code\handmade_win.cpp" user32.lib Gdi32.lib
popd
