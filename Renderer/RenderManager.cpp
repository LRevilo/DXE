#include "pch.h"
#include "Application.h"
#include "RenderManager.h"
#include "Mesh.h"
#include "Material.h"
#include "Logger.h"
#include "Renderer/ShadowMap.h"
#include "ShaderManager.h"
#include "Shaders/EmbeddedEngineShaders.h"

namespace DXE
{
    RenderManager* RenderManager::s_RenderManager = nullptr;

    void RenderManager::Init(RenderManager* renderManager) {
        if (!renderManager) {
            s_RenderManager = new RenderManager();
            DXE_WARN("RenderManager Created: " + s_RenderManager->name + " : ", s_RenderManager);
        }
        else {
            s_RenderManager = renderManager;
            DXE_WARN("RenderManager Set: " + s_RenderManager->name + " : ", s_RenderManager);
        }
    }


    void RenderManager::Initialise() {
        // add debugnormals shader
        DXE::ShaderManager::Get()->AddRawShaders(RawEngineShaderMap);
        if (!DXE::ShaderManager::Get()->Exists("DebugNormals")) {
            auto& shader = DXE::ShaderManager::Get()->GetRawShader("DebugNormals.hlsl");
            DXE::ShaderManager::Get()->AddShader("DebugNormals", shader);
        }

        m_DebugNormalShader = DXE::ShaderManager::Get()->GetShader("DebugNormals");
        if (!m_DebugNormalShader) { DXE_WARN("DebugNormals Shader not found"); }
    }





    void RenderManager::BeginScene() {

        UpdateGlobalBuffer();
        BindGlobalBuffer();
    }

    //void RenderManager::CreateGlobalBuffer() {
    //    if (!m_GlobalConstantBuffer) {
    //        D3D11_BUFFER_DESC bufferDesc = {};
    //        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    //        bufferDesc.ByteWidth = sizeof(GlobalCBuffer);  // Replace with actual struct size
    //        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    //        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //
    //        // Create the buffer
    //        HRESULT hr = Renderer::Device()->CreateBuffer(&bufferDesc, nullptr, m_GlobalConstantBuffer.GetAddressOf());
    //        if (FAILED(hr)) {
    //            // Handle failure (logging, exceptions, etc.)
    //            DXE_ERROR("Failed to create Global Buffer");
    //        }
    //    }
    //
    //    // add debugnormals shader
    //    DXE::ShaderManager::Get()->AddRawShaders(RawEngineShaderMap);
    //    if (!DXE::ShaderManager::Get()->Exists("DebugNormals")) {
    //        auto& shader = DXE::ShaderManager::Get()->GetRawShader("DebugNormals.hlsl");
    //        DXE::ShaderManager::Get()->AddShader("DebugNormals", shader);
    //    }
    //    m_DebugNormalShader = DXE::ShaderManager::Get()->GetShader("DebugNormals");
    //
    //}

   //void RenderManager::UpdateGlobalBuffer() {
   //    D3D11_MAPPED_SUBRESOURCE mappedResource;
   //    Renderer::Context()->Map(m_GlobalConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
   //    memcpy(mappedResource.pData, &GlobalBuffer, sizeof(GlobalCBuffer));
   //    Renderer::Context()->Unmap(m_GlobalConstantBuffer.Get(), 0);
   //}





    RenderManager::RenderManager() {
    
    }


