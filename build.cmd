:<<"::CMDGOTO"
@echo off
goto :CMDENTRY
rem https://stackoverflow.com/questions/17510688/single-script-to-run-in-both-windows-batch-and-linux-bash
::CMDGOTO

echo "==========  TRIP build ${SHELL} ================="
DIR=$(dirname "$0")
(mkdir -p ${DIR}/bin;)
(cd ${DIR}/bin;cc ../src/main.c -o trip)
(cd ${DIR}/bin;./trip -s ../samples/edit.3p)
exit $?
:CMDENTRY

echo ============= TRIP build %COMSPEC% ============
set OLDDIR=%CD%

mkdir bin  >nul 2>&1
chdir /d bin
if "%CD%" == "%OLDDIR%" (
	echo dont build in source tree! 
	exit 1
)
del *.log trip.exe
cl %~dp0\src\main.c /D_MBCS=1 User32.lib /D_CRT_SECURE_NO_WARNINGS=1 /Fe:trip.exe >build.log 2>&1
if "%errorlevel%"=="0" goto vsok
clang -D_CRT_SECURE_NO_WARNINGS=1 -D_MBCS=1 %~dp0\src\main.c -o trip.exe >buildclang.log 2>&1
if "%errorlevel%"=="0" goto vsok
type build.log
:vsok

echo build %1 %2
.\trip.exe -s ../samples/edit.3p %1 %2

chdir /d %OLDDIR%
exit 0


