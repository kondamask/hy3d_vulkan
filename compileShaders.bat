@echo off
echo Compiling Shaders...
pushd build
glslc.exe ..\shaders\triangle.vert -o vert.spv
glslc.exe ..\shaders\triangle.frag -o frag.spv
popd
echo Success!