#pragma once
#include "Maths/Maths.h"
#include "Scene/Camera.h" 
#include "Scene/UUID.h"
#include "Renderer/Mesh.h"
#include "Renderer/Texture.h"
#include "Scene/ScriptableEntity.h"
namespace DXE {

	struct DXE_API IDComponent 
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& uuid)
			: ID(uuid) {}

		const char* Name() { return "ID"; }
	};

	struct DXE_API TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}

		const char* Name() { return "Tag"; }
	};

	struct DXE_API TransformComponent
	{
		DXM::Matrix Transform = DXM::Matrix::Identity;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const DXM::Matrix& transform)
			: Transform(transform) {}

		operator DXM::Matrix& () { return Transform; }
		operator const DXM::Matrix& () const { return Transform; }

		const char* Name() { return "Transform"; }
	};

	class ScriptableEntity;
	struct DXE_API NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;
		ScriptableEntity* (*InstantiateScript)();

		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() {return static_cast<ScriptableEntity*>( new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) {delete nsc->Instance; nsc->Instance = nullptr; };

		}
		const char* Name() { return "NativeScript"; }

	};

	//struct SpriteRendererComponent
	//{
	//	glm::vec4 Color { 1.0f };
	//	EDNA::Ref<Texture2D> Texture = nullptr;
	//	bool ScreenSpace = false;
	//
	//	SpriteRendererComponent() = default;
	//	SpriteRendererComponent(const SpriteRendererComponent&) = default;
	//	SpriteRendererComponent(const glm::vec4& color)
	//		: Color(color) {}
	//	SpriteRendererComponent(const Ref<Texture2D> &texture, const glm::vec4& color)
	//		: Texture(texture), Color(color) {}
	//
	//	const char* Name() { return "SpriteRenderer"; }
	//};

	struct DXE_API CameraComponent
	{
		Camera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

		const char* Name() { return "Camera"; }
	};

	struct DXE_API CameraAttachmentComponent
	{

		bool Active = true;
		float SmoothingFactor = 0.0f;
		DXM::Vector3 Offset = { 0.f, 0.5f, 1.f };
		DXM::Vector3 Direction = { 0.f, 0.5f, -1.f };

		CameraAttachmentComponent() = default;
		CameraAttachmentComponent(const CameraAttachmentComponent&) = default;
		CameraAttachmentComponent(const float& smoothing, const DXM::Vector3& offset)
			: SmoothingFactor(smoothing), Offset(offset)  {}

		const char* Name() { return "CameraAttachment"; }
	};


	struct DXE_API RenderableComponent
	{
		bool Visible = true;
		RenderableComponent() = default;
		RenderableComponent(const RenderableComponent&) = default;
		RenderableComponent(const bool& visible)
			: Visible(visible) {}

		const char* Name() { return "Renderable"; }
	};

	struct DXE_API ShadowCasterComponent
	{
		bool Visible = true;
		ShadowCasterComponent() = default;
		ShadowCasterComponent(const ShadowCasterComponent&) = default;
		ShadowCasterComponent(const bool& visible)
			: Visible(visible) {}

		const char* Name() { return "ShadowCaster"; }
	};

	struct DXE_API MeshComponent
	{
		std::shared_ptr<MeshInstance> Mesh;
		bool Dirty = false;
		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(const std::shared_ptr<MeshInstance> mesh)
			: Mesh(mesh) {}
	
		const char* Name() { return "Mesh"; }
	};



	struct DXE_API PlayerInputComponent
	{
		bool Active = false;

		bool Up = false;
		bool Down = false;
		bool Left = false;
		bool Right = false;
		
		void Reset()
		{
			Up = false;
			Down = false;
			Left = false;
			Right = false;
		}


		PlayerInputComponent() = default;
		PlayerInputComponent(const PlayerInputComponent&) = default;

		const char* Name() { return "PlayerInput"; }
	};

}