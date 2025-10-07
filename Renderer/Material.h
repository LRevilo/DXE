#pragma once
#include "DXE.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
namespace DXE
{


    class DXE_API Material {
    public:
        // Base class constructor and destructor

        Material(const std::string& name, const std::string& shader);
        Material(const std::string& name);
        Material();
        virtual ~Material() = default;

        std::string GetName() { return m_Name; }
        virtual bool BindShaders() = 0;
        virtual void UpdateBuffers() = 0;
        virtual void BindBuffers() = 0;


    protected:

        std::string m_Name;
        Shader* m_Shader;  // Associated shader

    };

    class DXE_API ColourMaterial : public Material {
    public:

        struct ColourData {  // Struct is now inside the class
            float Colour[4];
        };

        ColourMaterial(const std::string& name, const std::string& shader) : Material(name, shader) {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.ByteWidth = sizeof(ColourData);
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            Renderer::Device()->CreateBuffer(&bufferDesc, nullptr, &m_ConstantBuffer);
        }

        bool BindShaders() override {
            if (m_Shader == nullptr) { return false; }
            m_Shader->Bind();
            return true;
        }

        void UpdateBuffers() override {
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            Renderer::Context()->Map(m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            memcpy(mappedResource.pData, &m_ColourData, sizeof(ColourData));
            Renderer::Context()->Unmap(m_ConstantBuffer.Get(), 0);
        }

        void BindBuffers() override {
            Renderer::Context()->VSSetConstantBuffers(1, 1, m_ConstantBuffer.GetAddressOf());
        }

        void SetColor(float r, float g, float b, float a) {
            m_ColourData.Colour[0] = r;
            m_ColourData.Colour[1] = g;
            m_ColourData.Colour[2] = b;
            m_ColourData.Colour[3] = a;
        }
    private:
        ColourData m_ColourData = {};  // Instance of the struct
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_ConstantBuffer;



    };
}