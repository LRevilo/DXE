#pragma once
#include "DXE.h"
#include "Renderer.h"
#include "Maths/Maths.h"

namespace DXE
{

    struct DXE_API GlobalCBuffer {
        DXM::Matrix ViewMatrix;
        DXM::Matrix ProjectionMatrix;
        DXM::Matrix ViewProjectionMatrix;

        DXM::Matrix LightViewMatrix;
        DXM::Matrix LightProjectionMatrix;
        DXM::Matrix LightViewProjectionMatrix;

        DXM::Vector3 CameraPosition;
        float DeltaTime;

        DXM::Vector3 SunColor;
        float Time;

        DXM::Vector3 SunDirection;
        float SunIntensity;

        DXM::Vector3 AmbientLight;
        uint32_t FrameCount;

        DXM::Vector2 ScreenSize;
        DXM::Vector2 MousePosition;
        DXM::Vector3 PlayerPos;
        int ShadowMapSize;
    };


    class MeshBase;
    class ShadowMap;
    class Shader;
    class DXE_API RenderManager
    {
    public:
        RenderManager();
     
        static void Init(RenderManager* renderManager = nullptr);
        static RenderManager* s_RenderManager;
        static RenderManager* Get() { return s_RenderManager; }
        std::string name = "DXRenderManager";
        void Initialise();

        template<typename T>
        void CreateGlobalBuffer() {
            m_BufferTypeSize = sizeof(T);
            m_GlobalBuffer = std::make_unique<uint8_t[]>(m_BufferTypeSize); // allocate raw memory

            if (!m_GlobalConstantBuffer)
            {
                D3D11_BUFFER_DESC bufferDesc{};
                bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
                bufferDesc.ByteWidth = static_cast<UINT>(m_BufferTypeSize);
                bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

                HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, nullptr, m_GlobalConstantBuffer.GetAddressOf());
                if (FAILED(hr)) {
                    DXE_ERROR("Failed to create global buffer");
                }
            }
        }
        // Get a reference to the internal buffer for writing
        template<typename T>
        T& GetGlobalBuffer() {
            return *reinterpret_cast<T*>(m_GlobalBuffer.get());
        }

        // Upload the internal buffer to GPU
        void UpdateGlobalBuffer() {
            if (!m_GlobalBuffer) return;
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            Renderer::Context()->Map(m_GlobalConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            memcpy(mappedResource.pData, m_GlobalBuffer.get(), m_BufferTypeSize);
            Renderer::Context()->Unmap(m_GlobalConstantBuffer.Get(), 0);
        }

        void BindGlobalBuffer() {
            Renderer::Context()->VSSetConstantBuffers(0, 1, m_GlobalConstantBuffer.GetAddressOf());
            Renderer::Context()->PSSetConstantBuffers(0, 1, m_GlobalConstantBuffer.GetAddressOf());
            Renderer::Context()->HSSetConstantBuffers(0, 1, m_GlobalConstantBuffer.GetAddressOf());
            Renderer::Context()->DSSetConstantBuffers(0, 1, m_GlobalConstantBuffer.GetAddressOf());
            Renderer::Context()->GSSetConstantBuffers(0, 1, m_GlobalConstantBuffer.GetAddressOf());
        }




        void BeginScene();
        void RenderMeshesByMaterial(const DX::BoundingFrustum& cullFrustum);
        void RenderShadowPass(const DX::BoundingOrientedBox& cullBox);


        void DrawMesh(MeshBase* mesh, const DX::BoundingFrustum& frustrum);

        void DrawMeshShadow(MeshBase* mesh, const DX::BoundingOrientedBox& cullBox);
        void DrawMeshVisible(MeshBase* mesh);
        void DrawMesh(MeshBase* mesh);





        //void CreateGlobalBuffer();
        //void UpdateGlobalBuffer();
        //void BindGlobalBuffer();


        GlobalCBuffer GlobalBuffer;
        bool m_DebugNormals = false;


    private:
         Microsoft::WRL::ComPtr<ID3D11Buffer> m_GlobalConstantBuffer;


         std::unique_ptr<uint8_t[]> m_GlobalBuffer;  // opaque storage
         size_t m_BufferTypeSize = 0;


         Shader* m_DebugNormalShader = nullptr;



    };


}