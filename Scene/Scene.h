#pragma once
#include "entt.hpp"
#include "Scene/UUID.h"
// "EDNA/Renderer/FrameBuffer.h"
#include "Scene/Camera.h"


#include "Maths/Maths.h"

namespace DXE {

	class Entity;             // Forward declaration
	class ScriptableEntity;   // Forward declaration

	class DXE_API Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);


		void OnUpdate(float ts);
		void OnViewportResize(uint32_t width, uint32_t height);

		void UpdateInputs();
		void UpdateScripts(float ts);
		void UpdateCamera();
	
		void RaycastMouse();
		void RenderScene();


		const DXM::Vector3 GetRayDirection() { return m_RayDirection; }
		//Ref<Framebuffer> GetShadowBuffer() { return m_ShadowFramebuffer; }

		void SetLightTransform(DXM::Matrix transform)
		{
			m_LightTransform = transform;
		}
		const DXM::Matrix GetLightTransform()
		{
			return m_LightTransform;
		}

		void SetLightColour(DXM::Vector3 colour)
		{
			m_LightColour = colour;
		}
		const DXM::Vector3 GetLightColour()
		{
			return m_LightColour;
		}

	private:

		Camera* m_SceneCamera = nullptr;
		DXM::Matrix* m_SceneCameraTransform = nullptr;

		DXM::Vector3 m_RayDirection = DXM::Vector3(0.f, 0.f, -1.f);

		entt::registry m_Registry;


		//Ref<Framebuffer> m_ShadowFramebuffer;
		int m_ShadowWidth = 2048;
		int m_ShadowHeight = 2048;
		DXM::Matrix m_LightTransform;
		DXM::Matrix m_LightProjection;
		DXM::Vector3 m_LightColour;
		float m_TimeOfDay;

		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		//friend class Generator;
		//Ref<Generator> m_Generator;
	};
}



