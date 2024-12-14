#ifndef GRAPHICS_D3D_H
#define GRAPHICS_D3D_H

#include <Windows.h>
#include <core/GameResource.h>
#include <d3d11sdklayers.h>
#include <DirectXMath.h>
#include <utils/Debug.h>
#include <utils/WICTextureLoader.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <vector>
#include <string>

/*
 * Header:  Graphics_d3d.h, Graphics.h
 * Impl:    Graphics_d3d.cpp
 * Purpose: Platform level graphics renderer for Direct3D
 * Author:  Michael Herman
 * */

using namespace DirectX;

extern ID3D11Device *device;
extern ID3D11DeviceContext *deviceContext;
extern ID3D11RenderTargetView *renderTarget;
extern ID3D11Texture2D *backBuffer;
extern IDXGISwapChain *swapChain;


enum ShaderType {
    VERTEX,
    PIXEL
};

enum GeomType {
    TRIANGLE,
    SPRITE,
    QUAD,
    LINE,
    TEXT,
    TEXT2,
    MAX_COUNT // <- do not use
};

struct Vertex {
    DirectX::XMFLOAT4 pos;
    DirectX::XMFLOAT2 texcoord;
};
struct LineVertex {
    DirectX::XMFLOAT4 pos;
    DirectX::XMFLOAT4 color;
};
struct alignas(16) GlobalConstantsBuffer {
    float time;
};

struct alignas(64) LocalConstantsBuffer {
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};

/*
 * Direct3D Specific Representations of Core Game Object
 * */
//

struct TextureD3D {
    std::string id;
    ID3D11ShaderResourceView *textureResource;
    ID3D11Resource *textureData;
    ID3D11SamplerState *textureSampler;
};
struct ShaderD3D {
    std::string id;
    ID3D11VertexShader *vShader;
    ID3D11PixelShader *pShader;
};
struct MaterialD3D {
    std::string id;
    ID3D11Buffer* customConstants = nullptr;
    // std::unordered_map<std::string, TextureD3D*> textures;
};
struct GeometryD3D {
    std::string id;
    GeomType instanceType;
    ID3D11Buffer *vertexBuffer;
};

static XMMATRIX CreateWorldMatrix(DirectX::XMMATRIX translation, DirectX::XMMATRIX scale, DirectX::XMMATRIX rotation);
static XMMATRIX CreateProjectionMatrix(float scale = 1.0f);
static HRESULT  LoadShader(std::string path, ShaderType st, ID3DBlob **sBuffer, ID3DBlob **eBuffer);
static bool     CheckFileExistence(std::string filePath);
static void     UpdateConstantBuffers(ID3D11Resource *resource, void* data, UINT dataSize);
static D3D11_TEXTURE2D_DESC GetTextureDescription(ID3D11Resource *texture);

static VOID     BindDefaultResourceToPipeline();
static VOID     InitDefaultGlobalConstants(GlobalConstantsBuffer *gc);
static VOID     InitDefaultLocalConstants(LocalConstantsBuffer *lc);
static HRESULT  InitBoundingRect();
static HRESULT  ConstructVertexBuffer(void *vertices, INT size, ID3D11Buffer **vBuffer); 
static HRESULT  ConstructInputLayout(GeomType type);
static HRESULT  ConstructD3DConstantBuffer(void *data, UINT size, bool isDynamic, ID3D11Buffer **cBuffer);
static HRESULT  ConstructD3DTexture(std::string texturePath, ID3D11ShaderResourceView **textureResource, ID3D11Resource **textureData, ID3D11SamplerState **textureSampler);
// static HRESULT  ConstructD3DTextureFromMemory(std::string texturePath, ID3D11ShaderResourceView **textureResource, ID3D11Resource **textureData, ID3D11SamplerState **textureSampler);
static HRESULT  ConstructD3DShader(std::string filePath, ID3D11VertexShader **vs, ID3D11PixelShader **ps);
static HRESULT  ConstructD3DBlending(ID3D11BlendState **blendState);
static VOID     ConstructD3DViewport(POINT &dim, DirectX::XMFLOAT2 offsets, D3D11_VIEWPORT *vp);
static VOID     ConstructD3DDefaultRasterizer(ID3D11RasterizerState **rasterizer, bool antialiased = true);
static PVOID    ConstructDynamicConstantBuffer(GameResource::ShaderParams **shaderParam, UINT &bufferSize);

namespace Graphics_D3D {
    HRESULT Initialize(HWND hwnd, POINT &wd);
    HRESULT Shutdown();
    std::vector<IDXGIAdapter*> EnumerateHardware();
    VOID ClearBackground(float bgClolor[]);
    VOID Present();
    VOID UpdateShaderGlobalConstants();
    VOID UpdateWindowSize(POINT &wd); // TODO
}
#endif
