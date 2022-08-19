@echo off
pushd %~dp0
setlocal enabledelayedexpansion

if not defined MESKKDIC exit /b 1
if not defined OUTDIR exit /b 1
if not defined DICDIR exit /b 1

if exist %OUTDIR% (
  rd /s /q %OUTDIR%
)
mkdir %OUTDIR%
pushd %OUTDIR%
del /s /q *
popd



echo {

%MESKKDIC% ^
   "%DICDIR%\SKK-JISYO.L" ^
 + "%DICDIR%\SKK-JISYO.JIS2" ^
 + "%DICDIR%\SKK-JISYO.JIS3_4" ^
 + "%DICDIR%\SKK-JISYO.JIS2004" ^
 + "%DICDIR%\SKK-JISYO.hukugougo" ^
 + "%DICDIR%\SKK-JISYO.assoc" ^
 + "%DICDIR%\SKK-JISYO.itaiji" ^
 + "%DICDIR%\SKK-JISYO.itaiji.JIS3_4" ^
 + "%DICDIR%\SKK-JISYO.station" ^
 + "%DICDIR%\SKK-JISYO.geo" ^
 + "%DICDIR%\SKK-JISYO.okinawa" ^
 + "%DICDIR%\SKK-JISYO.china_taiwan" ^
 + "%DICDIR%\SKK-JISYO.jinmei" ^
 + "%DICDIR%\SKK-JISYO.fullname" ^
 + "%DICDIR%\SKK-JISYO.propernoun" ^
 + "%DICDIR%\SKK-JISYO.law" ^
 + "%DICDIR%\SKK-JISYO.edict" ^
 + "%DICDIR%\SKK-JISYO.lisp" ^
 + "%DICDIR%\SKK-JISYO.zipcode" ^
 + "%DICDIR%\SKK-JISYO.office.zipcode" ^
"%OUTDIR%\e.txt"

%MESKKDIC% "%OUTDIR%\e.txt" "%OUTDIR%\ee.txt"

cveuc.exe -e -W "%DICDIR%\SKK-JISYO.L"              "%OUTDIR%\SKK-JISYO-W.L"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.JIS2"           "%OUTDIR%\SKK-JISYO-W.JIS2"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.JIS3_4"         "%OUTDIR%\SKK-JISYO-W.JIS3_4"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.JIS2004"        "%OUTDIR%\SKK-JISYO-W.JIS2004"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.hukugougo"      "%OUTDIR%\SKK-JISYO-W.hukugougo"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.assoc"          "%OUTDIR%\SKK-JISYO-W.assoc"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.itaiji"         "%OUTDIR%\SKK-JISYO-W.itaiji"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.itaiji.JIS3_4"  "%OUTDIR%\SKK-JISYO-W.itaiji.JIS3_4"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.station"        "%OUTDIR%\SKK-JISYO-W.station"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.geo"            "%OUTDIR%\SKK-JISYO-W.geo"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.okinawa"        "%OUTDIR%\SKK-JISYO-W.okinawa"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.china_taiwan"   "%OUTDIR%\SKK-JISYO-W.china_taiwan"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.jinmei"         "%OUTDIR%\SKK-JISYO-W.jinmei"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.fullname"       "%OUTDIR%\SKK-JISYO-W.fullname"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.propernoun"     "%OUTDIR%\SKK-JISYO-W.propernoun"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.law"            "%OUTDIR%\SKK-JISYO-W.law"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.edict"          "%OUTDIR%\SKK-JISYO-W.edict"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.lisp"           "%OUTDIR%\SKK-JISYO-W.lisp"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.zipcode"        "%OUTDIR%\SKK-JISYO-W.zipcode"
cveuc.exe -e -W "%DICDIR%\SKK-JISYO.office.zipcode" "%OUTDIR%\SKK-JISYO-W.office.zipcode"

%MESKKDIC% -W ^
   "%OUTDIR%\SKK-JISYO-W.L" ^
 + "%OUTDIR%\SKK-JISYO-W.JIS2" ^
 + "%OUTDIR%\SKK-JISYO-W.JIS3_4" ^
 + "%OUTDIR%\SKK-JISYO-W.JIS2004" ^
 + "%OUTDIR%\SKK-JISYO-W.hukugougo" ^
 + "%OUTDIR%\SKK-JISYO-W.assoc" ^
 + "%OUTDIR%\SKK-JISYO-W.itaiji" ^
 + "%OUTDIR%\SKK-JISYO-W.itaiji.JIS3_4" ^
 + "%OUTDIR%\SKK-JISYO-W.station" ^
 + "%OUTDIR%\SKK-JISYO-W.geo" ^
 + "%OUTDIR%\SKK-JISYO-W.okinawa" ^
 + "%OUTDIR%\SKK-JISYO-W.china_taiwan" ^
 + "%OUTDIR%\SKK-JISYO-W.jinmei" ^
 + "%OUTDIR%\SKK-JISYO-W.fullname" ^
 + "%OUTDIR%\SKK-JISYO-W.propernoun" ^
 + "%OUTDIR%\SKK-JISYO-W.law" ^
 + "%OUTDIR%\SKK-JISYO-W.edict" ^
 + "%OUTDIR%\SKK-JISYO-W.lisp" ^
 + "%OUTDIR%\SKK-JISYO-W.zipcode" ^
 + "%OUTDIR%\SKK-JISYO-W.office.zipcode" ^
"%OUTDIR%\w.txt"

cveuc.exe -w -E "%OUTDIR%\w.txt" "%OUTDIR%\we.txt"
%MESKKDIC% "%OUTDIR%\we.txt" "%OUTDIR%\wem.txt"

pushd "%OUTDIR%"

set ERRORCOUNT=0
for %%f in ( ee.txt ) do (
  fc /n e.txt %%f > nul
  if !ERRORLEVEL! neq 0 (
    set /a ERRORCOUNT = !ERRORCOUNT! + 1
    echo Failed without "-W"
  )
)
for %%f in ( wem.txt ) do (
  fc /n e.txt %%f > nul
  if !ERRORLEVEL! neq 0 (
    set /a ERRORCOUNT = !ERRORCOUNT! + 1
    echo Failed with "-W"
  )
)
if !ERRORCOUNT! equ 0 (
  echo Succeeded.
) else (
  echo Failed.
)

popd

echo |

%MESKKDIC% "%DICDIR%\SKK-JISYO.L" "%OUTDIR%\e-l.txt"
%MESKKDIC% ^
   "%OUTDIR%\e-l.txt" ^
 + "%DICDIR%\SKK-JISYO.zipcode" ^
 + "%DICDIR%\SKK-JISYO.office.zipcode" ^
"%OUTDIR%\e-lz.txt"
%MESKKDIC% ^
   "%OUTDIR%\e-lz.txt" ^
 - "%DICDIR%\SKK-JISYO.zipcode" ^
 - "%DICDIR%\SKK-JISYO.office.zipcode" ^
"%OUTDIR%\e-lzm.txt"

%MESKKDIC% -W "%OUTDIR%\SKK-JISYO-W.L" "%OUTDIR%\w-l.txt"
%MESKKDIC% -W ^
   "%OUTDIR%\w-l.txt" ^
 + "%OUTDIR%\SKK-JISYO-W.zipcode" ^
 + "%OUTDIR%\SKK-JISYO-W.office.zipcode" ^
"%OUTDIR%\w-lz.txt"
%MESKKDIC% -W ^
   "%OUTDIR%\w-lz.txt" ^
 - "%OUTDIR%\SKK-JISYO-W.zipcode" ^
 - "%OUTDIR%\SKK-JISYO-W.office.zipcode" ^
"%OUTDIR%\w-lzm.txt"

pushd "%OUTDIR%"

set ERRORCOUNT=0
for %%f in ( e-lzm.txt ) do (
  fc /n e-l.txt %%f > nul
  if !ERRORLEVEL! neq 0 (
    set /a ERRORCOUNT = !ERRORCOUNT! + 1
    echo Failed without "-W"
  )
)
for %%f in ( w-lzm.txt ) do (
  fc /n /u w-l.txt %%f > nul
  if !ERRORLEVEL! neq 0 (
    set /a ERRORCOUNT = !ERRORCOUNT! + 1
    echo Failed with "-W"
  )
)
if !ERRORCOUNT! equ 0 (
  echo Succeeded.
) else (
  echo Failed.
)

popd



endlocal
popd
