#pragma once
#include "DXE.h"
#include "Renderer/Renderer.h"
#include "Maths/Maths.h"
#include "Renderer/Shader.h"
#include "Scene/Camera.h"

namespace DXE {
    class DXE_API ShadowMap {
    public:

        float lightDistance = 100.0f;

        float cameraToFocusDistance = 10.f;

        float orthoWidth = 50.0f;
        float orthoHeight = 50.0f;
        float nearPlane = 0.01f;
        float farPlane = 200.0f;


        DXM::Matrix lightView;
        DXM::Matrix lightProj;
        DXM::Matrix lightViewProj;


        ShadowMap(UINT width, UINT height, Shader* shader);
        ~ShadowMap() = default;

        // Bind DSV for light rendering
        void BeginRender();
        void EndRender();

  

        // Accessors
        ID3D11ShaderResourceView* GetSRV() const { return shadowSRV.Get(); }
        ID3D11DepthStencilView* GetDSV() const { return shadowDSV.Get(); }
        ID3D11SamplerState* GetSampler() const { return shadowSampler.Get(); }
        ID3D11SamplerState* GetDefaultSampler() const { return defaultSampler.Get(); }
        //ID3D11ShaderResourceView* GetNormalSRV() const { return normalSRV.Get(); }

        UINT GetWidth() const { return width; }
        UINT GetHeight() const { return height; }


        void Update(DXM::Vector3 focusPos, DXM::Vector3 sunDirection, DXE::Camera& camera) {


             sunDirection.Normalize();
             // Calculate light position (offset from player along -SunDirection)
             DXM::Vector3 lightPos = focusPos - sunDirection * lightDistance;
             
             // Choose an up vector for the view matrix
             // Define your world up as +Z axis
             DXM::Vector3 worldUp(0, 0, 1);
             DXM::Vector3 up = fabs(sunDirection.Dot(worldUp)) > 0.99f ? DXM::Vector3(0, 1, 0) : worldUp;
             
             // Create light view matrix looking at the player position
             lightView = DXM::Matrix::CreateLookAt(lightPos, focusPos, up);
             


             // Create orthographic projection matrix
             lightProj = DXM::Matrix::CreateOrthographic(orthoWidth, orthoHeight, nearPlane, farPlane);
             lightViewProj = lightView * lightProj;
        }

        DX::BoundingOrientedBox  GetLightBoxWorldSpace() {
            DX::BoundingOrientedBox box;


            // The ortho frustum is just a centered rectangular prism in light space
            float depth = farPlane - nearPlane;

            // Center in light space (at near+far midpoint, looking down -Z)
            DXM::Vector3 centerLS(0.0f, 0.0f, -(nearPlane + depth * 0.5f));

            // Extents (half-size)
            DXM::Vector3 extents(orthoWidth * 0.5f, orthoHeight * 0.5f, depth * 0.5f);

            // Orientation: inverse of lightView rotation (to go from light space to world space)
            DXM::Matrix invView = lightView.Invert();
            DXM::Quaternion orientation = DXM::Quaternion::CreateFromRotationMatrix(invView);

            // Center in world space
            DXM::Vector3 centerWS = DXM::Vector3::Transform(centerLS, invView);

            box.Center = centerWS;
            box.Extents = extents;
            box.Orientation = orientation;

            return box;
        }


    private:
        void CreateResources();

        UINT width;
        UINT height;

        Shader* shader = nullptr;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> defaultSampler;

        //Microsoft::WRL::ComPtr<ID3D11Texture2D> normalTexture;
        //Microsoft::WRL::ComPtr<ID3D11RenderTargetView> normalRTV;
        //Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalSRV;

        D3D11_VIEWPORT viewport;
    };
}