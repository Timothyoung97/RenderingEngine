@echo off

del *.bin

"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E vs_main /T vs_5_0 /Fo vertex_shader.bin forwardRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E vs_fullscreen /T vs_5_0 /Fo vertex_shader_fullscreen.bin fullscreenRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_main /T ps_5_0 /Fo pixel_shader_forward.bin forwardRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_debug /T ps_5_0 /Fo pixel_shader_debug.bin debugRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_deferred_gbuffer /T ps_5_0 /Fo pixel_shader_deferred.bin deferredRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_lightingEnvPass /T ps_5_0 /Fo pixel_shader_deferred_lighting_env.bin deferredRenderingLightingEnv.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_lightingLocalPass /T ps_5_0 /Fo pixel_shader_deferred_lighting_local.bin deferredRenderingLightingLocal.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_ssao /T ps_5_0 /Fo pixel_shader_ssao_rendering.bin ssaoRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_texture_blur /T ps_5_0 /Fo pixel_shader_texture_blur.bin textureBlur.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E ps_hdr_tonedown /T ps_5_0 /Fo pixel_shader_hdr_rendering.bin hdrRendering.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E cs_updateLightPosition /T cs_5_0 /Fo compute_shader_ptLight_movement.bin ptLightMovement.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E cs_genHistogram /T cs_5_0 /Fo compute_shader_lumin_histogram.bin luminanceHistogram.hlsl
"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /nologo /E cs_luminAverage /T cs_5_0 /Fo compute_shader_lumin_average.bin luminanceAverage.hlsl
 