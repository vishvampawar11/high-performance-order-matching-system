@echo off

set PG_BIN=C:\msys64\mingw64\bin
set PG_DATA=C:\postgres_data

echo Stopping PostgreSQL...

"%PG_BIN%\pg_ctl.exe" -D "%PG_DATA%" stop

echo PostgreSQL stopped.
pause