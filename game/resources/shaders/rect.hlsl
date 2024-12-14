/*
    Bounding Rectangle
*/


#include "common.hlsl"

struct VSINPUT {
    float4 pos : POSITION;
    float4 color : COLOR0;
};

struct PSINPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

PSINPUT VS_MAIN(VSINPUT param){
    PSINPUT output = (PSINPUT)0; 
    float4x4 MVP = mul(mul(WORLD, VIEW), PROJECTION);
    output.pos = mul(param.pos, MVP);
    output.color = param.color;
    return output; 
}

float4 PS_MAIN(PSINPUT param) : SV_TARGET {
    return param.color;
}


