@echo off
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E vs_main /T vs_5_0 /Fo vertex_shader.bin triangle.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_main /T ps_5_0 /Fo pixel_shader.bin triangle.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_main /T ps_5_0 /Fo light_pixel.bin light_wireframe.hlsl
