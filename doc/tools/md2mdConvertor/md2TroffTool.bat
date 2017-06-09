@echo off

set SUFFIX_CN=_cn.troff
set SUFFIX_EN=_en.troff
set TOOL1=md2TroffTool.exe
set TOOL2=pandoc
set SCRIPT_PATH=%~dp0
set PROG_PATH=!SCRIPT_PATH!\sundown-master\bin\md2TroffTool.exe

set TOTAL_NUM=0
set SUCCESS_NUM=0
set FAILURE_NUM=0

set MODE=
set SOURCE=
set TARGET=
set INPUT_FILE=
set OUTPUT_FILE=
set /A ARGS_COUNT=0
FOR %%A in (%*) DO SET /A ARGS_COUNT+=1

:: 1) check tool program exist or not
if not EXIST "%PROG_PATH%" (
   echo Error: "%TOOL1%" or "%TOOL2%" do not exist. Please build  
   echo        "%TOOL1%" from source files or install %TOOL2% first.
   goto END:
)

:: 2) check and get the arguments
if %ARGS_COUNT% LSS 4 goto ERROR_SYNTAX:
if "%1" == "-i" set SOURCE=%2
if "%3" == "-i" set SOURCE=%4
if "%1" == "-o" set TARGET=%2
if "%3" == "-o" set TARGET=%4
if "%SOURCE%" EQU "" goto ERROR_SYNTAX:
if "%TARGET%" EQU "" goto ERROR_SYNTAX:
goto SKIP_SYNTAX:
:ERROR_SYNTAX
echo Syntax: %~nx0 -i ^<input file or directory^> -o ^<output file or directory^>
goto END:
:SKIP_SYNTAX

:: 3) check whether the input and output file and directory exist or not
if not exist "%SOURCE%" (
   echo Error: the input "%SOURCE%" does not exist.
   goto END:
)

:: 4) check the input and output paths are matched or not
set inputIsDir=0
pushd "%SOURCE%" 2>null
if %errorlevel% == 0 (set inputIsDir=1 & popd) else ( set inputIsDir=0 )
set outputIsDir=0
pushd "%TARGET%" 2>null
if %errorlevel% == 0 (set outputIsDir=1 & popd) else ( set outputIsDir=0 )
del null
if %inputIsDir% == 0 (
   if %outputIsDir% == 1 (
      echo Error: the input is a file, the output can not be a directory.
      goto END:
   )
)
if %inputIsDir% == 1 (
   if not exist "%TARGET%" (
      echo Error: the output directory "%TARGET%" does not exsist.
      goto END:
   )
)
if %outputIsDir% NEQ %inputIsDir% (
   echo Error: input and output paths should be either all files, or all directories.
   goto END:
)

:: 5) set the running mode. mode==0 for single, and mode==1 for batch
if %inputIsDir% == 1 ( set MODE=1 ) else ( set MODE=0 )

:: 6) begin to run
if %MODE% == 0 ( call :SINGLE %SOURCE% %TARGET%  ) else ( call :BATCH %SOURCE% %TARGET% )
call :COUNT
goto END:


:: function for batch running
:BATCH
set INPUT_DIR=%~1
set OUTPUT_DIR=%~2

