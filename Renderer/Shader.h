#pragma once
#include "DXE.h"
#include "Renderer.h"
#include <string>
#include <unordered_map>
#include <iostream>
#include "Maths/Maths.h"
#include "ShaderByte.h"

namespace DXE
{


    struct DXE_API TransformBuffer {
        DirectX::XMMATRIX TransformMatrix;
        DirectX::XMFLOAT4 Colour;
        uint32_t Instances;
    };

    struct DXE_API ShaderVariableInfo {
        std::string Name;
        uint32_t StartOffset;
        uint32_t Size;
        uint32_t uFlags;
    };

    struct DXE_API ShaderConstantBufferInfo {
        std::string Name;
        uint32_t Size;
        D3D_CBUFFER_TYPE Type;
        uint32_t VariableCount;
        std::vector<ShaderVariableInfo> Variables;
    };

    class DXE_API Shader {
    public:

        Shader();
        Shader(const std::wstring& path);
        Shader(const std::string& name, const std::wstring& path);
        Shader(const std::string& name, const std::string& source);
        Shader(const std::string& name, const ShaderStruct& shaderStruct);
        void LoadFromBytecode(const std::string& name,const ShaderStruct& shaderStruct);


        void CompileFromSource(std::unordered_map <std::string, std::string> includesShaderMap);

        void Bind();


        std::string GetName() { return m_Name; }
        void SetVertexConstantBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> buffer) { m_VertexConstantBuffer = buffer; }
        void SetPixelConstantBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> buffer) { m_PixelConstantBuffer = buffer; }
        const std::unordered_map<std::string, ShaderConstantBufferInfo>& GetConstantBufferInfo() const { return m_ConstantBufferInfo; }
        void SetTexture(ID3D11ShaderResourceView* texture);
        ID3D11ComputeShader* GetComputeShader() { return m_ComputeShader.Get(); };


    private:
        void ReflectInputLayout(ID3D11ShaderReflection* rawReflection);
        UINT GetDXGIFormatSize(DXGI_FORMAT format) {
            std::cout << "DXGI_FORMAT: " << format << std::endl;  // Debugging line
            switch (format) {
            case DXGI_FORMAT_R32_FLOAT:           return 4;
            case DXGI_FORMAT_R32G32_FLOAT:        return 8;
            case DXGI_FORMAT_R32G32B32_FLOAT:     return 12;
            case DXGI_FORMAT_R32G32B32A32_FLOAT:  return 16;
            case DXGI_FORMAT_R8G8B8A8_UNORM:      return 4;
            case DXGI_FORMAT_R16G16_FLOAT:        return 4;
            case DXGI_FORMAT_R16G16B16A16_FLOAT:  return 8;
            case DXGI_FORMAT_R8G8_UNORM:          return 2;
            case DXGI_FORMAT_R8_UNORM:            return 1;
            case DXGI_FORMAT_R32G32B32_UINT:      return 12;
            case DXGI_FORMAT_R32G32B32A32_UINT:   return 16;


                // Add any other formats you may encounter
            default:
                throw std::runtime_error("Unsupported DXGI_FORMAT in GetDXGIFormatSize()");
            }
        }

        std::string m_Name;
        std::wstring m_Path;
        std::string m_Source;
        std::vector<std::string> m_SemanticNameStorage;


        Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;

        bool hasVertex = false;
        bool hasPixel = false;

        bool hasTessellation = false;
        bool hasGeometry = false;
        bool hasCompute = false;
        Microsoft::WRL::ComPtr<ID3D11HullShader> m_HullShader;
        Microsoft::WRL::ComPtr<ID3D11DomainShader> m_DomainShader;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;


        
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_ComputeShader;


        Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;



        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
        std::vector<D3D11_INPUT_ELEMENT_DESC> m_InputLayoutDesc;


        Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexConstantBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_PixelConstantBuffer;

        std::unordered_map<std::string, ShaderConstantBufferInfo> m_ConstantBufferInfo;



    };


    struct DXE_API ShaderIncludeHandler : public ID3DInclude
    {
        std::unordered_map<std::string, std::string>& shaderMap;
        std::vector<std::string> allocatedBuffers; // keep alive until Close()

        ShaderIncludeHandler(std::unordered_map<std::string, std::string>& map)
            : shaderMap(map) {
        }

        HRESULT __stdcall Open(
            D3D_INCLUDE_TYPE /*IncludeType*/,
            LPCSTR pFileName,
            LPCVOID /*pParentData*/,
            LPCVOID* ppData,
            UINT* pBytes
        ) override
        {
            auto it = shaderMap.find(pFileName);
            if (it == shaderMap.end())
                return E_FAIL;

            allocatedBuffers.push_back(it->second); // copy shader text into storage
            std::string& buffer = allocatedBuffers.back();

            *ppData = buffer.data();
            *pBytes = static_cast<UINT>(buffer.size());

            return S_OK;
        }

        HRESULT __stdcall Close(LPCVOID /*pData*/) override
        {
            // We don't free anything here; buffers are owned in allocatedBuffers.
            return S_OK;
        }
    };




}