
@echo off
REM %comspec% /k ""C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"" x86

rem capture all output from batch file like this...
rem win32_draco_regression.bat > mylog.txt 2>&1

rem In Task Scheduler, create to actions:
rem 1. d:\cdash\draco\regression\update_regression_dir.bat
rem 2. c:\windows\system32\cmd.exe /k ""d:\cdash\draco\regression\win32_draco_regression.bat"" x86

@echo off
if "%1" == "" goto x86
if not "%2" == "" goto usage

if /i %1 == x86       goto x86
if /i %1 == amd64     goto amd64
if /i %1 == x64       goto amd64
if /i %1 == arm       goto arm
if /i %1 == x86_arm   goto x86_arm
if /i %1 == x86_amd64 goto x86_amd64
goto usage

:x86
if not exist "%vsloc%\%vsenvbat%" goto missing
call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat" x86
goto :vendorsetup

:usage
echo Error in script usage. The correct usage is:
echo     %0 [option]
echo where [option] is: x86 ^| amd64 ^| arm ^| x86_amd64 ^| x86_arm
echo:
echo For example:
echo     %0 x86_amd64
goto :eof

:missing
echo The specified configuration type is missing.  The tools for the
echo configuration might not be installed.
goto :eof

:vendorsetup
set VENDOR_DIR=d:\work\vendors
set GSL_INC_DIR=%VENDOR_DIR%\gsl-1.16\include
set GSL_LIB_DIR=%VENDOR_DIR%\gsl-1.16\lib
set QTDIR=c:/Qt/5.3/msvc2013
set SVN_SSH=c:\\Program Files\\TortoiseSVN\\bin\\TortoisePlink.exe

:cdash
rem set dashboard_type=Experimental
set dashboard_type=Nightly
set base_dir=d:\cdash
set comp=cl
set script_dir=d:\cdash\draco\regression
set script_name=Draco_Win32.cmake
set ctestparts=Configure,Build,Test,Submit

set subproj=draco

:cdashdebug
set build_type=Debug
set work_dir=%base_dir%\%subproj%\%dashboard_type%_%comp%\%build_type%

rem print some information
echo Environment:
echo .
set
echo .
echo -----     -----     -----     -----     -----     

rem navigate to the workdir
if not exist %work_dir% mkdir %work_dir%
cd /d %work_dir%

rem clear the build directory (need to do this here to avoid a hang).
rem if exist %work_dir%\build rmdir /s /q build
if not exist %work_dir%\build mkdir build
if not exist %work_dir%\source mkdir source
if not exist %work_dir%\target mkdir target

rem run the ctest script

echo ctest -VV -S %script_dir%\%script_name%,%dashboard_type%,%build_type%,%ctestparts%
ctest -VV -S %script_dir%\%script_name%,%dashboard_type%,%build_type%,%ctestparts% > %base_dir%\logs\draco-%build_type%-cbts.log

:cdashrelease
set build_type=Release
set work_dir=%base_dir%\%subproj%\%dashboard_type%_%comp%\%build_type%

rem navigate to the workdir
if not exist %work_dir% mkdir %work_dir%
cd /d %work_dir%

rem clear the build directory (need to do this here to avoid a hang).
rem if exist %work_dir%\build rmdir /s /q build
if not exist %work_dir%\build mkdir build
if not exist %work_dir%\source mkdir source
if not exist %work_dir%\target mkdir target

rem run the ctest script

echo ctest -VV -S %script_dir%\%script_name%,%dashboard_type%,%build_type%,%ctestparts%
ctest -VV -S %script_dir%\%script_name%,%dashboard_type%,%build_type%,%ctestparts% > %base_dir%\logs\draco-%build_type%-cbts.log

:done
echo You need to remove -k from script launch to let this window close automatically.


