#include "pch.h"
#include "InputManager.h"
#include "Logger.h"

namespace DXE
{
	InputManager* InputManager::s_InputManager = nullptr;
	void InputManager::Init(InputManager* inputManager) {
		if (!inputManager) {
			s_InputManager = new InputManager();
			DXE_WARN("InputManager Created: " + s_InputManager->name + " : ", s_InputManager);
		}
		else {
			s_InputManager = inputManager;
			DXE_WARN("InputManager Set: " + s_InputManager->name + " : ", s_InputManager);
		}
	}


}
