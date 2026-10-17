// Forge runtime: <DirectX> implementation (Direct3D 11). Compiled into games that #include <DirectX>.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#undef LoadImage  // windows.h renames it; we want raylib's

#include "forge_directx.hpp"
#include "native_window.inl"

extern "C" {
FrgRlImage LoadImage(const char* fileName);
void UnloadImage(FrgRlImage image);
void ImageFormat(FrgRlImage* image, int newFormat);
}

namespace frg::DirectX {

namespace {

template <class T>
void release(T*& p) {
    if (p) p->Release();
    p = nullptr;
}

struct VertexShader {
    ID3D11VertexShader* shader;
    ID3DBlob* code;  // kept for creating input layouts
};

// A texture that can be drawn into (createRenderTexture), with its own depth buffer.
struct RenderTarget {
    int32_t texture = 0;  // its handle in State::textures
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11Texture2D* depthTex = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;
    int32_t width = 0, height = 0;
};

struct State {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    IDXGISwapChain* swap = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11Texture2D* depthTex = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;
    ID3D11DepthStencilState* depthOn = nullptr;
    ID3D11DepthStencilState* depthOff = nullptr;
    ID3D11BlendState* blendOn = nullptr;
    ID3D11RasterizerState* raster[4] = {};  // by cull mode (1 none, 2 front, 3 back)
    ID3D11SamplerState* smooth = nullptr;
    ID3D11SamplerState* pixelated = nullptr;
    bool vsync = true;
    std::vector<VertexShader> vertexShaders;
    std::vector<ID3D11PixelShader*> pixelShaders;
    std::vector<ID3D11InputLayout*> layouts;
    std::vector<ID3D11Buffer*> buffers;
    std::vector<int32_t> bufferFloats;  // constant buffer sizes
    std::vector<ID3D11ShaderResourceView*> textures;
    std::vector<RenderTarget> targets;
    int32_t current = 0;               // the render texture being drawn into (0: the window)
    int32_t screenW = 0, screenH = 0;
};

State& st() {
    static State s;
    return s;
}

void need(const char* what) {
    if (!st().device) native::fail(what, "the window isn't open yet - call DirectX.openWindow(width, height, \"title\") first");
}

void check(HRESULT hr, const char* what, const char* doing) {
    if (FAILED(hr)) {
        char buf[32];
        std::snprintf(buf, sizeof buf, "0x%08lX", (unsigned long)hr);
        native::fail(what, std::string("Direct3D couldn't ") + doing + " (error " + buf + ")");
    }
}

// Handles are positions in these lists, counting from 1 (0 means "nothing").
template <class T>
T& lookup(std::vector<T>& v, int32_t handle, const char* what, const char* kind) {
    if (handle < 1 || handle > (int32_t)v.size()) native::fail(what, std::string("there's no ") + kind + " with handle " + str(handle));
    return v[handle - 1];
}

void present() { st().swap->Present(st().vsync ? 1 : 0, 0); }

void screenshot(const std::string& path) {
    State& s = st();
    ID3D11Texture2D* back = nullptr;
    if (FAILED(s.swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back))) return;
    D3D11_TEXTURE2D_DESC desc;
    back->GetDesc(&desc);
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;
    ID3D11Texture2D* staging = nullptr;
    if (SUCCEEDED(s.device->CreateTexture2D(&desc, nullptr, &staging))) {
        s.ctx->CopyResource(staging, back);
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(s.ctx->Map(staging, 0, D3D11_MAP_READ, 0, &mapped))) {
            std::vector<unsigned char> px((size_t)desc.Width * desc.Height * 4);
            for (UINT y = 0; y < desc.Height; y++)
                std::memcpy(&px[(size_t)y * desc.Width * 4], (unsigned char*)mapped.pData + (size_t)y * mapped.RowPitch, desc.Width * 4);
            s.ctx->Unmap(staging, 0);
            native::savePng(path, px.data(), (int)desc.Width, (int)desc.Height);
        }
        staging->Release();
    }
    back->Release();
}

ID3DBlob* compile(const std::string& hlsl, const std::string& entry, const char* target, const char* what) {
    ID3DBlob* code = nullptr;
    ID3DBlob* errors = nullptr;
    HRESULT hr = D3DCompile(hlsl.data(), hlsl.size(), "shader.hlsl", nullptr, nullptr, entry.c_str(), target,
                            D3DCOMPILE_ENABLE_STRICTNESS, 0, &code, &errors);
    if (FAILED(hr)) {
        std::string log = errors ? std::string((const char*)errors->GetBufferPointer(), errors->GetBufferSize()) : "unknown error";
        while (!log.empty() && (log.back() == '\0' || log.back() == '\n')) log.pop_back();
        release(errors);
        native::fail(what, "the shader has an error:\n" + log);
    }
    release(errors);
    return code;
}

}  // namespace

