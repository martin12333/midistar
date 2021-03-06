@ECHO off
REM Change to script dir:
CD "%~dp0"

REM Change to source dir if we're in the scripts folder:
FOR %%I in (.) do SET dir=%%~nxI
IF "%dir%"=="win32" (CD ..\..)
SET proj_dir="%cd%"

REM Find MSBuild:
ECHO Finding MSBuild...
WHERE msbuild.exe
if %errorLevel% == 1 (
    ECHO Could not find MSBuild^! Please install Visual Studio 2017, open "Developer Command Prompt For VS 2017", and run this script there.
    GOTO :error
)
ECHO.

REM Get build type:
SET build_type=%1
IF "%~1"=="" (
	ECHO WARNING: Build type not specified. Setting to Debug mode...
	SET build_type=Debug
)

REM Build project:
ECHO Building midistar in %build_type% mode...
MKDIR build
CD build
cmake .. || GOTO :error
msbuild midistar.sln /p:Configuration=%build_type% || GOTO :error
SET makeerror=0
GOTO :fin

:error
CD %proj_dir%
SET makeerror=1

:fin
CD %proj_dir%
