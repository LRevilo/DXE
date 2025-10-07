#pragma once
#include "DXE.h"
#include "Logger.h"

#include <d3d11_1.h>
#pragma comment(lib, "d3d11.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


#include <wrl/client.h>

#include <assert.h>
#include <string>

namespace DXE {

    class DXE_API Renderer {
       
    public: 
        static Renderer* s_Renderer;
        static Renderer* Get() { return s_Renderer; }
        static void Init(Renderer* renderer = nullptr);
        std::string name = "DXRenderer";



    private:
        HWND m_Hwnd;
        UINT m_ShaderCompileFlags;

        // DirectX 11 device and context
        ID3D11Device1* m_Device;
        ID3D11DeviceContext1* m_Context;
        ID3D11Debug* m_Debug;

        // Additional DirectX resources (swap chain, render target, etc.)
        IDXGISwapChain1* m_SwapChain;
        ID3D11RenderTargetView* m_FrameBufferView;
        ID3D11DepthStencilView* m_DepthBufferView;
        ID3D11RasterizerState* m_RasterizerState;
        ID3D11DepthStencilState* m_DepthStencilState;

        ID3D11SamplerState* m_SamplerState;

    public:
         inline static ID3D11Device1* Device() { return s_Renderer->m_Device; }
         inline static ID3D11DeviceContext1* Context() { return s_Renderer->m_Context; }

         inline static IDXGISwapChain1* SwapChain() { return s_Renderer->m_SwapChain; }
         inline static ID3D11RenderTargetView* FrameBufferView() { return s_Renderer->m_FrameBufferView; }
         inline static ID3D11DepthStencilView* DepthBufferView() { return s_Renderer->m_DepthBufferView; }
         inline static ID3D11RasterizerState* RasterizerState() { return s_Renderer->m_RasterizerState; }
         inline static ID3D11DepthStencilState* DepthStencilState() { return s_Renderer->m_DepthStencilState; }
         inline static ID3D11SamplerState* SamplerState() { return s_Renderer->m_SamplerState; }
         inline static HWND Hwnd() { return s_Renderer->m_Hwnd; }

         inline static UINT ShaderCompileFlags() { return s_Renderer->m_ShaderCompileFlags; }
         inline static void CheckCompileErrors(HRESULT hResult, ID3DBlob* compileErrors) {
             if (FAILED(hResult))
             {
                 const char* errorString = NULL;
                 if (hResult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                     errorString = "Could not compile shader; file not found";
                 else if (compileErrors) {
                     errorString = (const char*)compileErrors->GetBufferPointer();
                 }
                 MessageBoxA(0, errorString, "Shader Compiler Error", MB_ICONERROR | MB_OK);
             }
         }

    public:
         bool CreateDeviceAndContext() {
            ID3D11Device* baseDevice;
            ID3D11DeviceContext* baseDeviceContext;
            D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
            UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
            #if defined(DEBUG_BUILD)
                creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
            #endif

            HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
                0, creationFlags,
                featureLevels, ARRAYSIZE(featureLevels),
                D3D11_SDK_VERSION, &baseDevice,
                0, &baseDeviceContext);
            if (FAILED(hResult)) {
                MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
                return GetLastError();
            }

            // Get 1.1 interface of D3D11 Device and Context
            hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&m_Device);
            assert(SUCCEEDED(hResult));
            baseDevice->Release();

            hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&m_Context);
            assert(SUCCEEDED(hResult));
            baseDeviceContext->Release();


            #ifdef DEBUG_BUILD
                    // Set up debug layer to break on D3D11 errors
            
                    d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
                    if (d3dDebug)
                    {
                        ID3D11InfoQueue* d3dInfoQueue = nullptr;
                        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
                        {
                            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
                            d3dInfoQueue->Release();
                        }
                        d3dDebug->Release();
                    }
            #endif
            
                    m_ShaderCompileFlags = 0;
                    // Compiling with this flag allows debugging shaders with Visual Studio
            #if defined(DEBUG_BUILD)
                    shaderCompileFlags |= D3DCOMPILE_DEBUG;
            #endif