void openWindow_(int32_t width, int32_t height, const std::string& title) {
    native::open(width, height, title, &present, &screenshot);
    State& s = st();
    s.screenW = width;
    s.screenH = height;
    s.current = 0;

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 1;
    sd.BufferDesc.Width = (UINT)width;
    sd.BufferDesc.Height = (UINT)height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = (HWND)native::handle();
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd,
                                               &s.swap, &s.device, &level, &s.ctx);
    if (FAILED(hr))  // no suitable graphics card: fall back to Windows' software renderer
        hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &s.swap,
                                           &s.device, &level, &s.ctx);
    check(hr, "DirectX.openWindow()", "start up Direct3D 11");

    ID3D11Texture2D* back = nullptr;
    check(s.swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back), "DirectX.openWindow()", "get the window's image");
    check(s.device->CreateRenderTargetView(back, nullptr, &s.rtv), "DirectX.openWindow()", "draw into the window");
    back->Release();

    D3D11_TEXTURE2D_DESC dd{};
    dd.Width = (UINT)width;
    dd.Height = (UINT)height;
    dd.MipLevels = 1;
    dd.ArraySize = 1;
    dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dd.SampleDesc.Count = 1;
    dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    check(s.device->CreateTexture2D(&dd, nullptr, &s.depthTex), "DirectX.openWindow()", "make the depth buffer");
    check(s.device->CreateDepthStencilView(s.depthTex, nullptr, &s.dsv), "DirectX.openWindow()", "make the depth buffer");

    D3D11_DEPTH_STENCIL_DESC ds{};
    ds.DepthEnable = TRUE;
    ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    ds.DepthFunc = D3D11_COMPARISON_LESS;
    s.device->CreateDepthStencilState(&ds, &s.depthOn);
    ds.DepthEnable = FALSE;
    s.device->CreateDepthStencilState(&ds, &s.depthOff);

    D3D11_BLEND_DESC bd{};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    s.device->CreateBlendState(&bd, &s.blendOn);

    // Triangles count as front-facing when their corners go counter-clockwise (the same as OpenGL).
    for (int mode = 1; mode <= 3; mode++) {
        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = (D3D11_CULL_MODE)mode;
        rd.FrontCounterClockwise = TRUE;
        rd.DepthClipEnable = TRUE;
        s.device->CreateRasterizerState(&rd, &s.raster[mode]);
    }

    D3D11_SAMPLER_DESC smp{};
    smp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    smp.AddressU = smp.AddressV = smp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    smp.MaxLOD = D3D11_FLOAT32_MAX;
    smp.ComparisonFunc = D3D11_COMPARISON_NEVER;
    s.device->CreateSamplerState(&smp, &s.smooth);
    smp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    s.device->CreateSamplerState(&smp, &s.pixelated);

    D3D11_VIEWPORT vp{0, 0, (float)width, (float)height, 0, 1};
    s.ctx->RSSetViewports(1, &vp);
    s.ctx->OMSetRenderTargets(1, &s.rtv, s.dsv);
    s.ctx->OMSetDepthStencilState(s.depthOn, 0);
    s.ctx->RSSetState(s.raster[1]);
    s.ctx->PSSetSamplers(0, 1, &s.smooth);
    s.ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    addHook(shutdownHooks(), &closeWindow_);
}

void closeWindow_() {
    State& s = st();
    if (!s.device) return;
    s.ctx->ClearState();
    for (auto& v : s.vertexShaders) {
        release(v.shader);
        release(v.code);
    }
    for (auto* p : s.pixelShaders) p->Release();
    for (auto* l : s.layouts) l->Release();
    for (auto* b : s.buffers) b->Release();
    for (auto* t : s.textures) t->Release();
    for (auto& t : s.targets) {
        release(t.rtv);
        release(t.dsv);
        release(t.depthTex);
    }
    s.targets.clear();
    s.current = 0;
    s.vertexShaders.clear();
    s.pixelShaders.clear();
    s.layouts.clear();
    s.buffers.clear();
    s.textures.clear();
    for (auto*& r : s.raster) release(r);
    release(s.smooth);
    release(s.pixelated);
    release(s.blendOn);
    release(s.depthOn);
    release(s.depthOff);
    release(s.dsv);
    release(s.depthTex);
    release(s.rtv);
    release(s.swap);
    release(s.ctx);
    release(s.device);
    native::close();
}

