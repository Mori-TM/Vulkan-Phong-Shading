@echo off
glslc.exe VertexShader.vert -o VertexShader.spv
glslc.exe FragmentShader.frag -o FragmentShader.spv
pause