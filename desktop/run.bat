@echo off

setlocal

SET DIR=%~dp0
SET DIR=%DIR:~0,-1%

if exist %DIR%\.env\ (
    echo Python env found
) else (
    python -m venv %DIR%\.env
    %DIR%\.env\Scripts\pip install -r %DIR%\requirements.txt
)

%DIR%\.env\Scripts\python %DIR%\main.py