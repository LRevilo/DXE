#pragma once
#include "DXE.h"
#include <memory>
#include <string>

#include "MeshBase.h"
#include "MeshInstance.h"

namespace DXE {

	class MeshManager;
	class Mesh {
	public:

	static const std::vector<MeshBase*>& GetMeshes();
	static std::unordered_map<std::string, MeshBase*>& Map();


	static MeshBase* GetMeshBase(const std::string& name);
	static MeshBase* CreateMeshBase(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
	static std::shared_ptr<MeshInstance> CreateMeshInstance(MeshBase* mesh, InstanceData instanceData = InstanceData());
	static std::shared_ptr<MeshInstance> CreateMeshInstance(const std::string& name, InstanceData instanceData = InstanceData());
	static void RemoveMeshInstance(std::shared_ptr<MeshInstance> instance);


	};

}