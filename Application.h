#pragma once
#include "DXE.h"
#include "Layer.h"
#include "Window.h"
#include "Timer.h"
#define NOMINMAX
#include <windows.h>

namespace DXE {
	class DXE_API Application {
	public:

		Application(HINSTANCE _hInstance,  LPSTR _lpCmdLine, int _nShowCmd);
		~Application();
		void Run();
		static void Init(Application* application = nullptr);
		static Application* Get() { return s_Application; }
		static HWND GetHWND() { return Get()->hwnd; }
		void RunLoop();

		bool running = true;
		bool resized = true;
		bool resizing = false;
		int windowWidth = 1920;
		int	windowHeight = 1080;
		float windowAspectRatio = 1920.f / 1080.f;


	private: 
		static Application* s_Application;
		static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		

	protected:
		virtual void Initialise() = 0;
		virtual void Update(float dt) = 0;
		virtual void Render(float dt) = 0;
		virtual void Shutdown() = 0;
		virtual LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) = 0;

		
		HWND hwnd = nullptr;

		std::string name = "DXApplication";

		HINSTANCE hInstance;
		LPSTR lpCmdLine;
		int nShowCmd;

        Timer timer;




	};

	// Declare the function that will create the Application
	DXE_API Application* CreateApplication(HINSTANCE hInstance,  LPSTR lpCmdLine, int nShowCmd);

}