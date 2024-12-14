#include <cstdint>
#include <cstdio>
#include <platform/Graphics_d3d.h>
#include <platform/Graphics.h>
#include <core/CoreGlobals.h>
#include <EnginePlatformAPI.h>

// extern
ID3D11Device *device = 0;
ID3D11DeviceContext *deviceContext = 0;
ID3D11RenderTargetView *renderTarget = 0;
ID3D11RenderTargetView *_renderTarget = 0;
ID3D11Texture2D *backBuffer = 0;
IDXGISwapChain *swapChain = 0;

// intern
ID3D11Buffer *g_gcBuffer;                                   // Global Constants Buffer
ID3D11Buffer *g_lcBuffer;                                   // Instance Constants Buffer
ID3D11Buffer *g_cBuffer;             
ID3DBlob *g_vsBuffer;                                       // Vertex Shader Buffer
ID3DBlob *g_psBuffer;                                       // Pixel Shader Buffer
ID3D11BlendState *g_blendState;                             // Blending States
D3D11_VIEWPORT g_viewPorts[2];                              // Viewports
ID3D11RasterizerState *g_rasterizer;                        // Rasterizer States
ID3D11InputLayout *g_inputLayout[GeomType::MAX_COUNT];      // Input Layouts
ID3D11Buffer *g_vertexBuffer[GeomType::MAX_COUNT];          // Shared use vertex buffer
ID3D11VertexShader *g_vShader[GeomType::MAX_COUNT];         // Shared use vertex shader
ID3D11PixelShader *g_pShader[GeomType::MAX_COUNT];          // Shared use pixel shader
TextureD3D g_texture;
ID3D11Debug *debugger;

// Non-graphics related
std::string dir; 
const int MAX_DEVICE = 4;
POINT wd = {800, 600};
GlobalConstantsBuffer globalConstants;
LocalConstantsBuffer localConstants;

const DirectX::XMFLOAT4 RED = {1.0f, 0.0f, 0.0f, 1.0f};
const DirectX::XMFLOAT4 GREEN = {0.0f, 1.0f, 0.5f, 1.0f};
const DirectX::XMFLOAT4 BLUE = {0.0f, 0.5f, 1.0f, 1.0f};


HRESULT Graphics_D3D::Initialize(HWND hwnd, POINT &wDim) {
    HRESULT hr;

    // Get Hardware
    std::vector<IDXGIAdapter*> hardwares = Graphics_D3D::EnumerateHardware();
    if(hardwares.size() == 0){
        Debug::Logger("No Graphics hardware found");
        return E_FAIL;
    }

    // Get Window Size
    wd = wDim;
    Debug::Logger("Current window width ", wd.x);
    Debug::Logger("Current window height ", wd.y);

    // Device
    D3D_FEATURE_LEVEL maxFeatureLevel;
    D3D_FEATURE_LEVEL featureLevels[2] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0
    };
    hr = D3D11CreateDevice(
        hardwares[0],
        D3D_DRIVER_TYPE_UNKNOWN,
        NULL,
        D3D11_CREATE_DEVICE_DEBUG,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &device,
        &maxFeatureLevel,
        &deviceContext
    );
    if(FAILED(hr)){
        Debug::Logger("Fail create device");
        return E_FAIL;
    }

    device->QueryInterface(__uuidof(ID3D11Debug), (void**) &debugger);

    UINT multisampleQuality;
    hr = device->CheckMultisampleQualityLevels(DXGI_FORMAT_B8G8R8A8_UNORM, 4, &multisampleQuality);
    if(FAILED(hr)){
        Debug::Logger("Fail to get max multisample quality");
    }
    Debug::Logger("Max MultisampleQuality = ", multisampleQuality);

    // Swap Chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    DXGI_RATIONAL hz = {60, 1};
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = wd.x;
    swapChainDesc.BufferDesc.Height = wd.y;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate = hz;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 4;
    swapChainDesc.SampleDesc.Quality = multisampleQuality <= 1 ? 0 : 1;
    swapChainDesc.Windowed = true;

    IDXGIFactory *factory;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**) &factory);
    if(FAILED(hr)){
        Debug::Logger("Fail create dxgifactory");
        return E_FAIL;
    }
    hr = factory->CreateSwapChain(
        device,
        &swapChainDesc,
        &swapChain
    );
    if(FAILED(hr)){
        Debug::Logger("Fail create swap chain");
        return E_FAIL;
    }
    factory->Release();
    factory = nullptr;

    // Get swapChain's internal backbuffer
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **) &backBuffer);
    if(FAILED(hr)){
        Debug::Logger("Fail get back buffer from swapchain");
        return E_FAIL;
    }

    // Create backBuffer as renderTarget
    hr = device->CreateRenderTargetView(backBuffer, NULL , &renderTarget) ;
    if(FAILED(hr)){
        Debug::Logger("Fail bind buffer to render target");
        return E_FAIL;
    }

    // Bind the renderTarget to pipeline
    // call this again when changing to another render target
    deviceContext->OMSetRenderTargets(1, &renderTarget, NULL);

    // Setup Viewport
    ConstructD3DViewport(wd, DirectX::XMFLOAT2(0.0f, 0.0f), &g_viewPorts[0]);

    // Setup Rasterizer
    ConstructD3DDefaultRasterizer(&g_rasterizer, true);

    // Setup default global constants buffer
    InitDefaultGlobalConstants(&globalConstants);
    InitDefaultLocalConstants(&localConstants);
    hr = ConstructD3DConstantBuffer(&globalConstants, sizeof(globalConstants), true, &g_gcBuffer);
    if(FAILED(hr)) return hr;
    hr = ConstructD3DConstantBuffer(&localConstants, sizeof(localConstants), true, &g_lcBuffer);
    if(FAILED(hr)) return hr;

    // Setup default input layout
    hr = ConstructInputLayout(GeomType::QUAD);
    if(FAILED(hr)) return hr;
    hr = ConstructInputLayout(GeomType::LINE);
    if(FAILED(hr)) return hr;

    // Setup default blending
    hr = ConstructD3DBlending(&g_blendState);
    if(FAILED(hr)) return hr;

    // Setup Bounding Rect drawing resources
    hr = InitBoundingRect();
    if(FAILED(hr)) return hr;

    BindDefaultResourceToPipeline();

    return S_OK;
}


HRESULT Graphics_D3D::Shutdown() {
    if( g_vsBuffer) {
        g_vsBuffer->Release();
        g_vsBuffer = nullptr;
    }
    if( g_psBuffer) {
        g_psBuffer->Release();
        g_psBuffer = nullptr;
    }
    if(g_lcBuffer) {
        g_lcBuffer->Release();
        g_lcBuffer = nullptr;
    }
    if(g_gcBuffer) {
        g_gcBuffer->Release();
        g_gcBuffer = nullptr;
    }
    if(g_rasterizer){
        g_rasterizer->Release();
        g_rasterizer = nullptr;
    } 
    if(g_blendState) {
        g_blendState->Release();
        g_blendState = nullptr;
    }
    for(int i=0; i < GeomType::MAX_COUNT; i++){
        if(g_inputLayout[i]) {
            g_inputLayout[i]->Release();
            g_inputLayout[i] = nullptr;
        }
        if(g_vShader[i]) {
            g_vShader[i]->Release();
            g_vShader[i] = nullptr;
        }
        if(g_pShader[i]) {
            g_pShader[i]->Release();
            g_pShader[i] = nullptr;
        }
        if(g_vertexBuffer[i]) {
            g_vertexBuffer[i]->Release();
            g_vertexBuffer[i] = nullptr;
        }
    }
    swapChain->Release();
    swapChain = nullptr;
    backBuffer->Release();
    backBuffer = nullptr;
    renderTarget->Release();
    renderTarget = nullptr;

    deviceContext->ClearState();
    deviceContext->Flush();
    deviceContext->Release();
    deviceContext = nullptr;

    device->Release();
    device = nullptr;
    // debugger->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
    debugger->Release();
    Debug::Logger("GraphicsD3D:: Memory has been released");

    return S_OK;
}


