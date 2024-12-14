/*
    Standard Sprite with Texture
    that is transformed from local space
*/

#include "common.hlsl"

cbuffer LocalConstants : register(b4) {
    float brightness;
    int strength;
}

struct VSINPUT {
    float4 pos : POSITION;
    float4 uv : TEXCOORD0;
};

struct PSINPUT {
    float4 pos : SV_POSITION;
    // float4 pos : POSITION; <-- will not be interpreted by rasterizers
    float4 uv : TEXCOORD0;
    float4 color : COLOR0;
};

PSINPUT VS_MAIN(VSINPUT param) {
    PSINPUT output = (PSINPUT) 0; 
    float4x4 MVP = mul(mul(WORLD, VIEW), PROJECTION);
    output.pos = mul(param.pos, MVP);
    output.uv = param.uv;
    return output; 
}

float4 PS_MAIN(PSINPUT param) : SV_TARGET {
    float2 _uv = float2(param.uv.x * 1, param.uv.y);
    float v = ( 1.0f + sin(TIME) ) / 2.0f;
    v *= brightness;
    float4 rgba = defaultTexture.Sample(defaultSampler, _uv);
    rgba.rgb *= v;
    return rgba;
}

