@echo off
pushd %~dp0
setlocal

rem x86 ƒeƒXƒg

set MESKKDIC="..\Win32\Release\meskkdic.exe"
set OUTDIR=x86

call _run_sub.bat

endlocal
popd
