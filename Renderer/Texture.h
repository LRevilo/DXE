#pragma once
#include "DXE.h"

#include "Renderer.h"
#include <wrl/client.h>
#include <string>
#include <vector>
#include <stdexcept> 

namespace DXE
{

    class DXE_API Texture
    {
    public:
        Texture();
        Texture(int width, int height, int channels);
        ~Texture();


        ID3D11ShaderResourceView* GetShaderResourceView() const { return m_ShaderResourceView.Get(); }
        ID3D11RenderTargetView* GetRenderTargetView() const { return m_RenderTargetView.Get(); }
        ID3D11Texture2D* GetTexture() const { return m_Texture.Get(); }


        int Width() const { return m_Width; }
        int Height() const { return m_Height; }
        int Channels() const { return m_Channels; }

        std::vector<unsigned char>& Pixels() { return m_PixelData; } // Return a reference to the pixel data vector
        std::vector<unsigned char*> GetPixel(int x, int y);

        bool LoadFromFile(const std::string& filename);
        void UpdateTexture();


        DXGI_FORMAT GetFormatFromChannels(int channels) {
            switch (channels)
            {
            case 1: return DXGI_FORMAT_R8_UNORM; // Grayscale
            case 2: return DXGI_FORMAT_R8G8_UNORM;  // Grayscale + Alpha
            case 3: return DXGI_FORMAT_B8G8R8X8_UNORM; // RGB
            case 4: return DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA
            default: return DXGI_FORMAT_UNKNOWN; // Invalid channel count
            }
        }
        void Resize(int newWidth, int newHeight, int newChannels = -1);

    private:
        std::vector<unsigned char> m_PixelData;
        int m_Width, m_Height;
        int m_Channels;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
    };
}