            return true;
        }
         bool CreateSwapChain() {
            // Get DXGI Factory (needed to create Swap Chain)
            IDXGIFactory2* dxgiFactory;
            {
                IDXGIDevice1* dxgiDevice;
                HRESULT hResult = m_Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
                assert(SUCCEEDED(hResult));

                IDXGIAdapter* dxgiAdapter;
                hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
                assert(SUCCEEDED(hResult));
                dxgiDevice->Release();

                DXGI_ADAPTER_DESC adapterDesc;
                dxgiAdapter->GetDesc(&adapterDesc);

                OutputDebugStringA("Graphics Device: ");
                OutputDebugStringW(adapterDesc.Description);

                hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
                assert(SUCCEEDED(hResult));
                dxgiAdapter->Release();
            }

            DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
            SwapChainDesc.Width = 0; // use window width
            SwapChainDesc.Height = 0; // use window height
            SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            SwapChainDesc.SampleDesc.Count = 1;
            SwapChainDesc.SampleDesc.Quality = 0;
            SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            SwapChainDesc.BufferCount = 2;
            SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            SwapChainDesc.Flags = 0;

            HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(m_Device, m_Hwnd, &SwapChainDesc, 0, 0, &m_SwapChain);
            assert(SUCCEEDED(hResult));

            dxgiFactory->Release();

            return true;
        }
         bool CreateRenderTargets() {
            ID3D11Texture2D* d3d11FrameBuffer;
            HRESULT hResult = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
            assert(SUCCEEDED(hResult));

            hResult = m_Device->CreateRenderTargetView(d3d11FrameBuffer, 0, &m_FrameBufferView);
            assert(SUCCEEDED(hResult));

            D3D11_TEXTURE2D_DESC depthBufferDesc;
            d3d11FrameBuffer->GetDesc(&depthBufferDesc);
            d3d11FrameBuffer->Release();

            depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

            ID3D11Texture2D* depthBuffer;
            hResult = m_Device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);
            assert(SUCCEEDED(hResult));

            hResult = m_Device->CreateDepthStencilView(depthBuffer, nullptr, &m_DepthBufferView);
            assert(SUCCEEDED(hResult));

            depthBuffer->Release();

            return true;
        }
         bool CreateSamplerState() {
            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
            samplerDesc.BorderColor[0] = 1.0f;
            samplerDesc.BorderColor[1] = 1.0f;
            samplerDesc.BorderColor[2] = 1.0f;
            samplerDesc.BorderColor[3] = 1.0f;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

            HRESULT hResult = m_Device->CreateSamplerState(&samplerDesc, &m_SamplerState);
            assert(SUCCEEDED(hResult));
            return true;
        }
         bool CreateRasterizerState() {
            D3D11_RASTERIZER_DESC rasterizerDesc = {};
            rasterizerDesc.FillMode = D3D11_FILL_SOLID;
            rasterizerDesc.CullMode = D3D11_CULL_BACK;
            rasterizerDesc.FrontCounterClockwise = false;

            HRESULT hResult = m_Device->CreateRasterizerState(&rasterizerDesc, &m_RasterizerState);
            assert(SUCCEEDED(hResult));
            return true;
        }
         bool CreateDepthStencilState() {
            D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
            depthStencilDesc.DepthEnable = TRUE;
            depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
            
            HRESULT hResult = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
            assert(SUCCEEDED(hResult));
            return true;
        }



         bool CleanupRenderTargets() {
             if (m_FrameBufferView) {
                 m_FrameBufferView->Release();
                 m_FrameBufferView = nullptr;
             }
             if (m_DepthBufferView) {
                 m_DepthBufferView->Release();
                 m_DepthBufferView = nullptr;
             }
             return true;
         }
         bool ResizeBuffers(UINT w, UINT h) {
             CleanupRenderTargets();
             m_SwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);
             CreateRenderTargets();
             return true;
         }

    public:

         bool InitState(HWND hwnd = nullptr) {
            m_Hwnd = hwnd;
            CreateDeviceAndContext();
            CreateSwapChain();
            CreateRenderTargets();
            CreateSamplerState();
            CreateRasterizerState();
            CreateDepthStencilState();
            return true;
        }



    };
}