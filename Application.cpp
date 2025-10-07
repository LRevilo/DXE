#include "Application.h"
#include "Logger.h"
#include "Timer.h"
#include "Layer.h"
#include "InputManager.h"
#include <iostream>
#include <filesystem>


namespace DXE {
    Application* Application::s_Application = nullptr;
	Application::Application(HINSTANCE _hInstance, LPSTR _lpCmdLine, int _nShowCmd) :
        hInstance(_hInstance), lpCmdLine(_lpCmdLine), nShowCmd(_nShowCmd) {
        //DXE_INFO("App Constructor");
        if (s_Application) { MessageBoxA(0, "Application already exists!", "Error", MB_ICONERROR | MB_OK); throw; }
        else { s_Application = this; }
               
	}
    void Application::Init(Application* application) {
        if (application) {
            s_Application = application;
            DXE_WARN("Application Set: " + s_Application->name + " : ", s_Application);
        }
    }
	Application::~Application() {
		DXE_INFO("App Destructor");
	}
    LRESULT CALLBACK Application::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        return Application::Get()->WndProc(hwnd, msg, wParam, lParam);
    }
    
	void Application::Run() {
        Logger::AllocateConsole();
        InitSubsystems(DXE::InitData());
        hwnd = Window::MakeWindow(hInstance, &StaticWndProc, L"Window");
        DXE_INFO("App Run");
        Input::SetScreenSize();
        Initialise();
        timer = Timer();
        while (running) {
            RunLoop();
        }
        Shutdown();

	}

    void Application::RunLoop() {
        timer.Tick();

        Input::UpdateInputs(timer.dt);
        MSG msg = {};
        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (msg.message == WM_QUIT) { running = false; }
        Input::ProcessInputs(timer.dt);
        //DXE_INFO("dt:", timer.dt);

        Update(timer.dt);
        Layers::UpdateLayers(timer.dt);
        Layers::RenderLayers(timer.dt);
        Render(timer.dt);
        //DX11Renderer::SwapChain()->Present(1, 0);

        //windowResized = false;



        timer.SleepUntil(1.0 / (double)240);
    }
    
}