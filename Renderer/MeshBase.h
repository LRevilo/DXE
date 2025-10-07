#pragma once
#include "DXE.h"

#include "Material.h"
#include <memory>
#include "Scene/entt.hpp"
#include "Buffer.h"	   // contains FULL DEFINITION OF InstanceData
//#include "Maths/Maths.h"
namespace DXE
{
	class MeshInstance;

	struct VisibilityData {

		VisibilityData() = default;
		VisibilityData(const VisibilityData&) = default;
		VisibilityData& operator=(const VisibilityData&) = default;
		VisibilityData(float radius)
			: Radius(radius){}
		float Radius = 0.f;
		bool VisibleCamera = false;
		bool VisibleLight = false;
	};



	class DXE_API MeshBase {
	public:
		MeshBase(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) :
			m_Name(name),
			m_Vertices(vertices),
			m_Indices(indices),
			m_VertexBuffer(std::make_shared<VertexBuffer>(vertices, indices)),
			m_IndexBuffer(std::make_shared<IndexBuffer>(indices)),
			m_InstanceBuffer(std::make_shared<InstanceBuffer>()) {
			CalculateBoundingRadius();
		}

		std::shared_ptr<MeshInstance> CreateInstance(const InstanceData& data = InstanceData());
		void DestroyInstance(std::shared_ptr<MeshInstance> instance);


		std::string m_Name;
		std::shared_ptr<Material> m_Material;

		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;

		std::vector<uint32_t> m_ShadowIndices;

		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		std::shared_ptr<InstanceBuffer> m_InstanceBuffer;


		entt::registry m_Instances;



		bool m_HasShadowIndices = false;

		bool m_CastsShadow = true;
		bool m_CullOutsideFrustrum = true;
		uint32_t m_VisibleInstanceCount = 0;
		float m_BoundingRadius = 0.0f;

		int GetInstanceCount();
		int GetVisibleInstanceCount();
		int GetIndexCount();

		void SetMaterial(std::shared_ptr<Material> material);
		std::shared_ptr<Material> GetMaterial() const;
		void UpdateMeshData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		void BindVertexBuffer(int slot);
		void BindInstanceBuffer(int slot);

		void CalculateBoundingRadius();
		void CalculateInstanceBoundingRadius();

		void UpdateInstances();
		void UpdateVisibleInstances(const DX::BoundingOrientedBox& cullBox);
		void UpdateVisibleInstances(const DX::BoundingFrustum& frustum);

	};

}
