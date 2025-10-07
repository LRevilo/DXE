#include "pch.h"
#include "Mesh.h"
#include "MeshManager.h"


namespace DXE
{

	 const std::vector<MeshBase*>& Mesh::GetMeshes() {
		return MeshManager::Get()->GetMeshes();
	}

	 MeshBase* Mesh::CreateMeshBase(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		return MeshManager::Get()->CreateMeshBase(name, vertices, indices);
	}

	 std::shared_ptr<MeshInstance> Mesh::CreateMeshInstance(MeshBase* mesh, InstanceData instanceData ) {
		 return mesh->CreateInstance(instanceData); // calls MeshBase logic
	}

	 std::shared_ptr<MeshInstance> Mesh::CreateMeshInstance(const std::string& name, InstanceData instanceData) {
		 auto baseMesh = MeshManager::Get()->GetMeshBase(name);
		 return baseMesh->CreateInstance(instanceData);
	 }

	 void Mesh::RemoveMeshInstance(std::shared_ptr<MeshInstance> instance){
		 if (instance->IsValid()) {
			 instance->Base()->DestroyInstance(instance); // forward to MeshBase
		 }
	 }

	 std::unordered_map<std::string, MeshBase*>& Mesh::Map() {
		return MeshManager::Get()->Map();
	}

	 MeshBase* Mesh::GetMeshBase(const std::string& name) {
		return MeshManager::Get()->GetMeshBase(name);

	}
	
}