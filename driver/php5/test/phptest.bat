@echo off

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\type\typeTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\sdbTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\csTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\clTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\domainTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\groupTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\sqlTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\configure.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\authenticate.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\procedure.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\transaction.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\backupoffline.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\task.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDB\session.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaCS\csTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaCS\clTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaCollection\clTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaCollection\indexTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaCollection\recordTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaCollection\lobTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaCursor\cursorTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaDomain\domainTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaGroup\groupTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaGroup\nodeTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaNode\nodeTest.php
if %errorlevel% NEQ 0 goto ERR

..\..\..\tools\server\php_win\php.exe -c .\php.ini ..\..\test\php\tools\phpunit.phar --log-junit phptest.error.txt ..\..\test\php\SequoiaLob\lobTest.php
if %errorlevel% NEQ 0 goto ERR

goto SUCCESS

:ERR
echo ------------------ An Error! ------------------
goto END

:SUCCESS
echo ------------------ All Successful! ------------------
goto END

:END