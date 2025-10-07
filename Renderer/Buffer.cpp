#include "pch.h"
#include "Buffer.h"
#include "Renderer.h"
#include <iostream>

namespace DXE
{

    VertexBuffer::VertexBuffer() {}
    VertexBuffer::~VertexBuffer() = default;
    VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
        :
        m_VertexCount(0),
        m_IndexCount(0),
        m_Topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    {
        // Create vertex buffer
        UpdateVertices(vertices);
        // Create index buffer.
        UpdateIndices(indices);

        DXE_INFO("Vertex Buffer Created");

    }

    void VertexBuffer::UpdateVertices(const std::vector<Vertex>& vertices) {
        if (vertices.empty()) return;

        if (vertices.size() == m_VertexCount) {
            Renderer::Context()->UpdateSubresource(m_VertexBuffer.Get(), 0, nullptr, vertices.data(), 0, 0);
        }
        else {
            m_VertexCount = vertices.size();
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = sizeof(Vertex) * m_VertexCount;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = 0;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = vertices.data();

            HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, &initData, m_VertexBuffer.ReleaseAndGetAddressOf());
            assert(SUCCEEDED(hr));
        }
    }

    void VertexBuffer::UpdateIndices(const std::vector<uint32_t>& indices) {
        if (indices.empty()) return;
        if (indices.size() == m_IndexCount) {
            Renderer::Context()->UpdateSubresource(m_IndexBuffer.Get(), 0, nullptr, indices.data(), 0, 0);
        }
        else {
            m_IndexCount = indices.size();
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = sizeof(uint32_t) * m_IndexCount;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = indices.data();
            HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, &initData, m_IndexBuffer.ReleaseAndGetAddressOf());
            assert(SUCCEEDED(hr));
        }
    }

    void VertexBuffer::Bind(int slot) {
        ID3D11Buffer* buffers[] = { m_VertexBuffer.Get() };
        uint32_t stride = sizeof(Vertex);
        uint32_t offset = 0;
        Renderer::Context()->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        Renderer::Context()->IASetVertexBuffers(slot, 1, buffers, &stride, &offset);
    }

    void VertexBuffer::SetTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        m_Topology = topology;
    }



    IndexBuffer::IndexBuffer() {}
    IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices) {

        // Create index buffer.
        UpdateIndices(indices);
        DXE_INFO("Index Buffer Created");
    }
    IndexBuffer::~IndexBuffer() {}

    void IndexBuffer::UpdateIndices(const std::vector<uint32_t>& indices) {
        if (indices.empty()) return;
        if (indices.size() == m_IndexCount) {
            Renderer::Context()->UpdateSubresource(m_IndexBuffer.Get(), 0, nullptr, indices.data(), 0, 0);
        }
        else {
            m_IndexCount = indices.size();
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.ByteWidth = sizeof(uint32_t) * m_IndexCount;
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = indices.data();
            HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, &initData, m_IndexBuffer.ReleaseAndGetAddressOf());
            assert(SUCCEEDED(hr));
        }
    }
    void IndexBuffer::Bind(int slot) {
        ID3D11Buffer* buffers[] = { m_IndexBuffer.Get() };
        uint32_t stride = sizeof(Vertex);
        uint32_t offset = 0;
        Renderer::Context()->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        Renderer::Context()->IASetVertexBuffers(slot, 1, buffers, &stride, &offset);
    }


    InstanceBuffer::InstanceBuffer() {
    }
    InstanceBuffer::~InstanceBuffer() = default;
    InstanceBuffer::InstanceBuffer(const std::vector<InstanceData>& instances)
        :
        m_InstanceCount(instances.size())
    {
        // Create instance buffer.
        if (m_InstanceCount) {
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.ByteWidth = sizeof(InstanceData) * (m_InstanceCount);
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = instances.data();
            HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, &initData, m_InstanceBuffer.GetAddressOf());
            assert(SUCCEEDED(hr));
            DXE_LOG("Instance Buffer Created");
            DXE_LOG("Instances: ", m_InstanceCount);
        }
    }


    void InstanceBuffer::Bind(int slot) {
        ID3D11Buffer* buffers[] = { m_InstanceBuffer.Get() };
        uint32_t stride = sizeof(InstanceData);
        uint32_t offset = 0;
        Renderer::Context()->IASetVertexBuffers(slot, 1, buffers, &stride, &offset);
    }
    void InstanceBuffer::UpdateInstances(const std::vector<InstanceData>& instances) {
        if (instances.empty()) {
            m_InstanceCount = 0;
            m_InstanceBuffer.Reset();
            return;
        }
        if (instances.size() != m_InstanceCount) {
            // Recreate buffer with new size (this is costly but ensures safety)
            m_InstanceCount = instances.size();
            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.ByteWidth = sizeof(InstanceData) * m_InstanceCount;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = instances.data();
            HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, &initData, m_InstanceBuffer.GetAddressOf());
            assert(SUCCEEDED(hr));
        }
        //Update the instance buffer
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        Renderer::Context()->Map(m_InstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        memcpy(mappedResource.pData, instances.data(), sizeof(InstanceData) * m_InstanceCount);
        Renderer::Context()->Unmap(m_InstanceBuffer.Get(), 0);
    }

    void InstanceBuffer::UpdateInstances(const InstanceData* instances, uint32_t count) {
        if (count == 0) {
            m_InstanceCount = 0;
            m_InstanceBuffer.Reset();
            return;
        }

        // Recreate buffer if size changed
        if (count != m_InstanceCount)
        {
            m_InstanceCount = count;

            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.ByteWidth = sizeof(InstanceData) * m_InstanceCount;
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = instances;

            HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, &initData, m_InstanceBuffer.GetAddressOf());
            assert(SUCCEEDED(hr));
        }

        // Map and copy
        D3D11_MAPPED_SUBRESOURCE mapped;
        Renderer::Context()->Map(m_InstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, instances, sizeof(InstanceData) * m_InstanceCount);
        Renderer::Context()->Unmap(m_InstanceBuffer.Get(),0);
    }


    InstanceData* InstanceBuffer::Map()
    {
        if (!m_InstanceBuffer)
            return nullptr;

        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = Renderer::Context()->Map(m_InstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        assert(SUCCEEDED(hr));
        return reinterpret_cast<InstanceData*>(mapped.pData);
    }

    // Unmaps the GPU buffer
    void  InstanceBuffer::Unmap() {
        if (!m_InstanceBuffer) return;
        Renderer::Context()->Unmap(m_InstanceBuffer.Get(), 0);
    }

    uint32_t InstanceBuffer::Size() { return m_InstanceCount; }


    void InstanceBuffer::Resize(uint32_t newCount) {
        if (newCount == m_InstanceCount) return;
        m_InstanceCount = newCount;
        if (m_InstanceCount == 0) {
            m_InstanceBuffer.Reset();
            return;
        }

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(InstanceData) * m_InstanceCount;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, nullptr, m_InstanceBuffer.GetAddressOf());
        assert(SUCCEEDED(hr));
    }
}