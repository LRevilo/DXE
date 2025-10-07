#pragma once
#include "DXE.h"
#include "Logger.h"
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

namespace DXE
{
    enum class LayerState {
        Active,
        Paused,
        OnlyUpdate,
        OnlyRender
    };

    class DXE_API Layer {
    public:
        Layer() : name("NULL") {}
        Layer(const std::string& _name) : name(_name) {}
        std::string name;
        LayerState state = LayerState::Active;
        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void Update(float dt) = 0;
        virtual void Render(float dt) = 0;
        virtual ~Layer() = default;
    };

    class DXE_API ProxyLayer : public Layer {
    public:
        Layer* base;
        ProxyLayer(Layer* _base) : base(_base) {};
        ProxyLayer(const std::string& _name, Layer* _base) : Layer(_name), base(_base) {};
        void OnAttach() { DXE_INFO("Attached Proxy: ", name); }
        void OnDetach() { DXE_INFO("Detached Proxy: ", name); }
        void Update(float dt) { base->Update(dt); }
        void Render(float dt) { base->Render(dt); }
    };

    struct DXE_API LayerInfo {
        Layer* layer;
        HMODULE hModule;
        std::filesystem::path path;
        std::string name;
    };


    class DXE_API LayerManager {
    private:
        std::vector<Layer*> layers;
        std::unordered_map<Layer*, LayerInfo> layerInfoMap;

    public:
        static void Init(LayerManager* layerManager = nullptr);
        static LayerManager* s_LayerManager;
        static LayerManager* Get() { return s_LayerManager; }
        std::string name = "DXLayerManager";


        Layer* GetLayer(const std::string& name);
        void AddLayer(Layer* layer);
        void RemoveLayer(Layer* layer);

        // Insert a layer at a specific index in the vector
        void InsertLayer(Layer* layer, size_t index);

        void InsertLayerBefore(Layer* newLayer, Layer* existingLayer);
        void InsertLayerAfter(Layer* newLayer, Layer* existingLayer);


        void UpdateLayers(float dt);
        void RenderLayers(float dt);

        LayerInfo LoadLayerFromDLL(const std::wstring& dllPath, const std::string& layerName, InitData initData);
        void UnloadLayerFromDLL(Layer* layer);

    };

    // interface ------------------------------------------------------------------------------------------------------------------------

    class DXE_API Layers {
    public:

        static Layer* GetLayer(const std::string& name) {
            return LayerManager::Get()->GetLayer(name);
        }

        static void AddLayer(Layer* layer) {
            LayerManager::Get()->AddLayer(layer);
        }

        static void RemoveLayer(Layer* layer) {
            LayerManager::Get()->RemoveLayer(layer);
        }

        static void InsertLayer(Layer* layer, size_t index) {
            LayerManager::Get()->InsertLayer(layer, index);
        }

        static void InsertLayerBefore(Layer* newLayer, Layer* existingLayer) {
            LayerManager::Get()->InsertLayerBefore(newLayer, existingLayer);
        }

        static void InsertLayerAfter(Layer* newLayer, Layer* existingLayer) {
            LayerManager::Get()->InsertLayerAfter(newLayer, existingLayer);
        }

        static void UpdateLayers(float dt) {
            LayerManager::Get()->UpdateLayers(dt);
        }

        static void RenderLayers(float dt) {
            LayerManager::Get()->RenderLayers(dt);
        }

        static LayerInfo LoadLayerFromDLL(const std::wstring& dllPath, const std::string& layerName, InitData initData) {
            return LayerManager::Get()->LoadLayerFromDLL(dllPath, layerName, initData);
        }

        static void UnloadLayerFromDLL(Layer* layer) {
            LayerManager::Get()->UnloadLayerFromDLL(layer);
        }

    };
}
