@echo off

setlocal

SET DIR=%~dp0
SET DIR=%DIR:~0,-1%

cd %DIR%

if exist .env\ (
    echo Python env found
) else (
    python -m venv .env
    .env\Scripts\pip install -r requirements.txt
)

.env\Scripts\python main.py