void setVSync_(bool on) { st().vsync = on; }

std::string adapterName_() {
    need("DirectX.adapterName()");
    IDXGIDevice* dxgi = nullptr;
    IDXGIAdapter* adapter = nullptr;
    std::string name;
    if (SUCCEEDED(st().device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi)) && SUCCEEDED(dxgi->GetAdapter(&adapter))) {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        int n = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
        name.resize(n > 0 ? n - 1 : 0);
        WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, name.data(), n, nullptr, nullptr);
    }
    release(adapter);
    release(dxgi);
    return name;
}

void clear_(float r, float g, float b, float a) {
    need("DirectX.clear()");
    State& s = st();
    const float c[4] = {r, g, b, a};
    // whatever is being drawn into: the window, or a render texture
    ID3D11RenderTargetView* rtv = s.rtv;
    ID3D11DepthStencilView* dsv = s.dsv;
    for (auto& t : s.targets)
        if (t.texture == s.current) {
            rtv = t.rtv;
            dsv = t.dsv;
        }
    s.ctx->OMSetRenderTargets(1, &rtv, dsv);
    s.ctx->ClearRenderTargetView(rtv, c);
    s.ctx->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void setDepthTest_(bool on) {
    need("DirectX.setDepthTest()");
    st().ctx->OMSetDepthStencilState(on ? st().depthOn : st().depthOff, 0);
}

void setBlending_(bool on) {
    need("DirectX.setBlending()");
    const float factor[4] = {0, 0, 0, 0};
    st().ctx->OMSetBlendState(on ? st().blendOn : nullptr, factor, 0xffffffff);
}

void setCulling_(int32_t mode) {
    need("DirectX.setCulling()");
    if (mode < 1 || mode > 3) native::fail("DirectX.setCulling()", "use DirectX.CULL_NONE, DirectX.CULL_FRONT or DirectX.CULL_BACK");
    st().ctx->RSSetState(st().raster[mode]);
}

void setTopology_(int32_t topology) {
    need("DirectX.setTopology()");
    st().ctx->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
}

int32_t compileVertexShader_(const std::string& hlsl, const std::string& entry) {
    need("DirectX.compileVertexShader()");
    ID3DBlob* code = compile(hlsl, entry, "vs_5_0", "DirectX.compileVertexShader()");
    ID3D11VertexShader* vs = nullptr;
    check(st().device->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &vs),
          "DirectX.compileVertexShader()", "create the vertex shader");
    st().vertexShaders.push_back({vs, code});
    return (int32_t)st().vertexShaders.size();
}

int32_t compilePixelShader_(const std::string& hlsl, const std::string& entry) {
    need("DirectX.compilePixelShader()");
    ID3DBlob* code = compile(hlsl, entry, "ps_5_0", "DirectX.compilePixelShader()");
    ID3D11PixelShader* ps = nullptr;
    check(st().device->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &ps),
          "DirectX.compilePixelShader()", "create the pixel shader");
    code->Release();
    st().pixelShaders.push_back(ps);
    return (int32_t)st().pixelShaders.size();
}

void setShaders_(int32_t vertexShader, int32_t pixelShader) {
    need("DirectX.setShaders()");
    State& s = st();
    s.ctx->VSSetShader(lookup(s.vertexShaders, vertexShader, "DirectX.setShaders()", "vertex shader").shader, nullptr, 0);
    s.ctx->PSSetShader(lookup(s.pixelShaders, pixelShader, "DirectX.setShaders()", "pixel shader"), nullptr, 0);
}

// One element per input of the vertex shader, in order: a semantic name ("POSITION", "COLOR", "TEXCOORD")
// and how many floats it takes (1-4). Repeated names get index 0, 1, 2...
int32_t createInputLayout_(int32_t vertexShader, const List<std::string>& names, const List<int32_t>& sizes) {
    const char* what = "DirectX.createInputLayout()";
    need(what);
    State& s = st();
    if (names.size() != sizes.size()) native::fail(what, "names and sizes need the same number of items");
    VertexShader& vs = lookup(s.vertexShaders, vertexShader, what, "vertex shader");
    std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
    static const DXGI_FORMAT formats[] = {DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT,
                                          DXGI_FORMAT_R32G32B32A32_FLOAT};
    UINT offset = 0;
    for (size_t i = 0; i < names.size(); i++) {
        int n = sizes[i];
        if (n < 1 || n > 4) native::fail(what, "each size is how many floats an input takes: 1, 2, 3 or 4");
        UINT index = 0;
        for (size_t j = 0; j < i; j++)
            if (names[j] == names[i]) index++;
        elements.push_back({names[i].c_str(), index, formats[n - 1], 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0});
        offset += (UINT)n * 4;
    }
    ID3D11InputLayout* layout = nullptr;
    check(s.device->CreateInputLayout(elements.data(), (UINT)elements.size(), vs.code->GetBufferPointer(), vs.code->GetBufferSize(), &layout),
          what, "match the input layout to the vertex shader's inputs - check the names and sizes");
    s.layouts.push_back(layout);
    return (int32_t)s.layouts.size();
}

