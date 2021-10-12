@echo off

IF [%1] == [] (
	echo Enter build mode: d or r
	goto :eof
)

IF %1 == d (
	echo Debug Build
	set MODE=-DHY3D_DEBUG=1 -DVULKAN_VALIDATION_LAYERS_ON=1 -Od
)
IF %1 == r (
	echo Release Build
	SET MODE=-DHY3D_DEBUG=0 -DVULKAN_VALIDATION_LAYERS_ON=0 -O2
)

set COMPILER_FLAGS=%MODE% -MTd -WL %OPTIMIZATION% -D_CRT_SECURE_NO_WARNINGS=1 -nologo -fp:fast -fp:except- -Gm- -EHsc -Zo -Oi -W4 -wd4100 -wd4458 -wd4505 -wd4201 -FC -Zi -GS-
set LIBS=user32.lib gdi32.lib 
rem FOR THE DIALOG STUFF Comdlg32.lib
set LINKER_FLAGS=-subsystem:windows -incremental:no -opt:ref %LIBS%
set EXE_NAME=hy3d_vulkan

if not exist .\build mkdir .\build
pushd .\build

rc /fo win32_hy3d.res /nologo ..\code\win32_resource.rc

del *.pdb > NUL 2> NUL
set EXPOTED_FUNCS=-EXPORT:UpdateAndRender
cl  %COMPILER_FLAGS% ..\code\hy3d_engine.cpp -Fmhy3d_engine.map -LD -link -incremental:no -opt:ref  -PDB:hy3d_engine_%RANDOM%.pdb %EXPOTED_FUNCS%

rem set INCLUDES=/I%VULKAN_SDK%\Include

cl  %COMPILER_FLAGS% ..\code\win32_platform.cpp  win32_hy3d.res -Fwin32_platform.map -Fe%EXE_NAME% -link %LINKER_FLAGS%
popd

:eof