std::vector<IDXGIAdapter*> Graphics_D3D::EnumerateHardware() {
    IDXGIAdapter *currentAdapter;
    std::vector<IDXGIAdapter*> adapters;
    IDXGIFactory *factory;
    DXGI_ADAPTER_DESC adapterInfo;

    HRESULT hr;
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**) &factory);
    if(FAILED(hr)){
        Debug::Logger("Fail create dxgifactory");
    }

    for(int i = 0; i < MAX_DEVICE; i++) {
        hr = factory->EnumAdapters(i, &currentAdapter);
        if(hr != DXGI_ERROR_NOT_FOUND){
            adapters.push_back(currentAdapter);
            currentAdapter->GetDesc(&adapterInfo);
            Debug::Logger("---Device Found : ", adapterInfo.DeviceId);
            Debug::LoggerW(L"---Device Name : ", adapterInfo.Description);
        }else{
            Debug::Logger("DeviceID not found at slot : ", i);
        }
    }

    Debug::Logger("Finish enumerating device, release factory");
    factory->Release();
    return adapters;
}


void Graphics_D3D::UpdateShaderGlobalConstants() {
    // TODO: this is just mockup time
    if(globalConstants.time > 59.0f) globalConstants.time = 0.0f;
    globalConstants.time += 0.1f;
    UpdateConstantBuffers(g_gcBuffer, &globalConstants, sizeof(globalConstants));
}


/*
 * Internal Utils
 * */


static DirectX::XMMATRIX CreateWorldMatrix(
    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f),
    DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 1.0f, 0.0f),
    DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationZ(0.0f)
){
    // transpose rule: 
    // (SRT)' = T'R'S' . v
    DirectX::XMMATRIX SRT = DirectX::XMMatrixMultiply(XMMatrixMultiply(scale, rotation), translation);
    return XMMatrixTranspose(SRT); //T'R'S'
};


static DirectX::XMMATRIX CreateProjectionMatrix(float scale){
    DirectX::XMFLOAT2 offset = DirectX::XMFLOAT2(0.0f, 0.0f);
    float w = ((float) wd.x * scale) / 2.0f;
    float h = ((float) wd.y * scale) / 2.0f;
    float left = -w + offset.x;
    float bottom = -h + offset.y;
    float right = w + offset.x;
    float top = h + offset.y;
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterRH(left, right, bottom, top, 0.0f, -100.0f);
    return DirectX::XMMatrixTranspose(projection);
}


static HRESULT LoadShader(std::string path, ShaderType st, ID3DBlob **sBuffer, ID3DBlob **eBuffer){
    const char* entry = st == VERTEX ? "VS_MAIN" : "PS_MAIN";
    const char* target = st == VERTEX ? "vs_5_0" : "ps_5_0";
    D3D_SHADER_MACRO macros[] = {
        {"MYMACRO", "MYVALUE"},
        {NULL, NULL}
    };
    std::wstring wPath = Debug::ConvertStringToW(path);
    HRESULT hr = D3DCompileFromFile(
        wPath.c_str(),
        NULL,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entry, target,
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
        0,
        sBuffer,
        eBuffer
    );
    return hr;
}


static bool CheckFileExistence(std::string filePath){
    DWORD res = GetFileAttributes(filePath.c_str());
    if(res == INVALID_FILE_ATTRIBUTES) {
        Debug::Logger("Cannot find file within project directory", filePath);
        return false;
    }
    return true;
}


static void UpdateConstantBuffers(ID3D11Resource *resource, void* data, UINT dataSize){
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(mappedResource));
    deviceContext->Map(resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    memcpy(mappedResource.pData, data, dataSize);

    deviceContext->Unmap(resource, 0);
}


static D3D11_TEXTURE2D_DESC GetTextureDescription(ID3D11Resource *texture) {
    D3D11_TEXTURE2D_DESC desc;
    ID3D11Texture2D *tex = static_cast<ID3D11Texture2D *>(texture);
    tex->GetDesc(&desc);
    // float halfWidth = static_cast<float>(desc.Width) / 2.0f;
    // float halfHeight = static_cast<float>(desc.Height) / 2.0f;
    return desc;
}


/*
 * Internal
 * */


static void BindDefaultResourceToPipeline(){
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetConstantBuffers(0, 1, &g_gcBuffer);
    deviceContext->VSSetConstantBuffers(1, 1, &g_lcBuffer);
    deviceContext->PSSetConstantBuffers(0, 1, &g_gcBuffer);
    deviceContext->PSSetConstantBuffers(1, 1, &g_lcBuffer);
    deviceContext->OMSetBlendState(g_blendState, NULL, 0xffffffff);
    deviceContext->RSSetViewports(1, &g_viewPorts[0]);
    deviceContext->RSSetState(g_rasterizer);
}


static VOID InitDefaultGlobalConstants(GlobalConstantsBuffer *gc) {
    gc->time = 0.0f;
}


static VOID InitDefaultLocalConstants(LocalConstantsBuffer *ic){
    ic->world = CreateWorldMatrix(
        DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f),
        DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f),
        DirectX::XMMatrixRotationZ(0.0f)
        );
    ic->view = DirectX::XMMatrixIdentity();
    ic->projection = CreateProjectionMatrix();
}


static HRESULT InitBoundingRect() {
    HRESULT hr = ConstructD3DShader(
            "./" + CoreGlobals::RESOURCE_BASE_PATH + "/" + CoreGlobals::SHADERS_BASE_PATH + "/rect.hlsl", 
            &g_vShader[GeomType::QUAD],
            &g_pShader[GeomType::QUAD]
            );
    if(FAILED(hr)) return hr;
    LineVertex initRect[5];
    hr = ConstructVertexBuffer(initRect, sizeof(initRect), &g_vertexBuffer[GeomType::QUAD]);
    if(FAILED(hr)) return hr;
    return S_OK;
}


static HRESULT ConstructVertexBuffer(void *vertices, INT size, ID3D11Buffer **vBuffer) {
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

    D3D11_SUBRESOURCE_DATA initialData;
    ZeroMemory(&initialData, sizeof(initialData));

    bufferDesc.ByteWidth = size;
    initialData.pSysMem = vertices;
    hr = device->CreateBuffer(&bufferDesc, &initialData, vBuffer);
    if(FAILED(hr)) {
        Debug::Logger("Fail creating vertex buffer");
    }
    return hr;
}