for /R %INPUT_DIR% %%s in (*) do (
   set /A TOTAL_NUM+=1
:: get the number of tokens
   set tokensNum=0
   set INPUT_FILE=%%s
   for /f "delims=\ tokens=1-26" %%a in ("!INPUT_FILE!") do (
      if /i not "%%a" == "" set /a tokensNum+=1
      if /i not "%%b" == "" set /a tokensNum+=1
      if /i not "%%c" == "" set /a tokensNum+=1
      if /i not "%%d" == "" set /a tokensNum+=1
      if /i not "%%e" == "" set /a tokensNum+=1
      if /i not "%%f" == "" set /a tokensNum+=1
      if /i not "%%g" == "" set /a tokensNum+=1
      if /i not "%%h" == "" set /a tokensNum+=1
      if /i not "%%i" == "" set /a tokensNum+=1
      if /i not "%%j" == "" set /a tokensNum+=1
      if /i not "%%k" == "" set /a tokensNum+=1
      if /i not "%%l" == "" set /a tokensNum+=1
      if /i not "%%m" == "" set /a tokensNum+=1
      if /i not "%%o" == "" set /a tokensNum+=1
      if /i not "%%p" == "" set /a tokensNum+=1
      if /i not "%%q" == "" set /a tokensNum+=1
      if /i not "%%r" == "" set /a tokensNum+=1
      if /i not "%%s" == "" set /a tokensNum+=1
      if /i not "%%t" == "" set /a tokensNum+=1
      if /i not "%%u" == "" set /a tokensNum+=1
      if /i not "%%v" == "" set /a tokensNum+=1
      if /i not "%%w" == "" set /a tokensNum+=1
      if /i not "%%x" == "" set /a tokensNum+=1
      if /i not "%%y" == "" set /a tokensNum+=1
      if /i not "%%z" == "" set /a tokensNum+=1
   )
:: get the file name
   set fileName=
   for /f "delims=\ tokens=%tokensNum%" %%x in ("!INPUT_FILE!") do (
      set fileName=%%x
   )
:: get the function name
   set suffix=
   set targetFile=
   for /f "delims=_. tokens=1-3" %%x in ("!fileName!") do (
      set funcName=%%x
      set lang=%%y
      set suffix=%%z
      if "!lang!" == "md" ( 
         set suffix=!SUFFIX_CN!
         set targetFile=!funcName!
         set targetFile=!targetFile!!suffix!
         set OUTPUT_FILE=!OUTPUT_DIR!\!targetFile!
         call :TRANSFORM !INPUT_FILE! !OUTPUT_FILE!         
      ) else ( 
         if "!lang!" == "en" (
            if "!suffix!" == "md" (
               set suffix=!SUFFIX_EN!
               set targetFile=!funcName!
               set targetFile=!targetFile!!suffix!
               set OUTPUT_FILE=!OUTPUT_DIR!\!targetFile!
               call :TRANSFORM !INPUT_FILE! !OUTPUT_FILE!
            ) else (
               set /A TOTAL_NUM-=1
            )
         ) else (
            set /A TOTAL_NUM-=1
         )
      )
   )
)
goto END:



:: function for single running
:SINGLE
set INPUT_FILE="%SOURCE%"
set OUTPUT_FILE="%TARGET%"
set /A TOTAL_NUM+=1
call :TRANSFORM %INPUT_FILE% %OUTPUT_FILE%
goto END:



:: function for transforming
:TRANSFORM
set input_file=%~1
set output_file=%~2
set md2md_file="%output_file%.md2md"
set md2troff_file="%output_file%.md2troff"

:: 1) convert github's markdown to pandoc's markdown
%PROG_PATH% -i "%input_file%" -o "%md2md_file%"
if not ERRORLEVEL 0 (
   echo "ERROR: failed to convert %input_file% from github's markdown to pandoc's markdown, errno: %ERRORLEVEL%"
   goto :ERROR_TRANSFORM
)

:: 2) convert pandoc's markdown to troff file
pandoc -s --column=80 --wrap=auto   --from=markdown --to=man --output="%md2troff_file%"  "%md2md_file%"
if not ERRORLEVEL 0 (
   echo "ERROR: failed to use pandoc to convert %md2md_file% to %md2troff_file%, errno: %ERRORLEVEL%"
   goto :ERROR_TRANSFORM
)

:: 3) post-processing the troff file: reduce indent in #sample#
%PROG_PATH% -i "%md2troff_file%" -o "%output_file%" -p
if not ERRORLEVEL 0 (
   echo "ERROR: failed to convert %md2troff_file% to %output_file%, errno: %ERRORLEVEL%"
   goto :ERROR_TRANSFORM
)
goto DONE_TRANSFORM:

:: 4) handle error
:ERROR_TRANSFORM
echo ERROR: Failed to convert %1 to %2
set /A FAILURE_NUM+=1

:: 5) finish running
:DONE_TRANSFORM
if exist "%md2md_file%" del "%md2md_file%"
if exist "%md2troff_file%" del "%md2troff_file%"
set /A SUCCESS_NUM+=1
goto END:



:: function for counting
:COUNT
echo The result of converting md files to troff files are as below:
echo   TOTAL: %TOTAL_NUM%
echo Success: %SUCCESS_NUM%
echo Failure: %FAILURE_NUM%

:END
