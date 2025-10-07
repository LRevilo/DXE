#pragma once
#include "DXE.h"
#include <memory>
#include "Buffer.h"
#include "MeshBase.h"
#include "MeshInstance.h"
#include "Material.h"

namespace DXE
{
	class DXE_API MeshManager {
	public:
		static MeshManager* s_MeshManager;
		static MeshManager* Get() { return s_MeshManager; }
		std::string name = "DXMeshManager";
		static void Init(MeshManager* meshManager = nullptr);


		std::unordered_map<std::string, MeshBase*>& Map();
		const std::vector<MeshBase*>& GetMeshes();

		MeshBase* GetMeshBase(const std::string& name);


		void DestroyMeshBase(const std::string& name);
		MeshBase* CreateMeshBase(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);




	private:

		std::vector<MeshBase*> m_Meshes;
		std::unordered_map<std::string, MeshBase*> m_MeshMap;

	};


}