static HRESULT ConstructD3DConstantBuffer(void *data, UINT size, bool isDynamic, ID3D11Buffer **cBuffer) {
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.Usage = isDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = isDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    bufferDesc.ByteWidth = size;

    D3D11_SUBRESOURCE_DATA initialData;
    ZeroMemory(&initialData, sizeof(initialData));
    initialData.pSysMem = data;

    hr = device->CreateBuffer(&bufferDesc, &initialData, cBuffer);
    if(FAILED(hr)) {
        Debug::Logger("Fail create constant buffers");
        return hr;
    }
    return hr;
}



static HRESULT ConstructD3DTexture(
    // const wchar_t *texturePath, 
    std::string texturePath, 
    ID3D11ShaderResourceView **textureResource,
    ID3D11Resource **textureData,
    ID3D11SamplerState **textureSampler
){
    std::string path = "./" + CoreGlobals::RESOURCE_BASE_PATH + "/" + CoreGlobals::ASSETS_BASE_PATH;
    if(texturePath.empty()){
        texturePath = path + "/checker_trans.png";
    }else if(!CheckFileExistence(texturePath)){
        Debug::Logger("texture file not found, resort to default texture");
        texturePath = path + "/checker.png";
    }
    std::wstring wTexturePath = Debug::ConvertStringToW(texturePath);
    HRESULT hr = S_OK;
    hr = CreateWICTextureFromFile(
        device,
        deviceContext,
        wTexturePath.c_str(),
        textureData,
        textureResource
    );
    if(FAILED(hr)){
        Debug::Logger("Fail to create wic texture");
        return hr;
    }
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&samplerDesc, textureSampler);
    if(FAILED(hr)){
        Debug::Logger("Fail to create texture sampler");
        return hr;
    }
    return hr;
}


static HRESULT ConstructD3DShader(std::string filePath, ID3D11VertexShader **vs, ID3D11PixelShader **ps) {
    std::string path = "./" + CoreGlobals::RESOURCE_BASE_PATH + "/" + CoreGlobals::SHADERS_BASE_PATH; // default Path should be in /shaders/
    if(filePath.empty()){
        filePath = path + "/sprite.hlsl";
    }else if(!CheckFileExistence(filePath)){
        Debug::Logger("Cannot find shader file, resort to default shader", path);
        filePath = path + "/sprite.hlsl";
    }

    HRESULT hr = S_OK;
    ID3DBlob *error;

    // Vertex Shader
    hr = LoadShader(filePath, VERTEX, &g_vsBuffer, &error);
    if(FAILED(hr)){
        Debug::Logger("Fail load vertex shader : ", (char *) error->GetBufferPointer());
        return hr;
    }
    hr = device->CreateVertexShader(
        g_vsBuffer->GetBufferPointer(),
        g_vsBuffer->GetBufferSize(),
        0,
        vs
    ); 
    if(FAILED(hr)){
        Debug::Logger("Fail creating vertex shader");
        return hr;
    }

    // Pixel Shader
    hr = LoadShader(filePath, PIXEL, &g_psBuffer, &error);
    if(FAILED(hr)){
        Debug::Logger("Fail load pixel shader : ", (char *) error->GetBufferPointer());
        return hr;
    }
    hr = device->CreatePixelShader(
        g_psBuffer->GetBufferPointer(),
        g_psBuffer->GetBufferSize(),
        0,
        ps
    ); 
    if(FAILED(hr)){
        Debug::Logger("Fail creating pixel shader");
        return hr;
    }

    if(error) error->Release();


    return hr;
}


static HRESULT ConstructInputLayout(GeomType type) {
    HRESULT hr = S_OK; 
    std::string path = "./" + CoreGlobals::RESOURCE_BASE_PATH + "/" + CoreGlobals::SHADERS_BASE_PATH;
    switch(type){
        case GeomType::QUAD :
        {
            ID3DBlob *err;
            hr = LoadShader(
                path + "/sprite.hlsl",
                ShaderType::VERTEX, &g_vsBuffer, &err
                );
            if(FAILED(hr)) return hr;

            D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };
            hr = device->CreateInputLayout(
                inputLayoutDesc,
                ARRAYSIZE(inputLayoutDesc),
                g_vsBuffer->GetBufferPointer(),
                g_vsBuffer->GetBufferSize(),
                &g_inputLayout[GeomType::QUAD]
            );
            hr = device->CreateInputLayout(
                inputLayoutDesc,
                ARRAYSIZE(inputLayoutDesc),
                g_vsBuffer->GetBufferPointer(),
                g_vsBuffer->GetBufferSize(),
                &g_inputLayout[GeomType::SPRITE]
            );
            if(FAILED(hr)){
                Debug::Logger("Fail creating input layout buffer for type QUAD");
            }
            break;
        }
        case GeomType::LINE :
        {
            ID3DBlob *err;
            hr = LoadShader(
                path + "/rect.hlsl",
                ShaderType::VERTEX, &g_vsBuffer, &err
                );
            if(FAILED(hr)) return hr;

            D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
            };
            hr = device->CreateInputLayout(
                inputLayoutDesc,
                ARRAYSIZE(inputLayoutDesc),
                g_vsBuffer->GetBufferPointer(),
                g_vsBuffer->GetBufferSize(),
                &g_inputLayout[GeomType::LINE]
            );
            if(FAILED(hr)){
                Debug::Logger("Fail creating input layout buffer for type LINE");
            }
            break;
        }
        default: break;
    }
    return hr;
}


static HRESULT ConstructD3DBlending(ID3D11BlendState **blendState){
    // Blending Formula 
    // C = Cs * Fs + Cd * Fd;
    // A = As * FAs + Ad * FAd;
    D3D11_RENDER_TARGET_BLEND_DESC rt;
    rt.BlendEnable = true;
    rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rt.BlendOp = D3D11_BLEND_OP_ADD;
    rt.SrcBlendAlpha = D3D11_BLEND_ONE;
    rt.DestBlendAlpha = D3D11_BLEND_ZERO;
    rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.RenderTarget[0] = rt;
    bd.AlphaToCoverageEnable = false;
    bd.IndependentBlendEnable = false;
    HRESULT hr = device->CreateBlendState(&bd, blendState);
    if(FAILED(hr)){
        Debug::Logger("Fail Create BlendState");
    }
    return hr;
}


static VOID ConstructD3DViewport(POINT &dim, DirectX::XMFLOAT2 offset, D3D11_VIEWPORT *vp){
    vp->Width = dim.x;
    vp->Height = dim.y;
    vp->TopLeftX = offset.x;
    vp->TopLeftY = offset.y;
    vp->MinDepth = 0.0f;
    vp->MaxDepth = 1.0f;
}


//TODO: Improve this so that it supports general options
static VOID ConstructD3DDefaultRasterizer(ID3D11RasterizerState **rasterizer, bool antialiased){
    D3D11_RASTERIZER_DESC rsDesc;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.DepthBias = false;
    rsDesc.DepthBiasClamp = 0;
    rsDesc.SlopeScaledDepthBias = 0;
    rsDesc.DepthClipEnable = true;
    rsDesc.ScissorEnable = true;
    rsDesc.MultisampleEnable = antialiased ? true : false;
    rsDesc.AntialiasedLineEnable = antialiased;
    device->CreateRasterizerState(&rsDesc, rasterizer );
}


