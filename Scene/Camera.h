#pragma once
#include "DXE.h"
#include "Maths/Maths.h"
#include <optional>
#include "Renderer/Renderer.h"


namespace DXE {
	class DXE_API Camera
	{
	public:
	
		DXM::Vector3 Position;
		DXM::Matrix View;

		float Yaw, Roll, Pitch;
		float FOV = 60.f;
		float ZoomFactor = 1.f;
		float AspectRatio = 16.f/9.f;
		float NearPlane = 1.f;
		float FarPlane = 100.f;
	
		Camera() : 
			Position(0.0f, 0.0f, 10.0f),
			View(DXM::Matrix()),
			FOV(60.0f),
			AspectRatio(16.0f / 9.0f),
			NearPlane(0.1f),
			FarPlane(100.0f)
		{
		}
		~Camera() {};
	
		DXM::Matrix GetViewMatrix() const {
			return  View;
		}
	
		DXM::Matrix GetProjectionMatrix() const {
			return DXM::Matrix::CreatePerspectiveFieldOfView(ZoomFactor * FOV * DirectX::XM_PI/180.f, AspectRatio, NearPlane, FarPlane);
		}
	
		DXM::Matrix GetViewProjectionMatrix() const {
			return GetViewMatrix() * GetProjectionMatrix();
		}
	
		// Make the camera look at a target position
		void LookAt(const DXM::Vector3& target) {
			 View = DXM::Matrix::CreateLookAt(Position, target, DXM::Vector3(0.f, 0.f, 1.f));
		}
	

		bool IsMouseInViewport(float mouseX, float mouseY, const DXM::Viewport& viewport)
		{
			return (mouseX >= viewport.x) && (mouseX < viewport.x + viewport.width) &&
				(mouseY >= viewport.y) && (mouseY < viewport.y + viewport.height);
		}


		std::optional<DXM::Vector3> ScreenToWorldAtZ(float mouseX, float mouseY, float zPlane)
		{

			D3D11_VIEWPORT dxViewport = {};
			UINT numViewports = 1;
			Renderer::Context()->RSGetViewports(&numViewports, &dxViewport);

			DXM::Viewport viewport(
				dxViewport.TopLeftX,
				dxViewport.TopLeftY,
				dxViewport.Width,
				dxViewport.Height,
				dxViewport.MinDepth,
				dxViewport.MaxDepth
			);

			if (!IsMouseInViewport(mouseX, mouseY, viewport)) { return std::nullopt;  }

			//mouseY = dxViewport.Height - mouseY;

			// Step 1: Near and Far screen-space points (with depth)
			DXM::Vector3 screenNear(mouseX, mouseY, 0.0f);  // Depth = 0 (near plane)
			DXM::Vector3 screenFar(mouseX, mouseY, 1.0f);   // Depth = 1 (far plane)

			// Step 2: Unproject to world space


			DXM::Vector3 worldNear = viewport.Unproject(screenNear, GetProjectionMatrix(), GetViewMatrix(), DXM::Matrix::Identity);
			DXM::Vector3 worldFar = viewport.Unproject(screenFar, GetProjectionMatrix(), GetViewMatrix(), DXM::Matrix::Identity);

			// Step 3: Ray direction
			DXM::Vector3 rayDir = worldFar - worldNear;
			rayDir.Normalize();

			// Step 4: Intersect ray with z = zPlane
			float dz = rayDir.z;
			if (fabsf(dz) < 1e-6f) { return std::nullopt;}
				 // No intersection — ray is parallel to the plane

			float t = (zPlane - worldNear.z) / dz;
			return worldNear + rayDir * t;
		}

		DX::BoundingFrustum GetFrustumWorldSpace() {

			DXM::Matrix viewMatrix = GetViewMatrix();
			DXM::Matrix projMatrix = GetProjectionMatrix();

			DX::BoundingFrustum frustum;
			DX::BoundingFrustum::CreateFromMatrix(frustum, projMatrix, true);

			// Move frustum from view space to world space
			DXM::Matrix  invView = viewMatrix.Invert();

			frustum.Transform(frustum, invView);

			return frustum;
		}
	
	
	private:
	
	};

}

