#include "Layer.h"
#include <windows.h>
namespace DXE
{
    LayerManager* LayerManager::s_LayerManager = nullptr;
    void LayerManager::Init(LayerManager* layerManager) {
        if (!layerManager) {
            s_LayerManager = new LayerManager();
            DXE_WARN("LayerManager Created: " + s_LayerManager->name + " : ", s_LayerManager);
        }
        else {
            s_LayerManager = layerManager;
            DXE_WARN("LayerManager Set: " + s_LayerManager->name + " : ", s_LayerManager);
        }
    }
    LayerInfo LayerManager::LoadLayerFromDLL(const std::wstring& dllPath, const std::string& layerName, InitData initData) {
        LayerInfo layerInfo;
        layerInfo.layer = nullptr;
        layerInfo.hModule = nullptr;

        wchar_t buffer[MAX_PATH];
        DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (length == 0) {
            // handle error if needed
            return layerInfo;
        }

        auto exePath = std::filesystem::path(std::wstring(buffer, length));
        auto dirPath = std::filesystem::path(exePath).parent_path();
        auto dllFullPath = dirPath / std::filesystem::path(dllPath);
        auto path = dllFullPath.wstring();

        // Load the DLL
        DWORD dwAttrib = GetFileAttributesW(path.c_str());
        if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
            DXE_ERROR("Failed to find DLL: ", dllFullPath.string());
            return layerInfo;
        }

        HMODULE hDll = LoadLibraryW(path.c_str());
        if (!hDll) {
            DWORD error = GetLastError();  // Capture the last error
            DXE_ERROR("Failed to load DLL: ", dllFullPath.string());
            DXE_ERROR("Error code: ", error);
            return layerInfo;
        }

        // Define the expected function signature
        using CreateLayerFunc = DXE::Layer* (*)(const char*, InitData);

        // Get the function
        auto createLayer = reinterpret_cast<CreateLayerFunc>(GetProcAddress(hDll, "CreateLayer"));
        if (!createLayer) {
            DXE_ERROR("Failed to find CreateLayer function in: ", dllFullPath.string());
            FreeLibrary(hDll);
            return layerInfo;
        }

        // Create the layer
        DXE::Layer* layer = createLayer(layerName.c_str(), initData);

        if (!layer) {
            DXE_ERROR("CreateLayer returned null in: ", dllFullPath.string());
            FreeLibrary(hDll);
            return layerInfo;
        }

        layerInfo.layer = layer;
        layerInfo.hModule = hDll;
        layerInfo.path = dllFullPath;
        layerInfo.name = layerName;
        DXE_LOG("Adding layer to map:");

        DXE_INFO("layer  : ", layer);
        DXE_INFO("hmodule: ", hDll);
        DXE_INFO("layer  : ", dllFullPath);
        DXE_INFO("name   : ", layerName);
        layerInfoMap[layerInfo.layer] = layerInfo;

        DXE_LOG("Layer Added");


        return layerInfo;
    }

    void LayerManager::UnloadLayerFromDLL(Layer* layer) {
        auto mapIt = layerInfoMap.find(layer);
        auto lit = std::find(layers.begin(), layers.end(), layer);
        HMODULE dllHandle = nullptr;
        if (mapIt != layerInfoMap.end()) {
            dllHandle = mapIt->second.hModule;
            layerInfoMap.erase(mapIt);
            if (lit != layers.end()) { layers.erase(lit); }
            delete layer;
            // Delete the layer before unloading its DLL
        }
        if (dllHandle) {
            FreeLibrary(dllHandle);
        }
    }

    Layer* LayerManager::GetLayer(const std::string& name) {
        for (auto layer : layers) {
            if (layer->name == name) { return layer; }
        }
        return nullptr;
    }

    void LayerManager::AddLayer(Layer* layer) {
        layers.push_back(layer);
        layer->OnAttach();
    }

    void LayerManager::RemoveLayer(Layer* layer) {
        auto it = std::find(layers.begin(), layers.end(), layer);
        if (it != layers.end()) {
            (*it)->OnDetach();
            layers.erase(it);
        }
    }

    void LayerManager::InsertLayer(Layer* layer, size_t index) {
        if (index <= layers.size()) {
            layers.insert(layers.begin() + index, layer);
            layer->OnAttach();
        }
    }

    void LayerManager::InsertLayerBefore(Layer* newLayer, Layer* existingLayer) {
        auto it = std::find(layers.begin(), layers.end(), existingLayer);
        if (it != layers.end()) {
            layers.insert(it, newLayer);  // Insert before the found layer
            newLayer->OnAttach();
        }
    }

    void LayerManager::InsertLayerAfter(Layer* newLayer, Layer* existingLayer) {
        auto it = std::find(layers.begin(), layers.end(), existingLayer);
        if (it != layers.end()) {
            layers.insert(it + 1, newLayer);  // Insert after the found layer
            newLayer->OnAttach();
        }
    }

    void LayerManager::UpdateLayers(float dt) {
        for (auto& layer : layers) {
            switch (layer->state) {
            case LayerState::OnlyUpdate:
            case LayerState::Active: layer->Update(dt); break;
            default: break;
            }
        }
    }

    void LayerManager::RenderLayers(float dt) {
        for (auto& layer : layers) {
            switch (layer->state) {
            case LayerState::OnlyRender:
            case LayerState::Active: layer->Render(dt); break;
            default: break;
            }
        }

    }




}