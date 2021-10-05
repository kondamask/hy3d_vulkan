@echo off

set MODES=-DHYV_DEBUG=1 -DVULKAN_VALIDATION_LAYERS_ON=1 -D_CRT_SECURE_NO_WARNINGS=1
set COMPILER_FLAGS=%MODES% -MTd -WL -Od -nologo -fp:fast -fp:except- -Gm- -EHsc -Zo -Oi -W4 -wd4100 -wd4458 -wd4505 -wd4201 -FC -Zi -GS-
set LIBS=user32.lib gdi32.lib
set LINKER_FLAGS=-subsystem:windows -incremental:no -opt:ref %LIBS%
set EXE_NAME=hyv

if not exist .\build mkdir .\build
pushd .\build

del *.pdb > NUL 2> NUL
cl  %COMPILER_FLAGS% ..\code\hyv_engine.cpp -Fmhyv_engine.map -LD -link -incremental:no -opt:ref  -PDB:hyv_engine_%RANDOM%.pdb -EXPORT:UpdateAndRender
cl  %COMPILER_FLAGS% /I %VULKAN_SDK%\Include ..\code\hyv.cpp -Fmhyv.map -Fe%EXE_NAME% -link %LINKER_FLAGS%
popd