void setInputLayout_(int32_t layout) {
    need("DirectX.setInputLayout()");
    st().ctx->IASetInputLayout(lookup(st().layouts, layout, "DirectX.setInputLayout()", "input layout"));
}

namespace {
int32_t makeBuffer(const void* data, UINT bytes, UINT bind, D3D11_USAGE usage, UINT cpu, const char* what, int32_t floats) {
    need(what);
    if (bytes == 0) native::fail(what, "a buffer can't be empty");
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = bytes;
    bd.Usage = usage;
    bd.BindFlags = bind;
    bd.CPUAccessFlags = cpu;
    D3D11_SUBRESOURCE_DATA init{data, 0, 0};
    ID3D11Buffer* buf = nullptr;
    check(st().device->CreateBuffer(&bd, data ? &init : nullptr, &buf), what, "create the buffer");
    st().buffers.push_back(buf);
    st().bufferFloats.push_back(floats);
    return (int32_t)st().buffers.size();
}
}  // namespace

int32_t createVertexBuffer_(const List<float>& data) {
    return makeBuffer(data.data(), (UINT)(data.size() * 4), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE, 0,
                      "DirectX.createVertexBuffer()", (int32_t)data.size());
}

int32_t createIndexBuffer_(const List<int32_t>& indices) {
    return makeBuffer(indices.data(), (UINT)(indices.size() * 4), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE, 0,
                      "DirectX.createIndexBuffer()", (int32_t)indices.size());
}

int32_t createConstantBuffer_(int32_t floatCount) {
    int32_t rounded = (floatCount + 3) / 4 * 4;  // constant buffers come in blocks of 4 floats (16 bytes)
    return makeBuffer(nullptr, (UINT)(rounded * 4), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE,
                      "DirectX.createConstantBuffer()", rounded);
}

void updateConstantBuffer_(int32_t buffer, const List<float>& data) {
    const char* what = "DirectX.updateConstantBuffer()";
    need(what);
    ID3D11Buffer* buf = lookup(st().buffers, buffer, what, "buffer");
    int32_t capacity = st().bufferFloats[buffer - 1];
    if ((int32_t)data.size() > capacity)
        native::fail(what, "this buffer holds " + str(capacity) + " floats, but got " + str((int64_t)data.size()));
    D3D11_MAPPED_SUBRESOURCE mapped;
    check(st().ctx->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped), what, "write to the buffer (was it made with createConstantBuffer?)");
    std::memset(mapped.pData, 0, (size_t)capacity * 4);
    std::memcpy(mapped.pData, data.data(), data.size() * 4);
    st().ctx->Unmap(buf, 0);
}

void setVertexBuffer_(int32_t buffer, int32_t strideFloats) {
    need("DirectX.setVertexBuffer()");
    ID3D11Buffer* buf = lookup(st().buffers, buffer, "DirectX.setVertexBuffer()", "buffer");
    UINT stride = (UINT)strideFloats * 4, offset = 0;
    st().ctx->IASetVertexBuffers(0, 1, &buf, &stride, &offset);
}

void setIndexBuffer_(int32_t buffer) {
    need("DirectX.setIndexBuffer()");
    st().ctx->IASetIndexBuffer(lookup(st().buffers, buffer, "DirectX.setIndexBuffer()", "buffer"), DXGI_FORMAT_R32_UINT, 0);
}

void setConstantBuffer_(int32_t slot, int32_t buffer) {
    need("DirectX.setConstantBuffer()");
    ID3D11Buffer* buf = lookup(st().buffers, buffer, "DirectX.setConstantBuffer()", "buffer");
    st().ctx->VSSetConstantBuffers((UINT)slot, 1, &buf);
    st().ctx->PSSetConstantBuffers((UINT)slot, 1, &buf);
}

void draw_(int32_t vertexCount, int32_t startVertex) {
    need("DirectX.draw()");
    st().ctx->Draw((UINT)vertexCount, (UINT)startVertex);
}

