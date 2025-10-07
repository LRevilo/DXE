#pragma once
#include "SimpleMath.h"
#include <algorithm>
#include <cmath> 
#include <memory>

namespace DirectX::SimpleMath {

	template<typename T>
	constexpr T Min(T a, T b) {
		return (a < b) ? a : b;
	}

	// make sure to change the shader equivalent if this gets modified
	inline float SmoothStep(float z) {
		return z * z * (3.f - 2.f * z);
	}
	inline float SmoothStepD1(float z) {
		return  z * (6.f - 6.f * z);
	}
	inline float SmootherStep(float z) {
		return z * z * z * (10.f - 15.f * z + 6.f * z * z);
	}
	inline float ArctanType(float x) {
		// pi/2
		return 1.57079632679 * x / (1.f + abs(x));
	}
	inline float CosType(float x) {
		// 4/pi^2
		return 1.f - 0.405284734569f * x * x;
	}

	inline uint32_t Squirrel1D(int x, uint32_t seed = 0) {
		constexpr unsigned int P1 = 3039394381;
		constexpr unsigned int P2 = 1759714724;
		constexpr unsigned int P3 = 458671337;
		unsigned int mangled = x;

		mangled *= P1;
		mangled += seed;
		mangled ^= (mangled >> 8);
		mangled += P2;
		mangled ^= (mangled << 8);
		mangled *= P3;
		mangled ^= (mangled >> 8);

		return mangled;
	}

	inline uint32_t Squirrel2D(int x, int y, uint32_t seed = 0) {
		constexpr unsigned int P4 = 198491317;
		constexpr unsigned int P5 = 479001599;
		constexpr unsigned int P6 = 2971215073;
		unsigned int mangled = y;

		mangled *= P4;
		mangled += seed;
		mangled ^= (mangled >> 8);
		mangled += P5;
		mangled ^= (mangled << 8);
		mangled *= P6;
		mangled ^= (mangled >> 8);
		return Squirrel1D(x + mangled, seed);
	}

	inline float SmoothNoise(float x, float y, uint32_t seed = 0) {
		// Returns smoothly intepolated value noise
		auto x_floor = floor(x);
		auto x_frac = x - x_floor;
		auto y_floor = floor(y);
		auto y_frac = y - y_floor;

		static const int scalar = 16777215; // 2^24 - 1
		static const float inv_scalar = 1.f / 16777215.f; // 2^24 - 1

		float n0 = (Squirrel2D(x_floor, y_floor, seed) & scalar) * inv_scalar;
		float n1 = (Squirrel2D(x_floor + 1, y_floor, seed) & scalar) * inv_scalar;
		float n2 = (Squirrel2D(x_floor, y_floor + 1, seed) & scalar) * inv_scalar;
		float n3 = (Squirrel2D(x_floor + 1, y_floor + 1, seed) & scalar) * inv_scalar;

		float Sx = SmoothStep(x_frac);
		float Sy = SmoothStep(y_frac);

		auto f0 = (1.f - Sx) * n0 + Sx * n1;
		auto f1 = (1.f - Sx) * n2 + Sx * n3;
		float g0 = (1.f - Sy) * f0 + Sy * f1;

		return g0;
	}


	inline Vector3 SmoothNoiseD1(float x, float y, uint32_t seed = 0) {

		auto x_floor = floor(x);
		auto x_frac = x - x_floor;
		auto y_floor = floor(y);
		auto y_frac = y - y_floor;

		static const int scalar = 16777215; // 2^24 - 1
		static const float inv_scalar = 1.f / 16777215.f; // 2^24 - 1

		float n0 = (Squirrel2D(x_floor, y_floor, seed) & scalar) * inv_scalar;
		float n1 = (Squirrel2D(x_floor + 1, y_floor, seed) & scalar) * inv_scalar;
		float n2 = (Squirrel2D(x_floor, y_floor + 1, seed) & scalar) * inv_scalar;
		float n3 = (Squirrel2D(x_floor + 1, y_floor + 1, seed) & scalar) * inv_scalar;

		// normal vector = (-dP/dx, -dp/dy, 1)
		float Sx = SmoothStep(x_frac);
		float Sy = SmoothStep(y_frac);

		auto f0 = (1.f - Sx) * n0 + Sx * n1;
		auto f1 = (1.f - Sx) * n2 + Sx * n3;
		float g0 = (1.f - Sy) * f0 + Sy * f1;

		float ddx = SmoothStepD1(x_frac) * (-n0 + n1 + (n0 - n1 - n2 + n3) * Sy);
		float ddy = SmoothStepD1(y_frac) * (-n0 + n2 + (n0 - n1 - n2 + n3) * Sx);

		return Vector3(g0, ddx, ddy);
	}

