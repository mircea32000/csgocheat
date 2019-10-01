#pragma once

#include "../valve_sdk/sdk.hpp"

#include <DirectXMath.h>
#include <algorithm>

#define RAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define DEG2RAD(x) DirectX::XMConvertToRadians(x)
#define M_PI 3.14159265358979323846
#define PI_F	((float)(M_PI)) 

namespace Math
{
	inline float FASTSQRT(float x)
	{
		unsigned int i = *(unsigned int*)&x;

		i += 127 << 23;
		// approximation of square root
		i >>= 1;
		return *(float*)&i;
	}
	inline float DistancePointToLine(Vector Point, Vector LineOrigin, Vector Dir)
	{
		auto PointDir = Point - LineOrigin;

		auto TempOffset = PointDir.Dot(Dir) / (Dir.x * Dir.x + Dir.y * Dir.y + Dir.z * Dir.z);
		if (TempOffset < 0.000001f)
			return FLT_MAX;

		auto PerpendicularPoint = LineOrigin + (Dir * TempOffset);

		return (Point - PerpendicularPoint).Length();
	}
	QAngle CalcAngle(const Vector& src, const Vector& dst);
	float VectorDistance(const Vector& v1, const Vector& v2);
	float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle);
	template<class T>
	void Normalize3(T& vec)
	{
		for (auto i = 0; i < 2; i++) {
			while (vec[i] < -180.0f) vec[i] += 360.0f;
			while (vec[i] >  180.0f) vec[i] -= 360.0f;
		}
		vec[2] = 0.f;
	}
	void correct_angles(QAngle& angles);
    void VectorTransform(const Vector& in1, const matrix3x4_t& in2, Vector& out);
	Vector CalculateHitboxFromMatrix(matrix3x4_t matrix[128], const Vector& bbmins, const Vector& bbmaxs, int iBone);
	Vector VectorTransform(Vector in, matrix3x4_t matrix);
    void AngleVectors(const QAngle &angles, Vector& forward);
    void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up);

	inline float get_fov(const QAngle& view_angles, const Vector& eye_pos, const Vector& target_pos)
	{
		Vector cur_view_dir{};
		const Vector target_aim_dir{ (target_pos - eye_pos).Normalized() };
		AngleVectors (view_angles, cur_view_dir);
		float dot{ target_aim_dir.Dot(cur_view_dir) };

		dot = std::clamp(dot, -1.f, 1.f);

		return RAD2DEG(acos(dot));
	}
    void VectorAngles(const Vector& forward, QAngle& angles);
    bool WorldToScreen(const Vector& in, Vector& out);
}