static void* ConstructDynamicConstantBuffer(GameResource::ShaderParams *shaderParam, UINT &bufferSize){
    int stride = 16; // Maximum struct alignment
    int totalParams = (shaderParam)->size();
    bufferSize = totalParams * stride;
    char* constPtr = (char *) malloc(bufferSize);
    char* result = constPtr;
    if(constPtr == nullptr) {
        Debug::Logger("MaterialD3D:: Fail memory allocation ");
        return nullptr;
    }
    for(auto it = (shaderParam)->begin(); it != (shaderParam)->end(); ++it){
        switch(it->second.dataType){
            case GameResource::ShaderParamType::INTEGER : 
            {
                int value = std::any_cast<int>(it->second.value);
                memcpy(constPtr, &value, sizeof(int));
                break;
            }
            case GameResource::ShaderParamType::FLOATING : 
            {
                float value = std::any_cast<float>(it->second.value);
                memcpy(constPtr, &value, sizeof(float));
                break;
            }
            case GameResource::ShaderParamType::VEC4 : 
            {
                CoreMath::Vector4 value = std::any_cast<CoreMath::Vector4>(it->second.value);
                DirectX::XMFLOAT4 vec(value.r, value.g, value.b, value.a);
                memcpy(constPtr, &vec, sizeof(vec));
                break;
            }
            default:
                break;
        }
        constPtr += stride;
    }
    // constPtr -= stride * totalParams;
    return static_cast<void*>(result);
}


void Graphics_D3D::ClearBackground(float bgColor[]){
    if(!deviceContext) return;
    deviceContext->ClearRenderTargetView(renderTarget, bgColor);
}


VOID Graphics_D3D::Present() {
    swapChain->Present(1, 0);
}


/*
 * Graphics.h Function Implementations
 * it connects Core Renderer (platform independence) and GraphicsD3D 
 * these functions contains platfrom independent structures
 * such as GameObjects and Game Resources
 * */


bool Graphics::CreateShader(GameResource::Shader *shader){
    ShaderD3D *sd = new ShaderD3D();
    sd->id = shader->id;
    HRESULT hr = ConstructD3DShader(shader->filePath, &sd->vShader, &sd->pShader);
    if(FAILED(hr)){
        Debug::Logger("ShaderD3D:: Init Shader Failed");
        return false;
    }
    shader->resource.buffer = sd;
    shader->resource.type = GraphicsResource::Type::SHADER_RESOURCE;

    // Reflection
    GameResource::ShaderParams shaderMeta = {};
    ID3D11ShaderReflection *reflector = nullptr;
    hr = D3DReflect(g_vsBuffer->GetBufferPointer(), g_vsBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflector);
    if(FAILED(hr)) {
        Debug::Logger("ShaderD3D::","Cannot find 'CustomConstants' in the shader file");
        return false;
    }
    // ID3D11ShaderReflectionConstantBuffer *cb = reflector->GetConstantBufferByName("CustomConstants");
    ID3D11ShaderReflectionConstantBuffer *cb = nullptr;
    cb = reflector->GetConstantBufferByName("CustomConstants");
    if(cb != nullptr) {
        D3D11_SHADER_BUFFER_DESC desc;
        hr = cb->GetDesc(&desc);
        if(SUCCEEDED(hr)) {
            if(strcmp(desc.Name, "CustomConstants") == 0) {
                Debug::Logger("ShaderD3D::","Reflecting = ", desc.Name);
                for(int i = 0; i < desc.Variables; i++) {
                    ID3D11ShaderReflectionVariable *field = cb->GetVariableByIndex(i);
                    ID3D11ShaderReflectionType *fieldType = field->GetType();
                    D3D11_SHADER_VARIABLE_DESC vdesc;
                    D3D11_SHADER_TYPE_DESC tdesc;
                    field->GetDesc(&vdesc);
                    fieldType->GetDesc(&tdesc);
                    Debug::Logger("ShaderD3D::","Variable Name = ", vdesc.Name);
                    Debug::Logger("ShaderD3D::","Variable Type = ", tdesc.Name); 

                    GameResource::ShaderParamData param;
                    switch(tdesc.Type) {
                        case D3D_SVT_INT : 
                            {
                                int *value = (int *) vdesc.DefaultValue;
                                Debug::Logger("ShaderD3D","Variable value = ", *value);
                                param.dataType = GameResource::ShaderParamType::INTEGER;
                                param.value = *value;
                            } break;
                        case D3D_SVT_FLOAT : 
                            {
                                float *value = (float *) vdesc.DefaultValue;
                                Debug::Logger("ShaderD3D::","Variable value = ", *value);
                                param.dataType = GameResource::ShaderParamType::FLOATING;
                                param.value = *value;
                            } break;
                        default : {} break;
                    }
                    shaderMeta[vdesc.Name] = param;
                }
            }
        } else {
            Debug::Logger("ShaderD3D::","Cannot reflect 'CustomConstants', make sure it's not discarded by the compiler optimization");
        }
    } else {
        Debug::Logger("ShaderD3D::","Cannot find 'CustomConstants' in the shader file");
    }
    shader->parameterMeta = shaderMeta;
    Debug::Logger("ShaderD3D:: Constructed Shader ID : ", shader->id);
    return true;
}


bool Graphics::RemoveShader(GameResource::Shader *shader) {
    ShaderD3D *resource = static_cast<ShaderD3D*>(shader->resource.buffer);
    resource->pShader->Release();
    resource->vShader->Release();
    delete resource;
    return true;
}


bool Graphics::CreateTexture(GameResource::Texture *texture){
    TextureD3D *tex = new TextureD3D;
    tex->id = texture->id;
    HRESULT hr = ConstructD3DTexture(
        texture->filePath, 
        &tex->textureResource, 
        &tex->textureData, 
        &tex->textureSampler
        );
    if(FAILED(hr)){
        Debug::Logger("TextureD3D:: Init Texture Failed");
        return false;
    }
    D3D11_TEXTURE2D_DESC desc = GetTextureDescription(tex->textureData);
    texture->resource.buffer = tex;
    texture->resource.type = GraphicsResource::Type::TEXTURE_RESOURCE;
    texture->dimension.x = (float) desc.Width;
    texture->dimension.y = (float) desc.Height;
    Debug::Logger("TextureD3D:: Constructed Texture ID : ", texture->id);
    return true;
}


bool Graphics::RemoveTexture(GameResource::Texture *texture) {
    Debug::Logger("GraphicsD3D:: Free Texture ", texture->id);
    TextureD3D *resource = static_cast<TextureD3D*>(texture->resource.buffer);
    resource->textureData->Release();
    resource->textureResource->Release();
    resource->textureSampler->Release();
    delete resource;
    return true;
}


bool Graphics::CreateMaterial(GameResource::Material *material){
    MaterialD3D *mat = new MaterialD3D;
    mat->id = material->id;

    if(!material->shaderParameters.empty()) {
        UINT bufferSize;
        void *buffer = ConstructDynamicConstantBuffer(&material->shaderParameters, bufferSize);
        HRESULT hr = ConstructD3DConstantBuffer(buffer, bufferSize, true, &mat->customConstants);
        if(FAILED(hr)){
            Debug::Logger("MaterialD3D:: Fail registering shader parameters : ", material->id);
            return false;
        }
        free(buffer);
    }

    material->resource.buffer = mat;
    material->resource.type = GraphicsResource::Type::MATERIAL_RESOURCE;
    Debug::Logger("MaterialD3D:: Constructed Material ID : " , mat->id);
    return true;
}


