#pragma once
#include "DXE.h"
#include "SimpleMath.h"
#include "Noise.h"
#include <immintrin.h>
#include <cstdint>
#include <iostream>
#include <cmath>
#include <iomanip>

struct ScopedFlushDenormals {
    unsigned int old_mxcsr;
    ScopedFlushDenormals() {
        old_mxcsr = _mm_getcsr();
        // Set Flush-To-Zero (FTZ) and Denormals-Are-Zero (DAZ) bits
        _mm_setcsr(old_mxcsr | (1 << 15) | (1 << 6));
    }
    ~ScopedFlushDenormals() {
        // Restore old settings
        _mm_setcsr(old_mxcsr);
    }
};
inline void denormal_status() {
    unsigned int mxcsr = _mm_getcsr();
    std::cout << "MXCSR = 0x" << std::hex << mxcsr << std::dec << "\n";
    std::cout << "FTZ bit (15): " << ((mxcsr >> 15) & 1) << "\n";
    std::cout << "DAZ bit (6):  " << ((mxcsr >> 6) & 1) << "\n";
}

inline void classify_value(float x, const char* label) {
    int cls = std::fpclassify(x);
    std::cout << label << ": ";
    switch (cls) {
    case FP_ZERO: std::cout << "ZERO"; break;
    case FP_SUBNORMAL: std::cout << "SUBNORMAL"; break;
    case FP_NORMAL: std::cout << "NORMAL"; break;
    default: std::cout << "OTHER"; break;
    }
    std::cout << std::scientific << std::setprecision(50) << x << "\n";
    //std::cout << " (" << x << ")\n";
}

inline float generate_denormal() {
    // Smallest normal float in IEEE754 is ~1.17549435e-38
    // Divide it further to create a denormal
    return std::ldexp(1.0f, -150);
}

inline void test_without_scope() {
    float x = generate_denormal();
    classify_value(x, "Without FTZ/DAZ");
}

inline void test_with_scope() {
    ScopedFlushDenormals scope;
    float x = generate_denormal();
    classify_value(x, "With FTZ/DAZ");
}

namespace DXM = DirectX::SimpleMath;
namespace DX = DirectX;
// Only do this if C++17 or later is enabled
namespace DirectX::SimpleMath {
    inline const Vector3 Vector3::Up = Vector3(0.0f, 1.0f, 0.0f);
    inline const Vector3 Vector3::Down = Vector3(0.0f, -1.0f, 0.0f);
    inline const Vector3 Vector3::Right = Vector3(1.0f, 0.0f, 0.0f);
    inline const Vector3 Vector3::Left = Vector3(-1.0f, 0.0f, 0.0f);
    inline const Vector3 Vector3::Forward = Vector3(0.0f, 0.0f, 1.0f);
    inline const Vector3 Vector3::Backward = Vector3(0.0f, 0.0f, -1.0f);
    inline const Vector3 Vector3::One = Vector3(1.0f, 1.0f, 1.0f);
    inline const Vector3 Vector3::Zero = Vector3(0.0f, 0.0f, 0.0f);


    inline constexpr double Pi = 3.1415926535897932384626433832795028;



    inline void SetRotationPart(DXM::Matrix& transform, DXM::Vector3 right, DXM::Vector3 forward, DXM::Vector3 up) {
        transform._11 = right.x;    transform._12 = right.y;    transform._13 = right.z;
        transform._21 = forward.x;  transform._22 = forward.y;  transform._23 = forward.z;
        transform._31 = up.x;       transform._32 = up.y;       transform._33 = up.z;
    }


    inline float ToRadians(float d) { return XMConvertToRadians(d); }
    inline Quaternion QuatYPR(float yaw, float pitch, float roll) {
        return Quaternion::CreateFromYawPitchRoll(ToRadians(yaw),  ToRadians(pitch), ToRadians(roll));
    }
    inline DXM::Vector3 GetTranslation(const DXM::Matrix& m) {
        return DXM::Vector3(m._41, m._42, m._43);
    }

    inline DXM::Vector2 Get8DirectionVector2D(const DXM::Vector2& facing, const DXM::Vector2& velocity) {
        if (velocity.LengthSquared() == 0)
            return DXM::Vector2::Zero;

        DXM::Vector2 f = facing;
        DXM::Vector2 v = velocity;
        f.Normalize();
        v.Normalize();

        // Right vector = 90° clockwise rotation
        DXM::Vector2 right(f.y, -f.x);

        float forwardDot = f.Dot(v);
        float rightDot = right.Dot(v);

        const float diagThreshold = 0.382683f; // cos(67.5°) ? sin(22.5°)

        int x = 0;
        int y = 0;

        if (forwardDot > diagThreshold) y = 1;
        else if (forwardDot < -diagThreshold) y = -1;

        if (rightDot > diagThreshold) x = 1;
        else if (rightDot < -diagThreshold) x = -1;

        return DXM::Vector2((float)x, (float)y);
    }


    inline bool IsInsideFrustum(const BoundingFrustum& frustum, const Vector3& pos, float radius)
    {
        BoundingSphere sphere(pos, radius);
        auto result = frustum.Contains(sphere);
        return result != DirectX::DISJOINT;  // returns true if intersects or contained
    }

    inline bool IsInsideOrientedBox(const DX::BoundingOrientedBox& box, const Vector3& pos, float radius)
    {
        BoundingSphere sphere(pos, radius);
        auto result = box.Contains(sphere);
        return result != DirectX::DISJOINT;  // returns true if intersects or contained
    }
    inline DXM::Matrix OrthoToPerspective(const DXM::Matrix& ortho)
    {
        // Extract ortho width/height from the matrix
        float width = 2.0f / ortho._11;
        float height = 2.0f / ortho._22;

        // Extract RHS near/far
        float nearZ = ortho._43 / ortho._33;
        float farZ = nearZ - 1.0f / ortho._33;

        // Create perspective that covers the same extents at the near plane
        float fovY = 2.0f * atan(height * 0.5f / nearZ);
        float aspect = width / height;
        return DXM::Matrix::CreatePerspectiveFieldOfView(fovY, aspect, nearZ, farZ);
    }

}




//#include "DXMaths.h"
//#include "3DMaths.h"