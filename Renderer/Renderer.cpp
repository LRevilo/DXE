#pragma once
#include "Renderer.h"

namespace DXE {
    Renderer* Renderer::s_Renderer = nullptr;

    void Renderer::Init(Renderer* renderer) {
        if (!renderer) {
            s_Renderer = new Renderer();
            DXE_WARN("Renderer Created: " + s_Renderer->name + " : ", s_Renderer);
        }
        else {
            s_Renderer = renderer;
            DXE_WARN("Renderer Set: " + s_Renderer->name + " : ", s_Renderer);
        }
    }

}