	template <int N>
	struct NoiseMap {
		static_assert(N > 1, "NoiseMap size must be greater than 1");

		NoiseMap() : Map(std::make_unique<Vector4[]>(N* N)) {}

		int Seed = 0;
		int X = 0;
		int Y = 0;
		float Scale = 1.f;
		float Frequency = 1.f;
		int Size = N;
		std::unique_ptr<Vector4[]> Map;


		// Accessor
		Vector4 GetVector(int x, int y) const {
			return Map[x + N * y];
		}
		void SetVector(int x, int y, const Vector4& vec4) {
			Map[x + N * y] = vec4;
		}

		// Operator overloading — return new map
		NoiseMap<N> operator+(const NoiseMap<N>& other) const {
			NoiseMap<N> result = *this;
			for (int i = 0; i < N * N; ++i) {
				result.Map[i] += other.Map[i];
			}
			return result;
		}
		NoiseMap<N> operator-(const NoiseMap<N>& other) const {
			NoiseMap<N> result = *this;
			for (int i = 0; i < N * N; ++i) {
				result.Map[i] -= other.Map[i];
			}
			return result;
		}
		NoiseMap<N> operator*(const NoiseMap<N>& other) const {
			NoiseMap<N> result = *this;
			for (int i = 0; i < N * N; ++i) {
				result.Map[i] *= other.Map[i];
			}
			return result;
		}
		NoiseMap<N> operator*(float scalar) const {
			NoiseMap<N> result = *this;
			for (int i = 0; i < N * N; ++i) {
				result.Map[i] *= scalar;
			}
			return result;
		}

		// Optionally: compound assignment operators for efficiency
		NoiseMap<N>& operator+=(const NoiseMap<N>& other) {
			for (int i = 0; i < N * N; ++i) {
				Map[i] += other.Map[i];
			}
			return *this;
		}

		// Noise generation
		void GenerateNoiseMap_Spectrum() {
			float ds = 1.f / (N - 1.f);
			for (int y = 0; y < N; ++y) {
				for (int x = 0; x < N; ++x) {
					float dx = 2.f * x * ds - 1.f;
					float dy = 2.f * y * ds - 1.f;

					float cx = X + dx / (2.f);
					float cy = Y + dy / (2.f);

					cx *= Frequency*Scale;
					cy *= Frequency*Scale;

					float n0 = SmoothNoise(0.001f * cx, 0.001f * cy, Seed - 61);
					float n1 = SmoothNoise(0.125f * cx, 0.125f * cy, Seed + 5);
					float n2 = SmoothNoise(1.0f * cx, 1.0f * cy, Seed - 1);
					float n3 = SmoothNoise(8.0f * cx, 8.0f * cy, Seed + 1);

					SetVector(x, y, Vector4(n0, n1, n2, n3));
				}
			}
		}

		// Noise generation
		void GenerateNoiseMap_HeightDXDYMask(Vector2 pos, int seed) {
			X = pos.x;
			Y = pos.y;
			Seed = seed;

			float ds = 1.f / (N - 1.f);
			for (int y = 0; y < N; ++y) {
				for (int x = 0; x < N; ++x) {
					float dx = 2.f * x * ds - 1.f;
					float dy = 2.f * y * ds - 1.f;

					float cx = X + dx / 2.f;
					float cy = Y + dy / 2.f;

					Vector3 noise = SmoothNoiseD1(0.125f * cx, 0.125f * cy, Seed - 61);

					SetVector(x, y, Vector4(noise.x, noise.y, noise.z, 0.f));
				}
			}
		}

		Vector4 Sample(float fx, float fy) const {
			// Clamp input coordinates
			fx = std::clamp(fx, 0.0f, static_cast<float>(N - 1));
			fy = std::clamp(fy, 0.0f, static_cast<float>(N - 1));

			int x0 = static_cast<int>(floorf(fx));
			int y0 = static_cast<int>(floorf(fy));
			int x1 = Min(x0 + 1, (N - 1));
			int y1 = Min(y0 + 1, (N - 1));

			float sx = fx - x0;
			float sy = fy - y0;

			const Vector4& A = GetVector(x0, y0);
			const Vector4& B = GetVector(x1, y0);
			const Vector4& C = GetVector(x0, y1);
			const Vector4& D = GetVector(x1, y1);

			Vector4 AB = A + (B - A) * sx;
			Vector4 CD = C + (D - C) * sx;
			Vector4 result = AB + (CD - AB) * sy;

			return result;
		}
		Vector4 NormalisedSample(float fx, float fy) const {
			return Sample(fx * (N - 1.f), fy * (N - 1.f));
		}
	};

}