    void RenderManager::DrawMesh(MeshBase* mesh, const DX::BoundingFrustum& frustrum) {

   
        mesh->UpdateVisibleInstances(frustrum);
        int instanceCount = mesh->GetVisibleInstanceCount();

        int indexCount = mesh->GetIndexCount();
        mesh->BindVertexBuffer(0);
        if (instanceCount) {
            mesh->BindInstanceBuffer(1);
            Renderer::Context()->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);

        }


    }
    void RenderManager::DrawMeshShadow(MeshBase* mesh, const DX::BoundingOrientedBox& cullBox) {

        mesh->UpdateVisibleInstances(cullBox);
        int instanceCount = mesh->GetVisibleInstanceCount();

        // some meshes have tesselation shaders which expects quads as inputs
        // so the shadow shader wont be correct.
        // temporarily swap the index buffer to the 'shadow indices' to render normal triangles
        // during shadow pass.
        int indexCount = mesh->GetIndexCount();
        if (mesh->m_HasShadowIndices) {
            mesh->m_VertexBuffer->UpdateIndices(mesh->m_ShadowIndices);
            indexCount = mesh->m_ShadowIndices.size();
        }

 

        mesh->BindVertexBuffer(0);
        if (instanceCount) {
            mesh->BindInstanceBuffer(1);
            Renderer::Context()->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
        }

        if (mesh->m_HasShadowIndices) {
            mesh->m_VertexBuffer->UpdateIndices(mesh->m_Indices);
        }
    }
    void RenderManager::DrawMeshVisible(MeshBase* mesh) {

        int instanceCount = mesh->GetVisibleInstanceCount();
        int indexCount = mesh->GetIndexCount();
        mesh->BindVertexBuffer(0);
        if (instanceCount) {
            mesh->BindInstanceBuffer(1);
            Renderer::Context()->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
        }

    }
    void RenderManager::DrawMesh(MeshBase* mesh) {

        int instanceCount = mesh->GetInstanceCount();

        int indexCount = mesh->GetIndexCount();
        mesh->BindVertexBuffer(0);
        if (instanceCount) {
            mesh->BindInstanceBuffer(1);
            Renderer::Context()->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
        }

    }
    void RenderManager::RenderShadowPass(const DX::BoundingOrientedBox& cullBox) {

        
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> cullFrontState;

        // Front-face culling
        D3D11_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_FRONT;
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.DepthClipEnable = true;

      

        Renderer::Device()->CreateRasterizerState(&rasterDesc, &cullFrontState);
        Renderer::Context()->RSSetState(cullFrontState.Get());

        auto& meshes = Mesh::GetMeshes();
        for (auto& mesh : meshes) {
            if (mesh->GetMaterial() && mesh->m_CastsShadow) {
                DrawMeshShadow(mesh, cullBox);
            }
        }


        // Back-face culling
        rasterDesc.CullMode = D3D11_CULL_BACK;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> cullBackState;
        Renderer::Device()->CreateRasterizerState(&rasterDesc, cullBackState.GetAddressOf());
        Renderer::Context()->RSSetState(cullBackState.Get());
  

    }
    void RenderManager::RenderMeshesByMaterial(const DX::BoundingFrustum& cullFrustum) {
        // Use a map to group meshes by their material
        std::unordered_map<std::shared_ptr<Material>, std::vector<MeshBase*>> materialGroups;

        auto& meshes = Mesh::GetMeshes();

        // Group meshes by material
        for (auto& mesh : meshes) {
            auto material = mesh->GetMaterial();
            if (material != nullptr) {
                materialGroups[material].push_back(mesh);
                //std::cout << material->GetName() << std::endl;
            }
            else {
                // std::cout << "Null material" << std::endl;
            }
        }

        // Render the groups (this will minimize shader switching)
        // should this be ordered? Probabaly... (for transparency, shadows, etc)
        for (auto& group : materialGroups) {
            auto& material = group.first;
            auto& meshList = group.second;

            // bind the shader
            if (material->BindShaders()) {

                //upload material buffers
                material->UpdateBuffers();
                // bind material buffers
                material->BindBuffers();


                // Render all meshes in the group
                for (MeshBase* mesh : meshList) {
                    // render(mesh); // render the mesh
                    mesh->CalculateInstanceBoundingRadius();
                    DrawMesh(mesh, cullFrustum);

                }


                if (m_DebugNormals) {
                    m_DebugNormalShader->Bind();
                    for (MeshBase* mesh : meshList) {
                        DrawMeshVisible(mesh);
                    }
                }
            }

  
        }

    }
}