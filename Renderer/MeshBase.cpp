#include "pch.h"
#include "MeshInstance.h"
#include "MeshBase.h"
namespace DXE
{

	std::shared_ptr<MeshInstance> MeshBase::CreateInstance(const InstanceData& data) {
			entt::entity e = m_Instances.create();
			m_Instances.emplace<InstanceData>(e,data); // default transform
			m_Instances.emplace<VisibilityData>(e);	// radius
			// Wrap in shared_ptr
			return std::make_shared<MeshInstance>(this, e);
	}

	void MeshBase::DestroyInstance(std::shared_ptr<MeshInstance> instance) {
		if (instance) { instance->Destroy(); }
	}



	// Mesh Base
	int MeshBase::GetInstanceCount() { return m_Instances.view<InstanceData>().size();; }
	int MeshBase::GetVisibleInstanceCount() { return m_VisibleInstanceCount; }
	int MeshBase::GetIndexCount() { return m_Indices.size(); }

	void MeshBase::SetMaterial(std::shared_ptr<Material> material) { m_Material = material; }
	std::shared_ptr<Material> MeshBase::GetMaterial() const { return m_Material; }

	void MeshBase::UpdateMeshData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {

		m_Vertices = vertices;
		CalculateBoundingRadius();

		m_Indices = indices;

		m_VertexBuffer->UpdateVertices(m_Vertices);
		m_VertexBuffer->UpdateIndices(m_Indices);
	}
	void MeshBase::UpdateInstances() {
		//m_InstanceBuffer->UpdateInstances(m_InstanceData);
		auto instanceCount = GetInstanceCount();
		if (instanceCount)
			return;
		if (m_InstanceBuffer->Size() < instanceCount) {
			m_InstanceBuffer->Resize(instanceCount);
			DXE_LOG("InstanceBuffer resized");
		}

		InstanceData* gpuData = m_InstanceBuffer->Map();
		if (!gpuData) return;

		UINT totalCount = 0;
		auto maxSize = m_InstanceBuffer->Size();
		// Count visible instances first
		auto group = m_Instances.group<InstanceData>();
		for (auto e : group) {
			if (totalCount >= maxSize) {
				DXE_LOG("InstanceBuffer size exceeded");
				return;
			}
			auto& instanceData = group.get<InstanceData>(e);

			// Transpose directly into GPU buffer
			gpuData[totalCount].Transform = instanceData.Transform.Transpose();

			// Copy any other fields if necessary
			gpuData[totalCount].Color = instanceData.Color;

			DXM::Matrix& world = instanceData.Transform;
			DXM::Matrix rotationScale3x3(
				world._11, world._12, world._13, 0.f,
				world._21, world._22, world._23, 0.f,
				world._31, world._32, world._33, 0.f,
				0.f, 0.f, 0.f, 1.f
			);
			gpuData[totalCount].InvTransform = (rotationScale3x3.Invert()).Transpose();


			++totalCount;
		}

		m_InstanceBuffer->Unmap();

	}
	void MeshBase::BindVertexBuffer(int slot) {
		m_VertexBuffer->Bind(slot);
	}
	void MeshBase::BindInstanceBuffer(int slot) {
		m_InstanceBuffer->Bind(slot);
	}

	void MeshBase::CalculateBoundingRadius() {
		float maxRadiusSq = 0.0f;
		for (const auto& v : m_Vertices)
		{
			float lengthSq = v.Position.LengthSquared();  // squared distance from origin
			if (lengthSq > maxRadiusSq) {
				maxRadiusSq = lengthSq;
			}
		}
		m_BoundingRadius = sqrt(maxRadiusSq);

	}
	void MeshBase::CalculateInstanceBoundingRadius() {
		auto view = m_Instances.view<InstanceData, VisibilityData>();
		view.each([&](auto entity, auto& instanceData, auto& visibility) {

			DXM::Vector3 ScaleX(
				instanceData.Transform._11,
				instanceData.Transform._21,
				instanceData.Transform._31
			);
			DXM::Vector3 ScaleY(
				instanceData.Transform._12,
				instanceData.Transform._22,
				instanceData.Transform._32
			);
			DXM::Vector3 ScaleZ(
				instanceData.Transform._13,
				instanceData.Transform._23,
				instanceData.Transform._33
			);
			// Compare squared lengths to avoid unnecessary sqrt
			float lenSq0 = ScaleX.LengthSquared();
			float lenSq1 = ScaleY.LengthSquared();
			float lenSq2 = ScaleZ.LengthSquared();

			float maxLenSq = std::max(lenSq0, std::max(lenSq1, lenSq2));
			float maxScale = sqrtf(maxLenSq); // Only one sqrt

			visibility.Radius = m_BoundingRadius * maxScale;

			});
	}




