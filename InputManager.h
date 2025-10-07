#pragma once
#include "DXE.h"
#include <unordered_map>
#include <windows.h>
#include <string>
#include <iostream>
namespace DXE
{

    enum DXE_API MouseButton {
        LeftButton = 0,
        RightButton = 1,
        MiddleButton = 2,
        XButton1 = 3,
        XButton2 = 4
    };

    struct DXE_API ButtonState {
        bool wasPressed = false; // pressed this frame
        bool wasReleased = false; //released this frame
        bool isHeld = false; // is held
        float heldTime = 0.0f;  // Stores how long the key was held
    };

    class DXE_API InputManager {
    public:

        static InputManager* s_InputManager;
        static InputManager* Get() { return s_InputManager; }
        std::string name = "DXInputManager";
        static void Init(InputManager* inputManager = nullptr);



        std::unordered_map<int, ButtonState> KeyStates;
        std::unordered_map<int, ButtonState> MouseStates;

        POINT MousePosition;

        float MouseDeltaX = 0.f;
        float MouseDeltaY = 0.f;

        float MouseSensitivity = 0.5f;
        float MouseWheelX = 0.f;
        float MouseWheelY = 0.f;

        bool isFocused = true;
        bool isCapturingMouse = false;


    private:
        int ScreenWidth;
        int ScreenHeight;

    public:

        void SetScreenSize() {
            ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
            ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
        }
        bool RepeatedKeyPress(LPARAM lParam) { return ((lParam & (1 << 30)) != 0); }
        // Handle key press
        void OnKeyDown(WPARAM key, LPARAM lParam) {
            if (!RepeatedKeyPress(lParam)) {
                ButtonState& keyState = KeyStates[key];
                keyState.wasPressed = true;
                keyState.isHeld = true;
                keyState.heldTime = 0.f;
            }
        }

        // Handle key release
        void OnKeyUp(WPARAM key, LPARAM lParam) {
            ButtonState& keyState = KeyStates[key];
            keyState.wasReleased = true;
            keyState.isHeld = false;
        }

        // Check if a key is currently pressed
        bool IsKeyHeld(int key) {
            return KeyStates[key].isHeld;
        }
        // Get how long the key was held (even after release)
        float GetKeyHeldTime(int key) {
            return KeyStates[key].heldTime;
        }
        ButtonState GetKeyState(int key) {
            return KeyStates[key];
        }

        void OnMouseMove(POINT pos) {
            MousePosition = pos;
        }

        // Handle mouse press
        void OnMouseButtonDown(int button) {
            ButtonState& mouseState = MouseStates[button];
            mouseState.wasPressed = true;
            mouseState.isHeld = true;
            mouseState.heldTime = 0.f;

        }

        // Handle mouse release
        void OnMouseButtonUp(int button) {
            ButtonState& mouseState = MouseStates[button];
            mouseState.wasReleased = true;
            mouseState.isHeld = false;
        }





        // Check if a mouse is currently pressed!
        bool IsMouseButtonHeld(int button) {
            return MouseStates[button].isHeld;
        }
        // Get how long the mouse was held (even after release)
        float GetMouseButtonHeldTime(int button) {
            return MouseStates[button].heldTime;
        }

        ButtonState GetMouseButtonState(int button) {
            return MouseStates[button];
        }

        void OnMouseWheel(float wheel) {
            MouseWheelY = -wheel;
        }
        void OnMouseHWheel(float wheel) {
            MouseWheelX = wheel;
        }


        void ToggleCaptureMouse() {
            isCapturingMouse = !isCapturingMouse;
            ShowCursor(!isCapturingMouse);
        }

        // Update hold times every frame
        void ProcessInputs(float deltaTime) {
            if (isCapturingMouse && isFocused) {
                static POINT currentMousePos;
                GetCursorPos(&currentMousePos);
                MouseDeltaX = (float)(currentMousePos.x - (ScreenWidth / 2)) * deltaTime;
                MouseDeltaY = (float)(currentMousePos.y - (ScreenHeight / 2)) * deltaTime;
                SetCursorPos(ScreenWidth / 2, ScreenHeight / 2);
            }
        }

