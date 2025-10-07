#include "ShadowMap.h"

namespace DXE {
    ShadowMap::ShadowMap(UINT width, UINT height, Shader* _shader)
        : width(width), height(height), shader(_shader)
    {
        CreateResources();

        // Setup viewport
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<FLOAT>(width);
        viewport.Height = static_cast<FLOAT>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
    }

    void ShadowMap::CreateResources() {
        // Depth texture
        D3D11_TEXTURE2D_DESC shadowTexDesc = {};
        shadowTexDesc.Width = width;
        shadowTexDesc.Height = height;
        shadowTexDesc.MipLevels = 1;
        shadowTexDesc.ArraySize = 1;
        shadowTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        shadowTexDesc.SampleDesc.Count = 1;
        shadowTexDesc.Usage = D3D11_USAGE_DEFAULT;
        shadowTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

        DXE::Renderer::Device()->CreateTexture2D(&shadowTexDesc, nullptr, &shadowTexture);

        // DSV
        D3D11_DEPTH_STENCIL_VIEW_DESC shadowDsvDesc = {};
        shadowDsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        shadowDsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        shadowDsvDesc.Texture2D.MipSlice = 0;

        DXE::Renderer::Device()->CreateDepthStencilView(shadowTexture.Get(), &shadowDsvDesc, &shadowDSV);

        // SRV
        D3D11_SHADER_RESOURCE_VIEW_DESC shadowSrvDesc = {};
        shadowSrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        shadowSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shadowSrvDesc.Texture2D.MipLevels = 1;

        DXE::Renderer::Device()->CreateShaderResourceView(shadowTexture.Get(), &shadowSrvDesc, &shadowSRV);


        // Comparison Sampler (for hardware PCF)
        D3D11_SAMPLER_DESC shadowSampDesc = {};
        shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
        shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSampDesc.BorderColor[0] = 1.0f; // Outside the shadow map ? lit
        shadowSampDesc.BorderColor[1] = 1.0f;
        shadowSampDesc.BorderColor[2] = 1.0f;
        shadowSampDesc.BorderColor[3] = 1.0f;
        shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        shadowSampDesc.MinLOD = 0;
        shadowSampDesc.MaxLOD = D3D11_FLOAT32_MAX;

        DXE::Renderer::Device()->CreateSamplerState(&shadowSampDesc, &shadowSampler);




        //// normals
        //
        //D3D11_TEXTURE2D_DESC normalDesc = {};
        //normalDesc.Width = width;
        //normalDesc.Height = height;
        //normalDesc.MipLevels = 1;
        //normalDesc.ArraySize = 1;
        //normalDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        //normalDesc.SampleDesc.Count = 1;
        //normalDesc.Usage = D3D11_USAGE_DEFAULT;
        //normalDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        //
        //DXE::Renderer::Device()->CreateTexture2D(&normalDesc, nullptr, &normalTexture);
        //
        //// RTV
        //D3D11_RENDER_TARGET_VIEW_DESC normalRtvDesc = {};
        //normalRtvDesc.Format = normalDesc.Format;
        //normalRtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        //normalRtvDesc.Texture2D.MipSlice = 0;
        //DXE::Renderer::Device()->CreateRenderTargetView(normalTexture.Get(), &normalRtvDesc, &normalRTV);
        //
        //// SRV
        //D3D11_SHADER_RESOURCE_VIEW_DESC normalSrvDesc = {};
        //normalSrvDesc.Format = normalDesc.Format;
        //normalSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        //normalSrvDesc.Texture2D.MipLevels = 1;
        //DXE::Renderer::Device()->CreateShaderResourceView(normalTexture.Get(), &normalSrvDesc, &normalSRV);
        //
        // default sampler
        D3D11_SAMPLER_DESC defaultSampDesc = {};
        defaultSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // regular linear filtering
        defaultSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;   // clamp to edge to avoid wrapping
        defaultSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        defaultSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        defaultSampDesc.MinLOD = 0;
        defaultSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        defaultSampDesc.MipLODBias = 0.0f;
        defaultSampDesc.MaxAnisotropy = 1;
        defaultSampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;  // not used
        defaultSampDesc.BorderColor[0] = 0.0f;
        defaultSampDesc.BorderColor[1] = 0.0f;
        defaultSampDesc.BorderColor[2] = 0.0f;
        defaultSampDesc.BorderColor[3] = 0.0f;
        
        DXE::Renderer::Device()->CreateSamplerState(&defaultSampDesc, &defaultSampler);
    }

    void ShadowMap::BeginRender()
    {
        // Set null render target — we are only writing depth
        shader->Bind();
        DXE::Renderer::Context()->OMSetRenderTargets(0, nullptr, shadowDSV.Get());

        // ID3D11RenderTargetView* rtvs[] = { normalRTV.Get() };
        // DXE::Renderer::Context()->OMSetRenderTargets(1, rtvs, shadowDSV.Get());


        DXE::Renderer::Context()->RSSetViewports(1, &viewport);
        DXE::Renderer::Context()->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
 

    }

    void ShadowMap::EndRender()
    {
        // Unbind shadow DSV
        DXE::Renderer::Context()->OMSetRenderTargets(0, nullptr, nullptr);
    }
}