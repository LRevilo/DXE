#include "DXE.h"
#include "Logger.h"
#include "Application.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderManager.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/MeshManager.h"
#include "InputManager.h"
#include "Layer.h"


namespace DXE {
	void InitSubsystems(InitData initData, const char* src) {
		Application::Init(initData.p_Application);
		Logger::Init(initData.p_Logger, src);
		LayerManager::Init(initData.p_LayerManager);
		Renderer::Init(initData.p_Renderer);
		RenderManager::Init(initData.p_RenderManager);
		ShaderManager::Init(initData.p_ShaderManager);
		InputManager::Init(initData.p_InputManager);
		MeshManager::Init(initData.p_MeshManager);
		Logger::Get()->source = "";
	}
	InitData GetSubsystems() {
		InitData initData{};
		initData.p_Application = Application::Get();
		initData.p_Logger = Logger::Get();
		initData.p_LayerManager = LayerManager::Get();
		initData.p_Renderer = Renderer::Get();
		initData.p_RenderManager = RenderManager::Get();
		initData.p_ShaderManager = ShaderManager::Get();
		initData.p_InputManager = InputManager::Get();
		initData.p_MeshManager = MeshManager::Get();
		return initData;
	}
}