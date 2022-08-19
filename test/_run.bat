@echo off
pushd %~dp0
setlocal

set DICDIR=obj
if not exist %DICDIR% (
  mkdir %DICDIR%
)

pushd %DICDIR%

echo Downloading...

curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.L" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.JIS2" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.JIS3_4" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.JIS2004" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.hukugougo" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.assoc" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.itaiji" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.itaiji.JIS3_4" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.station" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.geo" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.okinawa" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.china_taiwan" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.jinmei" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.fullname" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.propernoun" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.law" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.edict" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.lisp" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/zipcode/SKK-JISYO.zipcode" > nul
curl -sS -L -O "https://github.com/skk-dev/dict/raw/master/zipcode/SKK-JISYO.office.zipcode" > nul

rem UTF-8
wsget "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.edict2" > nul
wsget "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.emoji" > nul
wsget "https://github.com/skk-dev/dict/raw/master/SKK-JISYO.ivd" > nul

popd


echo.
echo ======== x86 テスト ========
call _run_sub_x86.bat

echo.
echo ======== x64 テスト ========
call _run_sub_x64.bat


endlocal
popd

echo.
pause
