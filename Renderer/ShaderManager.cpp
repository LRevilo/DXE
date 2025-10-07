#include "pch.h"
#include "ShaderManager.h"
#include "Shader.h"
#include "Shaders/EmbeddedEngineShaders.h"

namespace DXE
{


    ShaderManager* ShaderManager::s_ShaderManager = nullptr;

    void ShaderManager::Init(ShaderManager* shaderManager) {
        if (!shaderManager) {
            s_ShaderManager = new ShaderManager();
            s_ShaderManager->AddRawShaders(DXE::RawEngineShaderMap);
            DXE_WARN("ShaderManager Created: " + s_ShaderManager->name + " : ", s_ShaderManager);
        }
        else {
            s_ShaderManager = shaderManager;
            DXE_WARN("ShaderManager Set: " + s_ShaderManager->name + " : ", s_ShaderManager);
        }
    }


    void ShaderManager::AddCompiledShaders(std::unordered_map<std::string, ShaderStruct>& compiledShaderMap) {
        for (const auto& pair : compiledShaderMap) {
            auto name = pair.first;
            auto& shader_struct = pair.second;
            DXE_LOG("Adding bytecode shader: ", name);
            bool status = AddShader(name, shader_struct);
            DXE_LOG(status ? "Success" : "Failed");
        }
    }



    bool ShaderManager::AddRawShaders(std::unordered_map<std::string, std::string> map) {
        for (const auto& pair : map) {
            m_RawShaders.insert_or_assign(pair.first, pair.second);
        }
        return true;
    }    


    const std::string& ShaderManager::GetRawShader(const std::string& name) {
        auto it = m_RawShaders.find(name);
        assert(it != m_RawShaders.end());
        return it->second;
    }

    bool ShaderManager::AddShader(const std::string& name, const std::string& source) {
        if (Exists(name)) { return false; }
        m_ShaderVector.push_back(new Shader(name, source));
        m_ShaderMap[name] = m_ShaderVector.back();
        m_ShaderMap[name]->CompileFromSource(m_RawShaders);
        return true;
    }

    bool ShaderManager::AddShader(const std::string& name, const DXE::ShaderStruct& shaderStruct) {
        if (Exists(name)) { return false; }
        m_ShaderVector.push_back(new Shader(name, shaderStruct));
        m_ShaderMap[name] = m_ShaderVector.back();
        //m_ShaderMap[name]->CompileFromSource(m_RawShaders);
        return true;
    }


    void ShaderManager::CompileShader(const std::string& name) {
        assert(Exists(name));
        m_ShaderMap[name]->CompileFromSource(m_RawShaders);
    }

    Shader* ShaderManager::GetShader(const std::string& name) {
        if (Exists(name)) { return m_ShaderMap[name]; }
        return nullptr;
    }

    bool ShaderManager::Exists(const std::string& name) {
        auto it = m_ShaderMap.find(name);
        return (it != m_ShaderMap.end());
    }



    std::vector<std::string> ShaderManager::GetNames() {
        std::vector<std::string> names;
        for (auto& shader : m_ShaderVector) {
            names.push_back(shader->GetName());
        }
        return names;
    }


    
}