bool Graphics::RemoveMaterial(GameResource::Material *material) {
    Debug::Logger("GraphicsD3D:: Free Material", material->id);
    MaterialD3D *mat = static_cast<MaterialD3D*>(material->resource.buffer);
    if(mat->customConstants) {
        mat->customConstants->Release();
    }
    delete mat;
    return true;
}


bool Graphics::CreateGeometry(GameObject::Sprite *sprite) {
    GeometryD3D *newGeom = new GeometryD3D;
    newGeom->id = sprite->attribute.id;
    newGeom->instanceType = GeomType::SPRITE;

    std::vector<Vector4> *v = &sprite->geometry.vertices;
    Vertex quad[6] = {
        {DirectX::XMFLOAT4((*v)[0].f), DirectX::XMFLOAT2(0.0f, 0.0f)},
        {DirectX::XMFLOAT4((*v)[1].f), DirectX::XMFLOAT2(1.0f, 0.0f)},
        {DirectX::XMFLOAT4((*v)[2].f), DirectX::XMFLOAT2(1.0f, 1.0f)},

        {DirectX::XMFLOAT4((*v)[2].f), DirectX::XMFLOAT2(1.0f, 1.0f)},
        {DirectX::XMFLOAT4((*v)[3].f), DirectX::XMFLOAT2(0.0f, 1.0f)},
        {DirectX::XMFLOAT4((*v)[0].f), DirectX::XMFLOAT2(0.0f, 0.0f)},
    };
   
    HRESULT hr = ConstructVertexBuffer(quad, sizeof(quad), &newGeom->vertexBuffer);
    if(FAILED(hr)){
        Debug::Logger("GeometryD3D:: fail create sprite vertexbuffers");
        return false;
    }
    sprite->geometry.mesh.buffer = newGeom; 
    sprite->geometry.mesh.type = GraphicsResource::Type::VERTEX_RESOURCE;
    Debug::Logger("GeometryD3D:: Constructed Geometry ID : ", sprite->attribute.id);
    return true;
}


bool Graphics::RemoveGeometry(GameObject::Sprite *sprite) {
    Debug::Logger("GraphicsD3D:: Free Sprite Geometry", sprite->attribute.id);
    GeometryD3D *geom = static_cast<GeometryD3D*>(sprite->geometry.mesh.buffer);
    geom->vertexBuffer->Release();
    delete geom;
    Debug::Logger("GraphicsD3D:: Free Instance");
    return true;
}


bool Graphics::CreateGeometry(GameObject::Text *text) {
    GeometryD3D* newGeom = new GeometryD3D();
    newGeom->id = text->attribute.id;
    newGeom->instanceType = GeomType::TEXT;
    TextureD3D* newTexture = new TextureD3D();
    newTexture->id = text->attribute.id;

    Vertex fontQuad[6] = {
        {DirectX::XMFLOAT4(text->geometry.vertices[0].f), DirectX::XMFLOAT2(0.0f, 0.0f)},
        {DirectX::XMFLOAT4(text->geometry.vertices[1].f), DirectX::XMFLOAT2(1.0f, 0.0f)},
        {DirectX::XMFLOAT4(text->geometry.vertices[2].f), DirectX::XMFLOAT2(1.0f, 1.0f)},
        {DirectX::XMFLOAT4(text->geometry.vertices[2].f), DirectX::XMFLOAT2(1.0f, 1.0f)},
        {DirectX::XMFLOAT4(text->geometry.vertices[3].f), DirectX::XMFLOAT2(0.0f, 1.0f)},
        {DirectX::XMFLOAT4(text->geometry.vertices[0].f), DirectX::XMFLOAT2(0.0f, 0.0f)},
    };

    HRESULT hr = ConstructVertexBuffer(fontQuad, sizeof(fontQuad), &newGeom->vertexBuffer);
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Test Font Fail Construct Vertex Buffer ");
        return false;
    }

    // TODO: You can move font shader initialization to init()
    if(!g_vShader[GeomType::TEXT] || !g_pShader[GeomType::TEXT]) {
        hr = ConstructD3DShader("./resources/shaders/font.hlsl", &g_vShader[GeomType::TEXT], &g_pShader[GeomType::TEXT]);
        if(FAILED(hr)) {
            Debug::Logger("GraphicsD3D:: Fail Construct Font1 Shader");
            return false;
        }
    }

    UINT qualityLevels = 0;
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 1, &qualityLevels);
    Debug::Logger("QUALITY LEVELS", qualityLevels);

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Width = text->width;
    textureDesc.Height = text->height;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureSubresource;
    ZeroMemory(&textureSubresource, sizeof(D3D11_SUBRESOURCE_DATA));
    textureSubresource.pSysMem = text->surfaceBuffer;
    textureSubresource.SysMemPitch = text->width * sizeof(RGBA); // uint8_t  or rgba
    hr = device->CreateTexture2D(
        &textureDesc,
        &textureSubresource,
        (ID3D11Texture2D **)(&newTexture->textureData)
        );
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Test Font Fail Create ID3DTexture2D ");
        return false;
    };
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D = {0, 1};
    hr = device->CreateShaderResourceView(newTexture->textureData, &shaderResourceViewDesc, &newTexture->textureResource);
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Test Font Fail Create ID3DTexture2D ShaderResourceView");
        return false;
    };

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&samplerDesc, &newTexture->textureSampler);
    if(FAILED(hr)){
        Debug::Logger("Fail to create texture sampler");
        return hr;
    }

    // Prepare font Constant Buffers
    ID3D11Buffer *cBuffer;
    struct alignas(64) {
        float scaleFactor;
    } fontConstants = {((float)text->size)/text->font->fontResource->size};
    hr = ConstructD3DConstantBuffer(&fontConstants, sizeof(fontConstants), true, &cBuffer);

    text->geometry.mesh.buffer = newGeom;
    text->geometry.mesh.type = GraphicsResource::Type::VERTEX_RESOURCE;
    text->textureResource.buffer = newTexture;
    text->textureResource.type = GraphicsResource::Type::TEXTURE_RESOURCE;
    text->constantBuffers.buffer = cBuffer;
    text->constantBuffers.type = GraphicsResource::Type::CONSTANT_BUFFER_RESOURCE;
    

    return true;
}


bool Graphics::RemoveGeometry(GameObject::Text *text) {
    Debug::Logger("GraphicsD3D:: Free Text Geometry", text->attribute.id);
    GeometryD3D *geom = static_cast<GeometryD3D*>(text->geometry.mesh.buffer);
    TextureD3D *tex = static_cast<TextureD3D*>(text->textureResource.buffer);
    ID3D11Buffer *constantBuffer = static_cast<ID3D11Buffer*>(text->constantBuffers.buffer);
    tex->textureResource->Release();
    tex->textureData->Release();
    tex->textureSampler->Release();
    geom->vertexBuffer->Release();
    constantBuffer->Release();
    delete geom;
    delete tex;
    delete constantBuffer;
    Debug::Logger("GraphicsD3D:: Free Instance");
    return true;
}


