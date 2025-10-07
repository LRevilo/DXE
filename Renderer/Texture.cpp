#include "pch.h"
#include "Texture.h"
#include "image_utils.h"
//#include "stb_image.h"
namespace DXE
{

    Texture::Texture() :
        m_Width(0), m_Height(0), m_Channels(0)
    {
    };

    Texture::Texture(int width, int height, int channels) :
        m_Width(width), m_Height(height), m_Channels(channels)
    {
        m_PixelData.resize(m_Width * m_Height * m_Channels);
        std::fill(m_PixelData.begin(), m_PixelData.end(), 0);

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = GetFormatFromChannels(m_Channels);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = m_PixelData.data();;
        initData.SysMemPitch = m_Width * m_Channels;

        HRESULT hr = Renderer::Device()->CreateTexture2D(&desc, &initData, m_Texture.GetAddressOf());
        assert(SUCCEEDED(hr));


        hr = Renderer::Device()->CreateShaderResourceView(m_Texture.Get(), nullptr, m_ShaderResourceView.GetAddressOf());
        assert(SUCCEEDED(hr));
    };

    void Texture::Resize(int newWidth, int newHeight, int newChannels ) {

        if (!m_Texture) { 
            m_PixelData.resize(newWidth * newHeight * newChannels);
            std::fill(m_PixelData.begin(), m_PixelData.end(), 0);

            D3D11_TEXTURE2D_DESC desc = {};
            desc.Width = newWidth;
            desc.Height = newHeight;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = GetFormatFromChannels(m_Channels);
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = m_PixelData.data();;
            initData.SysMemPitch = m_Width * ((desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM) ? 4 : m_Channels);

            HRESULT hr = Renderer::Device()->CreateTexture2D(&desc, &initData, m_Texture.GetAddressOf());
            assert(SUCCEEDED(hr));


            hr = Renderer::Device()->CreateShaderResourceView(m_Texture.Get(), nullptr, m_ShaderResourceView.GetAddressOf());
            assert(SUCCEEDED(hr));
            return;
        }

        D3D11_TEXTURE2D_DESC desc = {};
        m_Texture->GetDesc(&desc);

        if (newChannels < 0) newChannels = m_Channels;

        m_Width = newWidth;
        m_Height = newHeight;
        m_Channels = newChannels;

        // Resize CPU pixel buffer
        m_PixelData.resize(m_Width * m_Height * m_Channels);

        // Release old GPU resources
        m_ShaderResourceView.Reset();
        m_RenderTargetView.Reset();
        m_Texture.Reset();

        desc.Width = m_Width;
        desc.Height = m_Height;
        desc.Format = GetFormatFromChannels(m_Channels); // important if channels changed

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = m_PixelData.data();
        initData.SysMemPitch = m_Width * ((desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM) ? 4 : m_Channels);

        HRESULT hr = Renderer::Device()->CreateTexture2D(&desc, &initData, m_Texture.GetAddressOf());
        assert(SUCCEEDED(hr));

        hr = Renderer::Device()->CreateShaderResourceView(m_Texture.Get(), nullptr, m_ShaderResourceView.GetAddressOf());
        assert(SUCCEEDED(hr));

        // Recreate RTV if texture supports it
        if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
        {
            hr = Renderer::Device()->CreateRenderTargetView(m_Texture.Get(), nullptr, m_RenderTargetView.GetAddressOf());
            assert(SUCCEEDED(hr));
        }
    }
    Texture::~Texture() {};

    bool Texture::LoadFromFile(const std::string& filename) {
        unsigned char* data = stb_LoadImage(filename.c_str(), &m_Width, &m_Height, &m_Channels, 0);
        if (!data) return false;

        m_PixelData.assign(data, data + (m_Width * m_Height * m_Channels));
        stb_FreeImage(data);

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = m_Width;
        desc.Height = m_Height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = GetFormatFromChannels(m_Channels);
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = m_PixelData.data();;
        initData.SysMemPitch = m_Width * m_Channels;

        HRESULT hr = Renderer::Device()->CreateTexture2D(&desc, &initData, m_Texture.GetAddressOf());
        if (FAILED(hr)) return false;

        hr = Renderer::Device()->CreateShaderResourceView(m_Texture.Get(), nullptr, m_ShaderResourceView.GetAddressOf());
        return SUCCEEDED(hr);
    }

    void Texture::UpdateTexture() {
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if (SUCCEEDED(Renderer::Context()->Map(m_Texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
        {
            memcpy(mappedResource.pData, m_PixelData.data(), m_PixelData.size());
            Renderer::Context()->Unmap(m_Texture.Get(), 0);
        }
    }


    std::vector<unsigned char*> Texture::GetPixel(int x, int y) {
        if (x < 0 || x >= m_Width || y < 0 || y >= m_Height)
            throw std::out_of_range("Pixel coordinates out of bounds");

        int pixelIndex = (y * m_Width + x) * m_Channels;

        // Create a vector of references to the pixel's channels
        std::vector<unsigned char*> pixelChannels;
        pixelChannels.reserve(m_Channels);

        for (int i = 0; i < m_Channels; ++i) {
            pixelChannels.push_back(&m_PixelData[pixelIndex + i]);
        }

        return pixelChannels;
    }
}