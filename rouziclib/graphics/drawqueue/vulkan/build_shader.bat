@echo off
setlocal
cd /d "%~dp0"
set "PATH=C:\msys\ucrt64\bin;C:\msys\usr\bin;%PATH%"

where glslc >nul 2>nul || (
	echo glslc was not found 1>&2
	exit /b 1
)
where spirv-val >nul 2>nul || (
	echo spirv-val was not found 1>&2
	exit /b 1
)

glslc -O --target-env=vulkan1.1 drawqueue.comp -o drawqueue.comp.spv || exit /b 1
spirv-val --target-env vulkan1.1 drawqueue.comp.spv || exit /b 1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0embed_spirv.ps1" "%~dp0drawqueue.comp.spv" "%~dp0drawqueue.comp.spv.h" || exit /b 1

echo Generated and validated drawqueue.comp.spv.h
