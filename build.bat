@echo off

REM DEBUG
set MODE=-DHY3D_DEBUG=1 -DVULKAN_VALIDATION_LAYERS_ON=1 -Od

REM RELEASE
REM set MODE=-DHY3D_DEBUG=0 -DVULKAN_VALIDATION_LAYERS_ON=0 -O2

set COMPILER_FLAGS=%MODE% -MTd -WL %OPTIMIZATION% -D_CRT_SECURE_NO_WARNINGS=1 -nologo -fp:fast -fp:except- -Gm- -EHsc -Zo -Oi -W4 -wd4100 -wd4458 -wd4505 -wd4201 -FC -Zi -GS-
set LIBS=user32.lib gdi32.lib Comdlg32.lib
set LINKER_FLAGS=-subsystem:windows -incremental:no -opt:ref %LIBS%
set EXE_NAME=hy3d_vulkan

if not exist .\build mkdir .\build
pushd .\build

del *.pdb > NUL 2> NUL
set EXPOTED_FUNCS=-EXPORT:UpdateAndRender
cl  %COMPILER_FLAGS% ..\code\hy3d_engine.cpp -Fmhy3d_engine.map -LD -link -incremental:no -opt:ref  -PDB:hy3d_engine_%RANDOM%.pdb %EXPOTED_FUNCS%

cl  %COMPILER_FLAGS% -I %VULKAN_SDK%\Include ..\code\win32_platform.cpp -Fwin32_platform.map -Fe%EXE_NAME% -link %LINKER_FLAGS%
popd
