@echo off
echo Building Shaders...

pushd assets
if not exist shaders mkdir shaders
pushd shaders 

del *.spv

for /r %%i in (..\..\code\shaders\*.vert) do (
	echo %%i
	%VULKAN_SDK%/bin/glslc %%i -o %%~ni.vert.spv
)
for /r %%i in (..\..\code\shaders\*.frag) do (
	echo %%i	
	%VULKAN_SDK%/bin/glslc %%i -o %%~ni.frag.spv
)

popd
popd