void drawIndexed_(int32_t indexCount, int32_t startIndex) {
    need("DirectX.drawIndexed()");
    st().ctx->DrawIndexed((UINT)indexCount, (UINT)startIndex, 0);
}

int32_t loadTexture_(const std::string& path) {
    const char* what = "DirectX.loadTexture()";
    need(what);
    std::string file = native::findGameFile(path);
    if (file.empty()) native::fail(what, "can't find the file '" + path + "' - put it in the game's folder (next to main.frg)");
    FrgRlImage img = LoadImage(file.c_str());
    if (!img.data) native::fail(what, "couldn't read '" + path + "' as an image");
    ImageFormat(&img, 7);  // RGBA, 8 bits each
    D3D11_TEXTURE2D_DESC td{};
    td.Width = (UINT)img.width;
    td.Height = (UINT)img.height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_IMMUTABLE;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA init{img.data, (UINT)img.width * 4, 0};
    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = st().device->CreateTexture2D(&td, &init, &tex);
    UnloadImage(img);
    check(hr, what, "create the texture");
    ID3D11ShaderResourceView* view = nullptr;
    hr = st().device->CreateShaderResourceView(tex, nullptr, &view);
    tex->Release();
    check(hr, what, "create the texture view");
    st().textures.push_back(view);
    return (int32_t)st().textures.size();
}

void setTexture_(int32_t slot, int32_t texture) {
    need("DirectX.setTexture()");
    ID3D11ShaderResourceView* view = lookup(st().textures, texture, "DirectX.setTexture()", "texture");
    st().ctx->PSSetShaderResources((UINT)slot, 1, &view);
}

// A texture to draw into (setRenderTarget) and then use like any other (setTexture). It has its own depth buffer.
int32_t createRenderTexture_(int32_t width, int32_t height) {
    const char* what = "DirectX.createRenderTexture()";
    need(what);
    if (width <= 0 || height <= 0) native::fail(what, "the size has to be more than 0, but it's " + str(width) + " x " + str(height));
    State& s = st();
    D3D11_TEXTURE2D_DESC td{};
    td.Width = (UINT)width;
    td.Height = (UINT)height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    ID3D11Texture2D* tex = nullptr;
    check(s.device->CreateTexture2D(&td, nullptr, &tex), what, "create the render texture");
    RenderTarget t;
    t.width = width;
    t.height = height;
    ID3D11ShaderResourceView* view = nullptr;
    HRESULT hr = s.device->CreateShaderResourceView(tex, nullptr, &view);
    if (SUCCEEDED(hr)) hr = s.device->CreateRenderTargetView(tex, nullptr, &t.rtv);
    tex->Release();
    check(hr, what, "create the render texture's views");
    D3D11_TEXTURE2D_DESC dd = td;
    dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    check(s.device->CreateTexture2D(&dd, nullptr, &t.depthTex), what, "create the render texture's depth buffer");
    check(s.device->CreateDepthStencilView(t.depthTex, nullptr, &t.dsv), what, "create the render texture's depth view");
    s.textures.push_back(view);
    t.texture = (int32_t)s.textures.size();
    s.targets.push_back(t);
    return t.texture;
}

// Draws into this render texture from now on (0: the window again). The viewport is set to its size.
void setRenderTarget_(int32_t texture) {
    const char* what = "DirectX.setRenderTarget()";
    need(what);
    State& s = st();
    // a texture can't be read while it's drawn into: the textures given to the pixel shader are taken away
    ID3D11ShaderResourceView* none[8] = {};
    s.ctx->PSSetShaderResources(0, 8, none);
    if (texture == 0) {
        s.current = 0;
        s.ctx->OMSetRenderTargets(1, &s.rtv, s.dsv);
        D3D11_VIEWPORT vp{0, 0, (float)s.screenW, (float)s.screenH, 0, 1};
        s.ctx->RSSetViewports(1, &vp);
        return;
    }
    for (auto& t : s.targets)
        if (t.texture == texture) {
            s.current = texture;
            s.ctx->OMSetRenderTargets(1, &t.rtv, t.dsv);
            D3D11_VIEWPORT vp{0, 0, (float)t.width, (float)t.height, 0, 1};
            s.ctx->RSSetViewports(1, &vp);
            return;
        }
    native::fail(what, "texture " + str(texture) + " can't be drawn into - make one that can with DirectX.createRenderTexture(width, height) (0 is the window)");
}

void setTextureFilter_(bool smooth) {
    need("DirectX.setTextureFilter()");
    st().ctx->PSSetSamplers(0, 1, smooth ? &st().smooth : &st().pixelated);
}

}  // namespace frg::DirectX
