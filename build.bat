@echo off

set MODES=-DHY3D_DEBUG=1 -DVULKAN_VALIDATION_LAYERS_ON=1 -D_CRT_SECURE_NO_WARNINGS=1
set OPTIMIZATION=-Od
set COMPILER_FLAGS=%MODES% -MTd -WL %OPTIMIZATION% -nologo -fp:fast -fp:except- -Gm- -EHsc -Zo -Oi -W4 -wd4100 -wd4458 -wd4505 -wd4201 -FC -Zi -GS-
set LIBS=user32.lib gdi32.lib
set LINKER_FLAGS=-subsystem:windows -incremental:no -opt:ref %LIBS%
set EXE_NAME="HY3D VULKAN"

if not exist .\build mkdir .\build
pushd .\build

set EXPOTED_FUNCS=-EXPORT:UpdateAndRender -EXPORT:InitializeVulkan

del *.pdb > NUL 2> NUL
cl  %COMPILER_FLAGS% /I %VULKAN_SDK%\Include ..\code\hy3d_engine.cpp -Fmhy3d_engine.map -LD -link -incremental:no -opt:ref  -PDB:hy3d_engine_%RANDOM%.pdb %EXPOTED_FUNCS%
cl  %COMPILER_FLAGS% ..\code\hy3d.cpp -Fmhy3d.map -Fe%EXE_NAME% -link %LINKER_FLAGS%
popd
