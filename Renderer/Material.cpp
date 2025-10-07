#include "pch.h"
#include "Material.h"
#include "ShaderManager.h"


namespace DXE
{

	Material::Material(const std::string& name, const std::string& shader)
		: m_Name(name), m_Shader(ShaderManager::Get()->GetShader(shader))
	{
	}
	Material::Material(const std::string& name)
		: m_Name(name)
	{
	}
	Material::Material()
	{
	}

}