bool Graphics::CreateGeometry(DebugDraw::Text *text) {
    GeometryD3D* newGeom = new GeometryD3D();
    newGeom->id = text->attribute.id;
    newGeom->instanceType = GeomType::TEXT;
    TextureD3D* newTexture = new TextureD3D();
    newTexture->id = text->attribute.id;

    float hw = text->width / 2.0f;
    float hh = text->height / 2.0f;
    Vertex fontQuad[6] = {
        {DirectX::XMFLOAT4(-hw, hh, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},
        {DirectX::XMFLOAT4(hw, hh, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f)},
        {DirectX::XMFLOAT4(hw, -hh, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f)},
        {DirectX::XMFLOAT4(hw, -hh, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f)},
        {DirectX::XMFLOAT4(-hw, -hh, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f)},
        {DirectX::XMFLOAT4(-hw, hh, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},
    };

    HRESULT hr = ConstructVertexBuffer(fontQuad, sizeof(fontQuad), &newGeom->vertexBuffer);
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Test Font Fail Construct Vertex Buffer ");
        return false;
    }

    if(!g_vShader[GeomType::TEXT] || !g_pShader[GeomType::TEXT]) {
        hr = ConstructD3DShader("./resources/shaders/font.hlsl", &g_vShader[GeomType::TEXT], &g_pShader[GeomType::TEXT]);
        if(FAILED(hr)) {
            Debug::Logger("GraphicsD3D:: Fail Construct Font1 Shader");
            return false;
        }
    }

    UINT qualityLevels = 0;
    device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 1, &qualityLevels);
    Debug::Logger("QUALITY LEVELS", qualityLevels);

    Debug::Logger("Creating Debug Text Width : ",text->width, " Height : ", text->height );

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Width = text->width;
    textureDesc.Height = text->height;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = qualityLevels - 1;
    textureDesc.ArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureSubresource;
    ZeroMemory(&textureSubresource, sizeof(D3D11_SUBRESOURCE_DATA));
    textureSubresource.pSysMem = text->surfaceBuffer;
    textureSubresource.SysMemPitch = text->width * sizeof(RGBA); // uint8_t  or rgba
    hr = device->CreateTexture2D(
        &textureDesc,
        &textureSubresource,
        (ID3D11Texture2D **)(&newTexture->textureData)
        );
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Test Font Fail Create ID3DTexture2D ");
        return false;
    };
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D = {0, 1};
    hr = device->CreateShaderResourceView(newTexture->textureData, &shaderResourceViewDesc, &newTexture->textureResource);
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Test Font Fail Create ID3DTexture2D ShaderResourceView");
        return false;
    };

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&samplerDesc, &newTexture->textureSampler);
    if(FAILED(hr)){
        Debug::Logger("Fail to create texture sampler");
        return hr;
    }

    // Prepare font Constant Buffers
    ID3D11Buffer *cBuffer;
    struct alignas(64) {
        float scaleFactor;
    } fontConstants = {text->scaleFactor};
    hr = ConstructD3DConstantBuffer(&fontConstants, sizeof(fontConstants), true, &cBuffer);

    // assign
    text->vertexResource.buffer = newGeom;
    text->vertexResource.type = GraphicsResource::Type::VERTEX_RESOURCE;
    text->textureResource.buffer = newTexture;
    text->textureResource.type = GraphicsResource::Type::TEXTURE_RESOURCE;
    text->constantBuffers.buffer = cBuffer;
    text->constantBuffers.type = GraphicsResource::Type::CONSTANT_BUFFER_RESOURCE;

    return true;
}


bool Graphics::RemoveGeometry(DebugDraw::Text *text) {
    Debug::Logger("GraphicsD3D:: Free Text Geometry", text->attribute.id);
    GeometryD3D *geom = static_cast<GeometryD3D*>(text->vertexResource.buffer);
    TextureD3D *tex = static_cast<TextureD3D*>(text->textureResource.buffer);
    ID3D11Buffer *constantBuffer = static_cast<ID3D11Buffer*>(text->constantBuffers.buffer);
    tex->textureResource->Release();
    tex->textureData->Release();
    tex->textureSampler->Release();
    geom->vertexBuffer->Release();
    constantBuffer->Release();
    delete geom;
    delete tex;
    Debug::Logger("GraphicsD3D:: Free Instance");
    return true;
}


/* 
 * Draws
 * */


void Graphics::Draw(GameObject::Sprite *sprite) {
    GeometryD3D *instance = static_cast<GeometryD3D*>(sprite->geometry.mesh.buffer);
    MaterialD3D *mat = static_cast<MaterialD3D*>(sprite->material->resource.buffer);
    TextureD3D *tex = static_cast<TextureD3D*>(sprite->material->mainTexture->resource.buffer);
    ShaderD3D *shader = static_cast<ShaderD3D*>(sprite->material->shader->resource.buffer);

    // Update global constants
    localConstants.world = DirectX::XMMATRIX(sprite->transform.World.f); 
    UpdateConstantBuffers(g_lcBuffer, &localConstants, sizeof(localConstants));

    // Update local constants
    UINT strides = sizeof(Vertex);
    UINT offsets = 0;
    deviceContext->IASetVertexBuffers(0, 1, &instance->vertexBuffer, &strides, &offsets);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(g_inputLayout[instance->instanceType]);
    deviceContext->VSSetShader(shader->vShader, 0, 0);
    deviceContext->VSSetConstantBuffers(4, 1, &mat->customConstants);
    deviceContext->PSSetShader(shader->pShader, 0, 0);
    deviceContext->PSSetShaderResources(0, 1, &tex->textureResource);
    deviceContext->PSSetSamplers(0, 1, &tex->textureSampler);
    deviceContext->PSSetConstantBuffers(4, 1, &mat->customConstants);

    deviceContext->Draw(6, 0);
    // if(sprite->geometry.showBoundingRect) {
        Draw(sprite->geometry.AABB);
    // }
}


