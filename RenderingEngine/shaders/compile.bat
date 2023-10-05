@echo off

del *.bin

"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E vs_main /T vs_5_0 /Fo vertex_shader.bin forwardRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E vs_fullscreen /T vs_5_0 /Fo vertex_shader_fullscreen.bin fullscreenRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_main /T ps_5_0 /Fo pixel_shader_forward.bin forwardRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_debug /T ps_5_0 /Fo pixel_shader_debug.bin debugRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_deferred_gbuffer /T ps_5_0 /Fo pixel_shader_deferred.bin deferredRendering.hlsl

 