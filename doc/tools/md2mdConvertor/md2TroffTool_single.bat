@echo off&setlocal enabledelayedexpansion

set SCRIPTPATH=%~dp0
set PROGPATH=!SCRIPTPATH!\sundown-master\bin\md2TroffTool.exe
::set OUTPUTPATH=!SCRIPTPATH!..\..\manual\

set INPUTFILE=
set MD2MDFILE=
set MD2TROFFFILE=
set OUTPUTFILE=
set /A ARGS_COUNT=0
FOR %%A in (%*) DO SET /A ARGS_COUNT+=1


:: 1) check and get the input arguments
if %ARGS_COUNT% LSS 4 goto SYNTAX:
if "%1" == "-i" set INPUTFILE=%2
if "%3" == "-i" set INPUTFILE=%4
if "%1" == "-o" set OUTPUTFILE=%2
if "%3" == "-o" set OUTPUTFILE=%4
if "%INPUTFILE%" EQU "" goto SYNTAX:
if "%OUTPUTFILE%" EQU "" goto SYNTAX:
set MD2MDFILE="%OUTPUTFILE%.md2md"
set MD2TROFFFILE="%OUTPUTFILE%.md2troff"
goto SKIP_POINT:
:SYNTAX
echo Syntax: %~nx0 -i ^<input file^> [-o output file]
goto END:
:SKIP_POINT


:: 2) convert github's markdown to pandoc's markdown
%PROGPATH% -i "%INPUTFILE%" -o "%MD2MDFILE%"
if not ERRORLEVEL 0 goto ERROR_POINT1:
goto SKIP_POINT1:
:ERROR_POINT1
echo "Batch file ERROR: failed to convert %INPUTFILE% from github's markdown to pandoc's markdown, errno: %ERRORLEVEL%"
goto :END
:SKIP_POINT1


:: 3) convert pandoc's markdown to troff file
pandoc -s --column=80 --wrap=auto   --from=markdown --to=man --output="%MD2TROFFFILE%"  "%MD2MDFILE%"
if not ERRORLEVEL 0 goto ERROR_POINT2:
goto SKIP_POINT2:
:ERROR_POINT2
echo "Batch file ERROR: failed to use pandoc to convert %MD2TROFFFILE% to %OUTPUTFILE%, errno: %ERRORLEVEL%"
goto :END
:SKIP_POINT2


:: 4) post-processing the troff file

:: 4.1) reduce indent in #sample#
%PROGPATH% -i "%MD2TROFFFILE%" -o "%OUTPUTFILE%" -p

:: 4.2) remove the middle file
if EXIST "%MD2MDFILE%" del "%MD2MDFILE%"
if EXIST "%MD2TROFFFILE%" del "%MD2TROFFFILE%"


:END

