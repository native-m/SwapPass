#pragma once

// ALL SHADER GOES HERE!

static const char g_strVS[] =
"struct FillRect {\n"
"	float4 pos;\n"
"	float2 uv;\n"
"};\n"

// Basically we construct a big rectangle with UV coords on it
"void MainVS(uint id : SV_VertexID, out float4 pos : SV_Position, out float2 uv : TEXCOORD)\n"
"{\n"
"	static const FillRect rect[6] = {\n"
"		{ float4(1.0, 1.0, 0.0, 1.0), float2(1.0, 0.0) },\n"
"		{ float4(1.0,-1.0, 0.0, 1.0), float2(1.0, 1.0) },\n"
"		{ float4(-1.0,-1.0,0.0, 1.0), float2(0.0, 1.0) },\n"
"		{ float4(1.0, 1.0, 0.0, 1.0), float2(1.0, 0.0) },\n"
"		{ float4(-1.0,-1.0,0.0, 1.0), float2(0.0, 1.0) },\n"
"		{ float4(-1.0,1.0, 0.0, 1.0), float2(0.0, 0.0) }\n"
"	};\n"

"	pos = rect[id].pos;\n"
"	uv = rect[id].uv;\n"
"}\n"
;

// Pass a custom post-processing shader
static const char g_strPS[] =
"Texture2D tex : register(t0);\n"
"Texture2D zbuf : register(t1);\n"
"SamplerState texSampler : register(s0);\n"

"float4 MainPS(in float4 pos : SV_Position, in float2 uv : TEXCOORD) : SV_Target\n"
"{\n"
"	return tex.Sample(texSampler, uv).rgrg;\n"
"}\n"
;