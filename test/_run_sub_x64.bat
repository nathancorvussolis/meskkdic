@echo off
pushd %~dp0
setlocal

rem x64 �e�X�g

set MESKKDIC="..\x64\Release\meskkdic.exe"
set OUTDIR=x64

call _run_sub.bat

endlocal
popd