        void UpdateInputs(float deltaTime) {

            for (auto& [key, state] : KeyStates) {
                state.wasPressed = false;
                state.wasReleased = false;
                if (state.isHeld) {
                    state.heldTime += deltaTime;  // Accumulate time while held
                }
            }
            for (auto& [mouseButton, state] : MouseStates) {
                state.wasPressed = false;
                state.wasReleased = false;
                if (state.isHeld) {
                    state.heldTime += deltaTime;  // Accumulate time while held
                }
            }

            MouseWheelX = 0.f;
            MouseWheelY = 0.f;
        }

        void ForceRelease() {
            for (auto& [key, state] : KeyStates) {
                state.wasReleased = state.isHeld;
                state.isHeld = false;
            }
            for (auto& [mouseButton, state] : MouseStates) {
                state.wasReleased = state.isHeld;
                state.isHeld = false;
            }
        }


        // Convert virtual key code to a string (e.g., "A", "Enter", "Shift")
        std::string GetKeyName(WPARAM key) {
            char keyName[32] = { 0 };
            LONG lParam = MapVirtualKey(key, MAPVK_VK_TO_VSC) << 16;
            if (GetKeyNameTextA(lParam, keyName, sizeof(keyName)) > 0) {
                return std::string(keyName);
            }
            return "Unknown";
        }
    };



    // interface class

    class DXE_API Input {
    public:

        static void SetScreenSize() {
            InputManager::Get()->SetScreenSize();
        }

        static bool IsKeyHeld(int key) {
            return InputManager::Get()->IsKeyHeld(key);
        }

        static float GetKeyHeldTime(int key) {
            return InputManager::Get()->GetKeyHeldTime(key);
        }

        static ButtonState GetKeyState(int key) {
            return InputManager::Get()->GetKeyState(key);
        }

        static void OnKeyDown(WPARAM key, LPARAM lParam) {
            InputManager::Get()->OnKeyDown(key, lParam);
        }

        static void OnKeyUp(WPARAM key, LPARAM lParam) {
            InputManager::Get()->OnKeyUp(key, lParam);
        }

        static void OnMouseMove(POINT pos) {
            InputManager::Get()->OnMouseMove(pos);
        }

        static void OnMouseButtonDown(int button) {
            InputManager::Get()->OnMouseButtonDown(button);
        }

        static void OnMouseButtonUp(int button) {
            InputManager::Get()->OnMouseButtonUp(button);
        }

        static bool IsMouseButtonHeld(int button) {
            return InputManager::Get()->IsMouseButtonHeld(button);
        }

        static float GetMouseButtonHeldTime(int button) {
            return InputManager::Get()->GetMouseButtonHeldTime(button);
        }

        static ButtonState GetMouseButtonState(int button) {
            return InputManager::Get()->GetMouseButtonState(button);
        }

        static void OnMouseWheel(float wheel) {
            InputManager::Get()->OnMouseWheel(wheel);
        }

        static void OnMouseHWheel(float wheel) {
            InputManager::Get()->OnMouseHWheel(wheel);
        }

        static void ToggleCaptureMouse() {
            InputManager::Get()->ToggleCaptureMouse();
        }

        static void ProcessInputs(float deltaTime) {
            InputManager::Get()->ProcessInputs(deltaTime);
        }

        static void UpdateInputs(float deltaTime) {
            InputManager::Get()->UpdateInputs(deltaTime);
        }

        static void ForceRelease() {
            InputManager::Get()->ForceRelease();
        }

        static std::string GetKeyName(WPARAM key) {
            return InputManager::Get()->GetKeyName(key);
        }

        // Access to member variables ------------------------------------------------------------------------------------------
        // Access to POINT MousePosition
        static POINT& MousePosition() {
            return InputManager::Get()->MousePosition;
        }

        // Access to float MouseDeltaX
        static float& MouseDeltaX() {
            return InputManager::Get()->MouseDeltaX;
        }

        // Access to float MouseDeltaY
        static float& MouseDeltaY() {
            return InputManager::Get()->MouseDeltaY;
        }

        // Access to float MouseSensitivity
        static float& MouseSensitivity() {
            return InputManager::Get()->MouseSensitivity;
        }

        // Access to float MouseWheelX
        static float& MouseWheelX() {
            return InputManager::Get()->MouseWheelX;
        }

        // Access to float MouseWheelY
        static float& MouseWheelY() {
            return InputManager::Get()->MouseWheelY;
        }

        // Access to bool isFocused
        static bool& IsFocused() {
            return InputManager::Get()->isFocused;
        }

        // Access to bool isCapturingMouse
        static bool& IsCapturingMouse() {
            return InputManager::Get()->isCapturingMouse;
        }
    };
}



