@echo off
echo Compiling Shaders...

pushd build
if not exist shaders mkdir shaders
pushd shaders 

for /r %%i in (..\..\shaders\*.vert) do (
	echo %%i
	glslc %%i -o %%~ni.vert.spv
)
for /r %%i in (..\..\shaders\*.frag) do (
	echo %%i	
	glslc %%i -o %%~ni.frag.spv
)

popd
popd