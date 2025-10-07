#pragma once
#if defined(DXAPP)
#define _CRT_SECURE_NO_WARNINGS





#include <windows.h>
#include "DXE.h"
extern DXE::Application* DXE::CreateApplication(HINSTANCE hInstance, LPSTR lpCmdLine, int nShowCmd);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    auto app = DXE::CreateApplication(hInstance, lpCmdLine, nShowCmd);
    app->Run();

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    delete app;

    return 0;
}
#endif               