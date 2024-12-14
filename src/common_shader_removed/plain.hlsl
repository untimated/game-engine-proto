/*
    Standard Test Plain Shader
*/

#include "common.hlsl"

cbuffer CustomConstants : register(b4) {
    float brightness;
    float4 color;
}

struct VSINPUT {
    float4 pos : POSITION;
    float4 uv : TEXCOORD0;
};

struct PSINPUT {
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD0;
    float4 color : COLOR0;
};

PSINPUT VS_MAIN(VSINPUT param){
    PSINPUT output = (PSINPUT)0; 
    float4x4 MVP = mul(mul(WORLD, VIEW), PROJECTION);
    output.pos = mul(param.pos, MVP);
    output.uv = param.uv;
    return output; 
}

float4 PS_MAIN(PSINPUT param) : SV_TARGET {
    float3 rgb = color.rgb * brightness;
    return float4(rgb, 1.0f);
}


