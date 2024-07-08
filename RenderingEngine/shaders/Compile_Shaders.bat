@echo off

REM Define the path to fxc.exe
SET FXC_PATH="C:\Program Files (x86)\Windows Kits\10\bin\10.0.22000.0\x64\fxc.exe"

REM Check if the fxc.exe exists at the specified path
IF NOT EXIST %FXC_PATH% (
    ECHO fxc.exe not found at %FXC_PATH%. Please check the path.
    PAUSE
    EXIT /B 1
)

REM Delete existing bin files
del /s /q bin\*.bin

REM Compile shaders
%FXC_PATH% /nologo /E vs_main                        /T vs_5_0 /Fo ./bin/vertex_shader.bin                           forwardRendering.hlsl
%FXC_PATH% /nologo /E vs_fullscreen                  /T vs_5_0 /Fo ./bin/vertex_shader_fullscreen.bin                vs_fullscreenRendering.hlsl
%FXC_PATH% /nologo /E vs_shadowCast                  /T vs_5_0 /Fo ./bin/vertex_shader_csmShadowCast.bin             vs_csmShadowCast.hlsl
%FXC_PATH% /nologo /E vs_instancedRendering          /T vs_5_0 /Fo ./bin/vertex_shader_instancedRendering.bin        vs_instancedRendering.hlsl

%FXC_PATH% /nologo /E ps_main                        /T ps_5_0 /Fo ./bin/pixel_shader_forward.bin                    forwardRendering.hlsl
%FXC_PATH% /nologo /E ps_debug                       /T ps_5_0 /Fo ./bin/pixel_shader_debug.bin                      ps_debugRendering.hlsl
%FXC_PATH% /nologo /E ps_deferred_gbuffer            /T ps_5_0 /Fo ./bin/pixel_shader_deferred.bin                   ps_deferredRendering.hlsl
%FXC_PATH% /nologo /E ps_lightingEnvPass             /T ps_5_0 /Fo ./bin/pixel_shader_deferred_lighting_env.bin      ps_deferredRenderingLightingEnv.hlsl
%FXC_PATH% /nologo /E ps_lightingLocalPass           /T ps_5_0 /Fo ./bin/pixel_shader_deferred_lighting_local.bin    ps_deferredRenderingLightingLocal.hlsl
%FXC_PATH% /nologo /E ps_ssao                        /T ps_5_0 /Fo ./bin/pixel_shader_ssao_rendering.bin             ps_ssaoRendering.hlsl
%FXC_PATH% /nologo /E ps_texture_blur                /T ps_5_0 /Fo ./bin/pixel_shader_texture_blur.bin               ps_textureBlur.hlsl
%FXC_PATH% /nologo /E ps_tonemap                     /T ps_5_0 /Fo ./bin/pixel_shader_tone_map.bin                   ps_tonemapping.hlsl
%FXC_PATH% /nologo /E ps_instanced_deferred_gbuffer  /T ps_5_0 /Fo ./bin/pixel_shader_instanced_gbuffer.bin          ps_instancedDeferredRendering.hlsl
%FXC_PATH% /nologo /E ps_instanced_wireframe         /T ps_5_0 /Fo ./bin/pixel_shader_instanced_wireframe.bin        ps_instancedWireframeRendering.hlsl

%FXC_PATH% /nologo /E cs_updateLightPosition         /T cs_5_0 /Fo ./bin/compute_shader_ptLight_movement.bin         cs_ptLightMovement.hlsl
%FXC_PATH% /nologo /E cs_genHistogram                /T cs_5_0 /Fo ./bin/compute_shader_lumin_histogram.bin          cs_luminanceHistogram.hlsl
%FXC_PATH% /nologo /E cs_luminAverage                /T cs_5_0 /Fo ./bin/compute_shader_lumin_average.bin            cs_luminanceAverage.hlsl
%FXC_PATH% /nologo /E cs_bloomDownsample             /T cs_5_0 /Fo ./bin/compute_shader_bloom_downsample.bin         cs_bloomDownsample.hlsl
%FXC_PATH% /nologo /E cs_bloomUpsample               /T cs_5_0 /Fo ./bin/compute_shader_bloom_upsample.bin           cs_bloomUpsample.hlsl
 
ECHO Compilation completed.
PAUSE