#include "pch.h"
#include "Components.h"
#include "Scene/Entity.h"

namespace DXE {

	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{

	}

	Entity::~Entity()
	{

	}
	UUID Entity::GetUUID() { return GetComponent<IDComponent>().ID; }

}