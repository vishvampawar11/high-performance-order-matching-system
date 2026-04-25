@echo off

REM --- CONFIG ---
set PG_BIN=C:\msys64\mingw64\bin
set PG_DATA=C:\postgres_data
set LOGFILE=C:\postgres_data\logfile.txt

REM --- START POSTGRES ---
echo Starting PostgreSQL...

"%PG_BIN%\pg_ctl.exe" -D "%PG_DATA%" -l "%LOGFILE%" start

IF %ERRORLEVEL% NEQ 0 (
    echo Failed to start PostgreSQL.
    pause
    exit /b %ERRORLEVEL%
)

echo PostgreSQL started successfully.
pause