#include "pch.h"
#include "MeshManager.h"
namespace DXE
{
	MeshManager* MeshManager::s_MeshManager = nullptr;
	void MeshManager::Init(MeshManager* meshManager) {
		if (!meshManager) {
			s_MeshManager = new MeshManager();
			DXE_WARN("MeshManager Created: " + s_MeshManager->name + " : ", s_MeshManager);
		}
		else {
			s_MeshManager = meshManager;
			DXE_WARN("MeshManager Set: " + s_MeshManager->name + " : ", s_MeshManager);
		}
	}

	const std::vector<MeshBase*>& MeshManager::GetMeshes() { return m_Meshes; }
	std::unordered_map<std::string, MeshBase*>& MeshManager::Map() { return m_MeshMap; }

	MeshBase* MeshManager::CreateMeshBase(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
		auto it = m_MeshMap.find(name);
		if (it == m_MeshMap.end()) {
			MeshBase* mesh = new MeshBase(name, vertices, indices);
			m_Meshes.push_back(mesh);
			m_MeshMap[name] = mesh;
			return mesh;
		}
		else { return nullptr; }
	}

	MeshBase* MeshManager::GetMeshBase(const std::string& name) {
		auto& meshMap = MeshManager::Get()->Map();
		auto it = meshMap.find(name);
		if (it != meshMap.end()) { return it->second; }
		return nullptr;
	}

	void MeshManager::DestroyMeshBase(const std::string& name) {
		auto it = m_MeshMap.find(name);
		if (it != m_MeshMap.end()) {
			auto& meshBase = it->second;
			// Destroy all entities in the mesh's registry
			meshBase->m_Instances.clear(); // removes all entities and components

			m_MeshMap.erase(it);
		}
	}




}