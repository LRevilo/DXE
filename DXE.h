#pragma once

// Change this when building DXE as dll or static in visual studio properties

//#define DXENGINE_BUILD_DLL
#define DXENGINE_BUILD_STATIC

#if defined(DXENGINE_BUILD_DLL)
	#if defined(DXENGINE)
		#pragma message("Building DXE.dll")
		#define DXE_API __declspec(dllexport)
		#define DXE_GLOBAL static
	#else
		#pragma message("Linking to DXE.dll")
		#define DXE_API __declspec(dllimport)
		#define DXE_GLOBAL extern
	#endif
#elif defined(DXENGINE_BUILD_STATIC)
	#if defined(DXENGINE)
		#pragma message("Building DXE.lib")
		#define DXE_API
		#define DXE_GLOBAL static
	#else
		#pragma message("Linking to DXE.lib")
		#define DXE_API
		#define DXE_GLOBAL extern
	#endif
#else
	#error "You must define either DXENGINE_BUILD_DLL or DXENGINE_BUILD_STATIC!"
#endif

#if defined(DXLAYER)
	#define DXE_LAYER __declspec(dllexport)
#else 
	#define DXE_LAYER
#endif


#define NOMINMAX

struct HINSTANCE__;
typedef struct HINSTANCE__* HINSTANCE;  // Forward declare HINSTANCE as a pointer to an incomplete struct
typedef struct HINSTANCE__* HMODULE;
typedef char* LPSTR;					// Forward declare LPSTR as a pointer to char





namespace DXE {
	class Application;
	class Logger;
	class Renderer;
	class LayerManager;
	class RenderManager;
	class ShaderManager;
	class InputManager;
	class MeshManager;
	DXE_API struct InitData {
		Application* p_Application = nullptr;
		Logger* p_Logger = nullptr;
		LayerManager* p_LayerManager = nullptr;
		Renderer* p_Renderer = nullptr;
		RenderManager* p_RenderManager = nullptr;
		ShaderManager* p_ShaderManager = nullptr;
		InputManager* p_InputManager = nullptr;
		MeshManager* p_MeshManager = nullptr;
	};
	DXE_API InitData GetSubsystems();
	DXE_API void InitSubsystems(InitData initData, const char* src = nullptr);

}
