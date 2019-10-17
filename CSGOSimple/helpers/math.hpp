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
	inline Vector ExtrapolateTick(Vector p0, Vector v0)
	{
		return p0 + (v0 * g_GlobalVars->interval_per_tick);
	}
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
	inline void CalcAngle(Vector src, Vector dst, QAngle& angles)
	{
		Vector delta = src - dst;
		double hyp = delta.Length2D(); //delta.Length
		angles.yaw = (atan(delta.y / delta.x) * 57.295779513082f);
		angles.pitch = (atan(delta.z / hyp) * 57.295779513082f);
		angles[2] = 0.00;

		if (delta.x >= 0.0)
			angles.yaw += 180.0f;
	}
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
	inline QAngle CorrectAnglesQAngle(QAngle& angles)
	{
		correct_angles(angles);

		return angles;
	}
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

	inline void MatrixAngles(const matrix3x4_t& matrix, float* angles)
	{
		float forward[3];
		float left[3];
		float up[3];

		//
		// Extract the basis vectors from the matrix. Since we only need the Z
		// component of the up vector, we don't get X and Y.
		//
		forward[0] = matrix[0][0];
		forward[1] = matrix[1][0];
		forward[2] = matrix[2][0];
		left[0] = matrix[0][1];
		left[1] = matrix[1][1];
		left[2] = matrix[2][1];
		up[2] = matrix[2][2];

		float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

		// enough here to get angles?
		if (xyDist > 0.001f)
		{
			// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
			angles[1] = RAD2DEG(atan2f(forward[1], forward[0]));

			// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
			angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

			// (roll)	z = ATAN( left.z, up.z );
			angles[2] = RAD2DEG(atan2f(left[2], up[2]));
		}
		else	// forward is mostly Z, gimbal lock-
		{
			// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
			angles[1] = RAD2DEG(atan2f(-left[0], left[1]));

			// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
			angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

			// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
			angles[2] = 0;
		}
	}
	inline void MatrixAngles(const matrix3x4_t& matrix, QAngle& angles)
	{
		MatrixAngles(matrix, &angles.pitch);
	}
	inline void MatrixAngles(const matrix3x4_t& matrix, QAngle& angles, Vector& position)
	{
		MatrixAngles(matrix, angles);
		MatrixPosition(matrix, position);
	}
}