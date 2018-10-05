@echo off

IF NOT EXIST %~dp0\..\bin mkdir %~dp0\..\bin
pushd %~dp0\..\bin

cl /nologo /GS- /Od /Zi /FC /Wall /wd4820 /wd4201 /wd4189 /wd4711 /DMSVC=1 /DX64=1 ..\src\main.c /link /nodefaultlib /subsystem:console /incremental:no /entry:main /out:resin.exe kernel32.lib

popd
