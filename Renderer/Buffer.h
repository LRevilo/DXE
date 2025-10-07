#pragma once
#include "DXE.h"

#include <d3d11.h>
#include <wrl.h>
#include <vector>


#include "Maths/Maths.h"

namespace DXE
{


    struct Vertex {
        DXM::Vector3 Position;
        DXM::Vector3 Normal;
        DXM::Vector3 Tangent;
        DXM::Vector2 UV;
        DXM::Vector4 Color;
    };
    struct InstanceData {
        InstanceData() = default;
        InstanceData(const InstanceData&) = default;
        InstanceData(const DXM::Matrix& transform, const DXM::Vector4 color)
            : Transform(transform), Color(color) {
        }

        DXM::Matrix Transform;
        DXM::Vector4 Color;
        DXM::Matrix InvTransform;
    };

    class DXE_API VertexBuffer
    {
    public:
        VertexBuffer();
        VertexBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~VertexBuffer();
        void SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
        void UpdateVertices(const std::vector<Vertex>& vertices);
        void UpdateIndices(const std::vector<uint32_t>& indices);
        void Bind(int slot);

        uint32_t m_VertexCount = 0;
        uint32_t m_IndexCount = 0;

        Microsoft::WRL::ComPtr<ID3D11Buffer> VB_GPU() { return m_VertexBuffer; }
        Microsoft::WRL::ComPtr<ID3D11Buffer> IB_GPU() { return m_IndexBuffer; }
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
        D3D11_PRIMITIVE_TOPOLOGY m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    };


    class IndexBuffer {
    public:

        IndexBuffer();
        IndexBuffer(const std::vector<uint32_t>& indices);
        ~IndexBuffer();

        uint32_t m_IndexCount = 0;
        void UpdateIndices(const std::vector<uint32_t>& indices);
        void Bind(int slot);
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
    };


    class InstanceBuffer {
    public:

        uint32_t m_InstanceCount = 0;


        InstanceBuffer();
        ~InstanceBuffer();
        InstanceBuffer(const std::vector<InstanceData>& instances);
        void UpdateInstances(const std::vector<InstanceData>& instances);
        void UpdateInstances(const InstanceData* instances, uint32_t count);
        void Bind(int slot);

        ID3D11Buffer* Get() const { return m_InstanceBuffer.Get(); }



        InstanceData* Map();
        void Unmap();
        void Resize(uint32_t newCount);
        uint32_t Size();


    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_InstanceBuffer;


    };


}