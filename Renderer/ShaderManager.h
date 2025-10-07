#pragma once
#include "DXE.h"
#include <unordered_map>
#include <string>

#include "ShaderByte.h"

namespace DXE
{

	class Shader;
	class DXE_API ShaderManager
	{
	public:

		static void Init(ShaderManager* shaderManager = nullptr);
		static ShaderManager* s_ShaderManager;
		static ShaderManager* Get() { return s_ShaderManager; }
		std::string name = "DXShaderManager";



		const std::string& GetRawShader(const std::string& name);
		Shader* GetShader(const std::string& name);
		bool AddShader(const std::string& name, const std::string& source);
		bool AddShader(const std::string& name, const DXE::ShaderStruct& shaderStruct);
		bool Exists(const std::string& name);
		void CompileShader(const std::string& name);
		std::vector<std::string> GetNames();
		bool AddRawShaders(std::unordered_map<std::string, std::string> map);


		void AddCompiledShaders(std::unordered_map<std::string, ShaderStruct>& compiledShaderMap);

	private:
		std::vector<Shader*> m_ShaderVector;
		std::unordered_map<std::string, Shader*> m_ShaderMap;
		std::unordered_map < std::string, std::string > m_RawShaders;
	};


}