	void MeshBase::UpdateVisibleInstances(const DX::BoundingFrustum& frustum) {

		auto instanceCount = GetInstanceCount();
		if (!instanceCount)
			return;
		if (m_InstanceBuffer->Size() < instanceCount) {
			m_InstanceBuffer->Resize(instanceCount);
			DXE_LOG("InstanceBuffer resized");
		}

		InstanceData* gpuData = m_InstanceBuffer->Map();
		if (!gpuData) return;

		UINT visibleCount = 0;
		auto maxSize = m_InstanceBuffer->Size();
		// Count visible instances first
		auto group = m_Instances.group<InstanceData, VisibilityData>();
		for (auto e : group) {
			auto& instanceData = group.get<InstanceData>(e);
			auto& visibility = group.get<VisibilityData>(e);

			DXM::Vector3 position(
				instanceData.Transform._41,
				instanceData.Transform._42,
				instanceData.Transform._43);


			if (DXM::IsInsideFrustum(frustum, position, visibility.Radius)) {
				if (visibleCount >= maxSize) {
					DXE_LOG("InstanceBuffer size exceeded");
					return;
				}


				gpuData[visibleCount].Transform = instanceData.Transform.Transpose();
				gpuData[visibleCount].Color = instanceData.Color;



				gpuData[visibleCount].InvTransform = DXM::Matrix::Identity- DXM::Matrix::Identity;



				++visibleCount;

				visibility.VisibleCamera = true;
			}
			else {
				visibility.VisibleCamera = false;
			}

		}


		m_InstanceBuffer->Unmap();
		m_VisibleInstanceCount = visibleCount;
	}

	void MeshBase::UpdateVisibleInstances(const DX::BoundingOrientedBox& cullBox) {

		auto instanceCount = GetInstanceCount();
		if (!instanceCount)
			return;
		if (m_InstanceBuffer->Size() < instanceCount) {
			m_InstanceBuffer->Resize(instanceCount);
			DXE_LOG("InstanceBuffer resized");
		}

		InstanceData* gpuData = m_InstanceBuffer->Map();
		if (!gpuData) return;

		UINT visibleCount = 0;
		auto maxSize = m_InstanceBuffer->Size();
		// Count visible instances first
		auto group = m_Instances.group<InstanceData, VisibilityData>();
		for (auto e : group) {
			auto& instanceData = group.get<InstanceData>(e);
			auto& visibility = group.get<VisibilityData>(e);

			DXM::Vector3 position(
				instanceData.Transform._41,
				instanceData.Transform._42,
				instanceData.Transform._43);


			if ( DXM::IsInsideOrientedBox(cullBox, position, visibility.Radius)) {
				if (visibleCount >= maxSize) {
					DXE_LOG("InstanceBuffer size exceeded");
					return;
				}
	
				// Transpose directly into GPU buffer
				gpuData[visibleCount].Transform = instanceData.Transform.Transpose();

				// Copy any other fields if necessary
				gpuData[visibleCount].Color = instanceData.Color;
				
				DXM::Matrix& world = instanceData.Transform;
				// Extract rotation+scale
				float m00 = world._11, m01 = world._12, m02 = world._13;
				float m10 = world._21, m11 = world._22, m12 = world._23;
				float m20 = world._31, m21 = world._32, m22 = world._33;

				// Compute 3x3 determinant
				float det = m00 * (m11 * m22 - m12 * m21) - m01 * (m10 * m22 - m12 * m20) + m02 * (m10 * m21 - m11 * m20);
				if (fabs(det) < 1e-6f) det = 1.f; // avoid div by zero

				float invDet = 1.f / det;

				// Compute inverse 3x3
				float i00 = (m11 * m22 - m12 * m21) * invDet;
				float i01 = -(m01 * m22 - m02 * m21) * invDet;
				float i02 = (m01 * m12 - m02 * m11) * invDet;

				float i10 = -(m10 * m22 - m12 * m20) * invDet;
				float i11 = (m00 * m22 - m02 * m20) * invDet;
				float i12 = -(m00 * m12 - m02 * m10) * invDet;

				float i20 = (m10 * m21 - m11 * m20) * invDet;
				float i21 = -(m00 * m21 - m01 * m20) * invDet;
				float i22 = (m00 * m11 - m01 * m10) * invDet;

				// Transpose -> normal matrix
				gpuData[visibleCount].InvTransform = DXM::Matrix(
					i00, i10, i20, 0.f,
					i01, i11, i21, 0.f,
					i02, i12, i22, 0.f,
					0.f, 0.f, 0.f, 1.f
				);

				++visibleCount;
				visibility.VisibleLight = true;
			}
			else {
				visibility.VisibleLight = false;
			}

		}

		m_InstanceBuffer->Unmap();
		m_VisibleInstanceCount = visibleCount;
	}
}