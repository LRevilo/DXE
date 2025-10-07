#pragma once
#include "DXE.h"
#include "Scene/entt.hpp"

#include "Buffer.h" // for instance data
#include "Maths/Maths.h"
#include "MeshBase.h"

namespace DXE {

	class DXE_API MeshInstance {
	public:
		friend class MeshBase;
		MeshInstance() = default;

		explicit MeshInstance(MeshBase* mesh, entt::entity e) : m_Mesh(mesh), m_ID(e){}
		// Allow implicit conversion to entt::entity for registry functions
		
		bool operator==(const MeshInstance& other) const = default;
		bool IsValid() const { return m_ID != entt::null && m_Mesh && m_Mesh->m_Instances.valid(m_ID); }
		void Invalidate() { m_ID = entt::null; m_Mesh = nullptr; }
		void Destroy() {
			if (IsValid()) {
				m_Mesh->m_Instances.destroy(m_ID); // Remove entity from registry
				Invalidate();                      // Reset the handle
			}
		}
		operator bool() const { return IsValid(); }
		operator entt::entity() const { return m_ID; }



		// Access the InstanceData component
		InstanceData& Instance() const { return m_Mesh->m_Instances.get<InstanceData>(m_ID);}
		DXM::Matrix& Transform() const { return Instance().Transform; }
		DXM::Vector4& Colour() const { return Instance().Color; }
		MeshBase* Base() const { return m_Mesh; }


	private:
		entt::entity m_ID{ entt::null };
		MeshBase* m_Mesh {nullptr};

	};

}