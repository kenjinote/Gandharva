cd "%~dp0"
if errorlevel 1 goto :eof

del /A gandharva.sdf
del /A gandharva.v11.suo
rd /S /Q gandharva\Release
rd /S /Q gandharva\Debug
rd /S /Q Release
rd /S /Q Debug
rd /S /Q .vs

:eof
