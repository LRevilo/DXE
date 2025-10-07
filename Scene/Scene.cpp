#include "pch.h"
#include "Scene.h"
 
#include "Components.h"

#include "Renderer/Renderer.h"
//#include "EDNA/Renderer/Renderer2D.h"

#include "InputManager.h"
//#include "EDNA/Core/KeyCodes.h"
#include "Maths/Maths.h"
#include "Entity.h"
#include "ScriptableEntity.h"

namespace DXE {

	Scene::Scene()
	{
		//m_Generator = CreateRef<Generator>(this);


		// Initialise lighting
		//FramebufferSpecification fbSpec;
		//fbSpec.Attachments = { FramebufferTextureFormat::Depth };
		//fbSpec.Width = m_ShadowWidth;
		//fbSpec.Height = m_ShadowHeight;
		//m_ShadowFramebuffer = Framebuffer::Create(fbSpec);


		//  m_LightTransform = glm::inverse(glm::lookAt(
		//  	//camera position in world space
		//  	glm::vec3(0.f, 0.f, 2.f),
		//  
		//  	//looking at which point in world space
		//  	glm::vec3(0.f, 0.f, 0.f),
		//  
		//  	// up direction 
		//  	glm::vec3(0.f, 0.f, 1.f)));
		//  
		//  m_LightProjection = glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, 0.01f, 40.f);
		//  
		//  Renderer::s_SceneData->depthTextureID = GetShadowBuffer()->GetDepthAttachmentRendererID();
		//  Renderer::s_SceneData->DepthTexture->Bind(Renderer::s_SceneData->depthTextureID);


	}
	Scene::~Scene()
	{
		//m_Generator->~Generator();
	}
	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}
	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}
	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnUpdate(float ts)
	{
		UpdateInputs();
		UpdateCamera();

		RaycastMouse();

		UpdateScripts(ts);

		RenderScene();

	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		//	m_ViewportWidth = width;
		//	m_ViewportHeight = height;
		//	float aspect = (float)width / (float)height;
		//	
		//	// Resize non fixed aspect ratio cameras
		//	auto cameraView = m_Registry.view<CameraComponent>();
		//	for (auto entity : cameraView)
		//	{
		//		auto& cameraComponent = cameraView.get<CameraComponent>(entity);
		//		if (!cameraComponent.FixedAspectRatio)
		//		{
		//			cameraComponent.Camera.SetAspectRatio(aspect);
		//		}
		//	}

	}




	void Scene::UpdateInputs()
	{
		// // update input components
		// auto playerInputComponentView = m_Registry.view<PlayerInputComponent>();
		// for (auto entity : playerInputComponentView)
		// {
		// 	auto& input = playerInputComponentView.get<PlayerInputComponent>(entity);
		// 	input.Reset();
		// 
		// 	if (Input::IsKeyPressed(EDNA_KEY_W)) { input.Up = true; }
		// 	if (Input::IsKeyPressed(EDNA_KEY_S)) { input.Down = true; }
		// 	if (Input::IsKeyPressed(EDNA_KEY_A)) { input.Left = true; }
		// 	if (Input::IsKeyPressed(EDNA_KEY_D)) { input.Right = true; }
		// }
	}
	void Scene::UpdateScripts(float ts)
	{
		//update scripts
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}
				nsc.Instance->OnUpdate(ts);
			});
	}
	void Scene::UpdateCamera()
	{
		// update camera position
		DXM::Matrix cameraAttachmentTransform = DXM::Matrix::Identity;
		float smoothingFactor = 0.0f;
		auto cameraAttachmentView = m_Registry.view<TransformComponent, CameraAttachmentComponent>();
		for (auto [entity, transformComponent, cameraAttachmentComponent] : cameraAttachmentView.each()) {

			if (cameraAttachmentComponent.Active)
			{
				smoothingFactor = cameraAttachmentComponent.SmoothingFactor;
				//cameraAttachmentTransform = glm::inverse(glm::lookAt(glm::vec3(0), glm::vec3(0,0, -1), glm::vec3(0,0,1)));//glm::mat4{ 1.f };// transformComponent.Transform;
				auto x = transformComponent.Transform.m[3][0] + cameraAttachmentComponent.Offset.x;
				auto y = transformComponent.Transform.m[3][1] + cameraAttachmentComponent.Offset.y;
				auto z = transformComponent.Transform.m[3][2] + cameraAttachmentComponent.Offset.z;

				cameraAttachmentTransform.m[3][0] = x;
				cameraAttachmentTransform.m[3][1] = y;
				cameraAttachmentTransform.m[3][2] = z;



				auto dx = cameraAttachmentComponent.Direction.x;
				auto dy = cameraAttachmentComponent.Direction.y;
				auto dz = cameraAttachmentComponent.Direction.z;


				cameraAttachmentTransform = DXM::Matrix::CreateLookAt(
					DXM::Vector3(x - dx, y - dy, z - dz),
					DXM::Vector3(x, y, z),
					DXM::Vector3(0.f, 0.f, 1.f));
				cameraAttachmentTransform.Invert(cameraAttachmentTransform);

				break;
			}
		}


		auto cameraView = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto [entity, transformComponent, cameraComponent] : cameraView.each()) {
		// Use entity, transform, camera
			if (cameraComponent.Primary)
			{
				float a = (1.f - smoothingFactor); 
				a *= a;
				a *= a;

				transformComponent = (a) * cameraAttachmentTransform + (1.f - a) * (transformComponent.Transform);

				//mainCamera = &cameraComponent.Camera;
				m_SceneCamera = &cameraComponent.Camera;
				m_SceneCameraTransform = &transformComponent.Transform;
				break;
			}
		}
	}

	void Scene::RaycastMouse()
	{
		// Raycast mouse
		if (m_SceneCamera != nullptr && m_SceneCameraTransform != nullptr)
		{
			
			// auto [mouseX, mouseY] = EDNA::Input::GetMouseClientPosition();
			// float mouseXNormalized = (2.0 * mouseX) / (float)m_ViewportWidth - 1.0;
			// float mouseYNormalized = 1.0 - (2.0 * mouseY) / (float)m_ViewportHeight;
			// 
			// 
			// glm::mat4 inverseProjectionMatrix = glm::inverse(m_SceneCamera->GetProjection());
			// 
			// 
			// glm::vec4 rayClip = glm::vec4(mouseXNormalized, mouseYNormalized, -1.0, 1.0);
			// glm::vec4 rayEye = inverseProjectionMatrix * rayClip;
			// rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);
			// 
			// 
			// glm::mat4 inverseViewMatrix = *m_SceneCameraTransform;
			// glm::vec4 rayWorld = glm::normalize(inverseViewMatrix * rayEye);
			// 
			// m_RayDirection = glm::vec3{ rayWorld.x,rayWorld.y,rayWorld.z };



		}
	}

	void Scene::RenderScene()
	{
		if (m_SceneCamera)
		{

	
			// Renderer
			// Sort all meshes by shader
			// Add mesh data to big vertex buffer and render in 1 draw call
			 // better solutioin

			// iterate every mesh
			// call Renderer::SubmitMesh(...)
			// Renderer sorts all the meshes by shader
			// end scene renders all meshes with same shader in 1 draw call

		
			
			//	Renderer::BeginScene(*m_SceneCamera, *m_SceneCameraTransform, m_LightProjection, m_LightTransform, m_LightColour);
			//	  
			//	// Shadow pass
			//	// 
			//	Renderer::BeginShadowPass();
			//	m_ShadowFramebuffer->Bind();
			//	RenderCommand::ClearDepth();
			//	Renderer::OnWindowResize(m_ShadowWidth, m_ShadowHeight);
			//	auto shadowCasterView = m_Registry.view<RenderableComponent, TransformComponent, MeshComponent, ShadowCasterComponent>();
			//	for (auto entity : shadowCasterView)
			//	{
			//		auto [render, transform, mesh] = shadowCasterView.get<RenderableComponent, TransformComponent, MeshComponent>(entity);
			//		Renderer::DrawMeshShadow(mesh.MeshData, transform);
			//	}
			//	m_ShadowFramebuffer->Unbind();
			//	Renderer::EndShadowPass();
			//	
			//	
			//	
			//	
			//	RenderCommand::EnableDepth(true);
			//	RenderCommand::Clear();
			//	// 3D Pass
			//	Renderer::OnWindowResize(m_ViewportWidth, m_ViewportHeight);
			//	auto renderableView = m_Registry.view<RenderableComponent, TransformComponent, MeshComponent>();
			//	for (auto entity : renderableView)
			//	{
			//		auto [render, transform, mesh] = renderableView.get<RenderableComponent, TransformComponent, MeshComponent>(entity);
			//		Renderer::DrawMesh(mesh.MeshData, transform);
			//	}
			//	
			//	
			//	
			//	
			//	Renderer::EndScene();
			//	
			//	
			//	
			//	
			//	
			//	
			//	// 2D pass 
			//	Renderer2D::BeginScene(*m_SceneCamera, *m_SceneCameraTransform);
			//	auto spriteView = m_Registry.view<TransformComponent, SpriteRendererComponent>();
			//	int i = 0;
			//	for (auto entity : spriteView)
			//	{
			//		auto [transform, sprite] = spriteView.get<TransformComponent, SpriteRendererComponent>(entity);
			//	
			//		i++;
			//		if (sprite.Texture) 
			//		{ 
			//			Renderer2D::DrawQuad(transform, sprite.Texture, 1.0f, sprite.Color); 
			//			//EDNA_CORE_INFO("Drawing sprite {0}",i);
			//		}
			//		else 
			//		{ 
			//			Renderer2D::DrawQuad(transform, sprite.Color);
			//			//EDNA_CORE_INFO("Drawing quad {0}", i);
			//		}
			//	}
			//	Renderer2D::EndScene();
		}
	}
}