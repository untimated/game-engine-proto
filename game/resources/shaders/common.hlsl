/*
    Shared Declaration of HLSL
*/

// Vertex
cbuffer GlobalConstants : register(b0) {
    // TODO: use real clock second (related to querycounter)
    // seconds = clamp(0.0,59.0)
    float TIME;
}

cbuffer LocalConstants : register(b1) {
    float4x4 WORLD;
    float4x4 VIEW;
    float4x4 PROJECTION;
}

// Pixel
Texture2D defaultTexture : register(ps, t);
SamplerState defaultSampler : register(ps, s);