void Graphics::Draw(GameObject::AnimatedSprite *animatedSprite) {
    GameObject::Sprite *sprite = &animatedSprite->sprite;
    GeometryD3D *instance = static_cast<GeometryD3D*>(sprite->geometry.mesh.buffer);
    MaterialD3D *mat      = static_cast<MaterialD3D*>(sprite->material->resource.buffer);
    TextureD3D *tex       = static_cast<TextureD3D*>(sprite->material->mainTexture->resource.buffer);
    ShaderD3D *shader     = static_cast<ShaderD3D*>(sprite->material->shader->resource.buffer);

    // Update UVs
    UINT frameAdvance = std::round(EnginePlatformAPI::GetTargetFPS() / animatedSprite->fps);
    BOOL canUpdateFrame = (animatedSprite->localFrameCount % frameAdvance) == 0; 
    if(canUpdateFrame) {
        animatedSprite->currentFrame = (animatedSprite->currentFrame + 1) % animatedSprite->totalFrames;

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
        deviceContext->Map(instance->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        Vector2 maxUV = animatedSprite->frameDimensionNormalized;
        Vector2 minUV = {
            ((float) (animatedSprite->currentFrame % animatedSprite->pitch)) * animatedSprite->frameDimensionNormalized.x,
            ((float) std::floor(animatedSprite->currentFrame / animatedSprite->pitch)) * animatedSprite->frameDimensionNormalized.y,
        };
        // Debug::Logger("CurrentFrame ; ", animatedSprite->currentFrame, "minUV:", CoreMath::VectorToString(minUV), "maxUV", CoreMath::VectorToString(maxUV));
        Vertex newTarget[6] = {
            {DirectX::XMFLOAT4(animatedSprite->sprite.geometry.vertices[0].f), DirectX::XMFLOAT2(minUV.x, minUV.y)},
            {DirectX::XMFLOAT4(animatedSprite->sprite.geometry.vertices[1].f), DirectX::XMFLOAT2(minUV.x + maxUV.x, minUV.y)},
            {DirectX::XMFLOAT4(animatedSprite->sprite.geometry.vertices[2].f), DirectX::XMFLOAT2(minUV.x + maxUV.x, minUV.y + maxUV.y)},
            {DirectX::XMFLOAT4(animatedSprite->sprite.geometry.vertices[2].f), DirectX::XMFLOAT2(minUV.x + maxUV.x, minUV.y + maxUV.y)},
            {DirectX::XMFLOAT4(animatedSprite->sprite.geometry.vertices[3].f), DirectX::XMFLOAT2(minUV.x, minUV.y + maxUV.y)},
            {DirectX::XMFLOAT4(animatedSprite->sprite.geometry.vertices[0].f), DirectX::XMFLOAT2(minUV.x, minUV.y)},
        };
        memcpy(mappedResource.pData, newTarget, sizeof(newTarget));
        deviceContext->Unmap(instance->vertexBuffer, 0);
    }
    if(animatedSprite->localFrameCount >= EnginePlatformAPI::GetTargetFPS() - 1) {
        animatedSprite->localFrameCount = 0;
    }else{
        animatedSprite->localFrameCount++;
    }

    // Update global constants
    localConstants.world = DirectX::XMMATRIX(animatedSprite->sprite.transform.World.f); 
    UpdateConstantBuffers(g_lcBuffer, &localConstants, sizeof(localConstants));

    // Update local constants
    UINT strides = sizeof(Vertex);
    UINT offsets = 0;
    deviceContext->IASetVertexBuffers(0, 1, &instance->vertexBuffer, &strides, &offsets);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(g_inputLayout[instance->instanceType]);
    deviceContext->VSSetShader(shader->vShader, 0, 0);
    // deviceContext->VSSetConstantBuffers(4, 1, &mat->customConstants);
    deviceContext->PSSetShader(shader->pShader, 0, 0);
    deviceContext->PSSetShaderResources(0, 1, &tex->textureResource);
    deviceContext->PSSetSamplers(0, 1, &tex->textureSampler);
    deviceContext->PSSetConstantBuffers(4, 1, &mat->customConstants);

    deviceContext->Draw(6, 0);
    if(sprite->geometry.showBoundingRect) {
        Draw(sprite->geometry.AABB);
    }
}


void Graphics::Draw(GameObject::Text *text) {
    GeometryD3D *instance = static_cast<GeometryD3D*>(text->geometry.mesh.buffer);
    TextureD3D *tex = static_cast<TextureD3D*>(text->textureResource.buffer);
    ID3D11Buffer *cb = (ID3D11Buffer*) text->constantBuffers.buffer;

    localConstants.world = DirectX::XMMATRIX(text->transform.World.f);
    UpdateConstantBuffers(g_lcBuffer, &localConstants, sizeof(localConstants));
    
    UINT strides = sizeof(Vertex);
    UINT offsets = 0;
    deviceContext->IASetVertexBuffers(0, 1, &instance->vertexBuffer , &strides, &offsets);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(g_inputLayout[GeomType::QUAD]);
    deviceContext->VSSetShader(g_vShader[GeomType::TEXT], 0, 0);
    deviceContext->PSSetShader(g_pShader[GeomType::TEXT], 0, 0);
    deviceContext->PSSetShaderResources(1, 1, &tex->textureResource);
    deviceContext->PSSetSamplers(0, 1, &tex->textureSampler);
    deviceContext->PSSetConstantBuffers(3, 1, &cb);
    deviceContext->Draw(6, 0);

    if(text->geometry.showBoundingRect) {
        Draw(text->geometry.AABB);
    }

}


void Graphics::Draw(GameObject::Camera *cm) {
    if(!cm->geometry.showBoundingRect) return;
    LineVertex rect[5] = {
        {XMFLOAT4(cm->geometry.vertices[0].f), RED},
        {XMFLOAT4(cm->geometry.vertices[1].f), RED},
        {XMFLOAT4(cm->geometry.vertices[2].f), RED},
        {XMFLOAT4(cm->geometry.vertices[3].f), RED},
        {XMFLOAT4(cm->geometry.vertices[0].f), RED},
    };
    UpdateConstantBuffers(g_vertexBuffer[GeomType::QUAD], rect, sizeof(rect));

    // camera is drawn as bounding box, it doesn't requires translation
    XMMATRIX worldTemp = localConstants.world;
    XMMATRIX viewTemp = localConstants.view;
    localConstants.world = XMMatrixScaling(cm->transform.pos.z, cm->transform.pos.z, 1.0f);
    localConstants.view = DirectX::XMMatrixIdentity();
    UpdateConstantBuffers(g_lcBuffer, &localConstants, sizeof(localConstants));
    localConstants.world = worldTemp;
    localConstants.view = viewTemp;
    
    UINT strides = sizeof(LineVertex);
    UINT offsets = 0;
    deviceContext->IASetVertexBuffers(0, 1, &g_vertexBuffer[GeomType::QUAD], &strides, &offsets);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    deviceContext->IASetInputLayout(g_inputLayout[GeomType::LINE]);
    deviceContext->VSSetShader(g_vShader[GeomType::QUAD], 0, 0);
    deviceContext->PSSetShader(g_pShader[GeomType::QUAD], 0, 0);
    deviceContext->Draw(5, 0);

}


void Graphics::Draw(CoreGeometry::BoundingRect &aabb) {
    CoreMath::Vector2 min = aabb.bound.min;
    CoreMath::Vector2 max = aabb.bound.max;
    LineVertex rect[5] = {
        {DirectX::XMFLOAT4(min.x, max.y, 0.0f, 1.0f),BLUE}, 
        {DirectX::XMFLOAT4(max.x, max.y, 0.0f, 1.0f),BLUE},
        {DirectX::XMFLOAT4(max.x, min.y, 0.0f, 1.0f),GREEN},
        {DirectX::XMFLOAT4(min.x, min.y, 0.0f, 1.0f),GREEN},
        {DirectX::XMFLOAT4(min.x, max.y, 0.0f, 1.0f),BLUE}
    };

    localConstants.world = DirectX::XMMatrixIdentity();
    UpdateConstantBuffers(g_lcBuffer, &localConstants, sizeof(localConstants));
    UpdateConstantBuffers(g_vertexBuffer[GeomType::QUAD], rect, sizeof(rect));
    
    UINT strides = sizeof(LineVertex);
    UINT offsets = 0;
    deviceContext->IASetVertexBuffers(0, 1, &g_vertexBuffer[GeomType::QUAD], &strides, &offsets);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
    deviceContext->IASetInputLayout(g_inputLayout[GeomType::LINE]);
    deviceContext->VSSetShader(g_vShader[GeomType::QUAD], 0, 0);
    deviceContext->PSSetShader(g_pShader[GeomType::QUAD], 0, 0);
    deviceContext->Draw(5, 0);

}


void Graphics::Draw(DebugDraw::Text *text) {
    GeometryD3D *instance = static_cast<GeometryD3D*>(text->vertexResource.buffer);
    TextureD3D *tex = static_cast<TextureD3D*>(text->textureResource.buffer);
    ID3D11Buffer *cb = (ID3D11Buffer*) text->constantBuffers.buffer;

    // Update global constants
    localConstants.world = CreateWorldMatrix(
            DirectX::XMMatrixTranslation(text->pos.x, text->pos.y, 0.0f),
            DirectX::XMMatrixScaling(text->scale.x, text->scale.y, 0.0f),
            DirectX::XMMatrixRotationZ(0.0f)
            );
    DirectX::XMMATRIX tempView = localConstants.view;
    localConstants.view = DirectX::XMMatrixIdentity();
    UpdateConstantBuffers(g_lcBuffer, &localConstants, sizeof(localConstants));
    localConstants.view = tempView;
    
    UINT strides = sizeof(Vertex);
    UINT offsets = 0;
    deviceContext->IASetVertexBuffers(0, 1, &instance->vertexBuffer , &strides, &offsets);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->IASetInputLayout(g_inputLayout[GeomType::QUAD]);
    deviceContext->VSSetShader(g_vShader[GeomType::TEXT], 0, 0);
    deviceContext->PSSetShader(g_pShader[GeomType::TEXT], 0, 0);
    deviceContext->PSSetShaderResources(1, 1, &tex->textureResource);
    deviceContext->PSSetSamplers(0, 1, &tex->textureSampler);
    deviceContext->PSSetConstantBuffers(3, 1, &cb);
    deviceContext->Draw(6, 0);

}


/* 
 * Helpers 
 * */


bool Graphics::UpdateMaterialParameters(GameResource::Material **material) {
    UINT bufferSize;
    GameResource::ShaderParams *params = &((*material)->shaderParameters);
    void* newLocalConstants = ConstructDynamicConstantBuffer(params, bufferSize);
    GraphicsResource resource = (*material)->resource;
    if(resource.buffer == nullptr){
        Debug::Logger("GraphicsD3D:: empty material resource");
        return false;
    } 
    MaterialD3D *matd3d = static_cast<MaterialD3D*>(resource.buffer);
    UpdateConstantBuffers(matd3d->customConstants, newLocalConstants, bufferSize);
    free(newLocalConstants);
    return true;
};


void Graphics::UpdateViewProjectionMatrix(GameObject::Camera *camera) {
    localConstants.view = DirectX::XMMATRIX(camera->view.f);
    localConstants.projection = CreateProjectionMatrix(camera->transform.pos.z);
    UpdateConstantBuffers(g_lcBuffer, &localConstants, sizeof(localConstants));
}


bool Graphics::UpdateTexture(GraphicsResource &textureResource, GraphicsResource &vertexResource, void *data, uint32_t width, uint32_t height) {
    // Updates texture resource that will differ in sizes
    // which requires entire shaderresourceview update
    // the more performant sibling of this function is UpdateStaticTexture(..)
    // which updates only the texture buffer itself (assumed that the size is constant)
    
    /*
        Update Vertex Buffer
    */
    float hw = width / 2.0f;
    float hh = height / 2.0f;
    Vertex fontQuad[6] = {
        {DirectX::XMFLOAT4(-hw, hh, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},
        {DirectX::XMFLOAT4(hw, hh, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 0.0f)},
        {DirectX::XMFLOAT4(hw, -hh, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f)},
        {DirectX::XMFLOAT4(hw, -hh, 0.0f, 1.0f), DirectX::XMFLOAT2(1.0f, 1.0f)},
        {DirectX::XMFLOAT4(-hw, -hh, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 1.0f)},
        {DirectX::XMFLOAT4(-hw, hh, 0.0f, 1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)},
    };
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    GeometryD3D *vert = (GeometryD3D*) vertexResource.buffer;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    if(FAILED(deviceContext->Map(vert->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        Debug::Logger("GraphicsD3D:: Error Mapping vertex buffer", GetLastError());
    }
    memcpy(mappedResource.pData, fontQuad, sizeof(fontQuad));
    deviceContext->Unmap(vert->vertexBuffer, 0);

    /*
        Update Texture data / buffer
    */
    TextureD3D *tex = (TextureD3D*) textureResource.buffer;
    ID3D11Texture2D *tex2D = (ID3D11Texture2D*) tex->textureData;
    ID3D11Texture2D *newTex2D;
    D3D11_TEXTURE2D_DESC textureDesc;
    tex2D->GetDesc(&textureDesc);
    textureDesc.Width = width;
    textureDesc.Height = height;
    HRESULT hr = device->CreateTexture2D(&textureDesc, 0, &newTex2D);
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Fail create new tex description");
        return false;
    }
    tex->textureData->Release();
    tex->textureResource->Release();
    tex->textureData = newTex2D;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D = {0, 1};
    hr = device->CreateShaderResourceView(tex->textureData, &shaderResourceViewDesc, &tex->textureResource);
    if(FAILED(hr)) {
        Debug::Logger("GraphicsD3D:: Fail create ShaderResourceView");
        return false;
    };

    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    if(FAILED(deviceContext->Map(tex->textureData, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        Debug::Logger("GraphicsD3D:: Error Mapping texture buffer", GetLastError());
    }
    RGBA* ptr = (RGBA*) data;
    uint8_t *pDataPtr = (uint8_t*) mappedResource.pData;
    for(int i = 0; i < height; i++) {
        memcpy(pDataPtr, ptr, sizeof(RGBA) * width);
        pDataPtr += mappedResource.RowPitch;
        ptr += width;
    }
    deviceContext->Unmap(tex->textureData, 0);

    return true;
}


bool Graphics::UpdateStaticTexture(GraphicsResource &textureResource, void *data, uint32_t boxW, uint32_t boxH) {
    TextureD3D *tex = (TextureD3D*) textureResource.buffer;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    if(FAILED(deviceContext->Map(tex->textureData, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
        Debug::Logger("GraphicsD3D:: Error mapping texture buffer", GetLastError());
        return false;
    }
    RGBA *ptr = (RGBA *) data;
    uint8_t* pDataPtr = (uint8_t*) mappedResource.pData;
    for(int i = 0; i < boxH; i++) {
        memcpy(pDataPtr, ptr, sizeof(RGBA) * boxW);
        pDataPtr += mappedResource.RowPitch;
        ptr += boxW;
    }
    deviceContext->Unmap(tex->textureData, 0);
    return true;
}


CoreMath::Vector2 Graphics::GetScreenDimension() {
    // Graphics_D3D::GetWindowSize(hwnd, &wd);
    return CoreMath::CreateVector2((float) wd.x, (float) wd.y); 
}

