@echo off

IF NOT EXIST %~dp0\..\bin mkdir %~dp0\..\bin
pushd %~dp0\..\bin

cl /nologo /wd4820 /wd4201 /DMSVC=1 /DX64=1 /GS- /FC /Wall ..\src\prop_util.c /link /nodefaultlib /incremental:no /entry:main kernel32.lib

popd