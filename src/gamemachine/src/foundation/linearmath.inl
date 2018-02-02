﻿#if GM_USE_DX11
using namespace DirectX;
#else
namespace glm
{
	inline vec4 combine_vec4(const vec3& v3, const vec4& v4)
	{
		return glm::vec4(v3, v4[3]);
	}

	// identity
	template <typename T>
	inline T& identity();

	template <>
	inline mat4& identity()
	{
		static mat4 m(1.f);
		return m;
	}

	template <>
	inline mat3& identity()
	{
		static mat3 m(1.f);
		return m;
	}

	template <>
	inline mat2& identity()
	{
		static mat2 m(1.f);
		return m;
	}

	template <>
	inline quat& identity()
	{
		static quat q(1, 0, 0, 0);
		return q;
	}

	//transform
	inline mat4 scale(const vec3& v)
	{
		return scale(identity<mat4>(), v);
	}

	inline mat4 scale(gm::GMfloat x, gm::GMfloat y, gm::GMfloat z)
	{
		return scale(identity<mat4>(), vec3(x, y, z));
	}

	inline mat4 translate(const vec3& v)
	{
		return translate(identity<mat4>(), v);
	}

	inline void copyToArray(const mat4& mat, gm::GMfloat* value)
	{
		const auto v = value_ptr(mat);
		memcpy_s(value, sizeof(gm::GMfloat) * 16, v, sizeof(gm::GMfloat) * 16);
	}

	inline void copyToArray(const vec4& vec, gm::GMfloat* value)
	{
		const auto v = value_ptr(vec);
		memcpy_s(value, sizeof(gm::GMfloat) * 4, v, sizeof(gm::GMfloat) * 4);
	}

	inline void copyToArray(const vec3& vec, gm::GMfloat* value)
	{
		const auto v = value_ptr(vec);
		memcpy_s(value, sizeof(gm::GMfloat) * 3, v, sizeof(gm::GMfloat) * 3);
	}

	inline bool fuzzyCompare(gm::GMfloat p1, gm::GMfloat p2)
	{
		return (gm::gmFabs(p1 - p2) <= 0.01f);
	}

	inline vec3 toInhomogeneous(const vec4& v4)
	{
		return vec3(v4.x / v4.x, v4.y / v4.w, v4.z / v4.w);
	}

	inline vec4 toHomogeneous(const vec3& v3)
	{
		return vec4(v3.x, v3.y, v3.z, 1);
	}

	inline vec3 make_vec3(const gm::GMfloat(&v)[3])
	{
		return make_vec3(static_cast<const gm::GMfloat*>(v));
	}

	template <typename T>
	inline gm::GMfloat lengthSquare(const T& left)
	{
		return dot(left, left);
	}

	inline gm::GMfloat lerp(const gm::GMfloat& start, const gm::GMfloat& end, gm::GMfloat percentage)
	{
		return percentage * (end - start) + start;
	}

	inline glm::vec3 safeNormalize(const glm::vec3& vec, const glm::vec3& n = glm::vec3(1, 0, 0))
	{
		gm::GMfloat l2 = glm::length2(vec);
		if (l2 >= FLT_EPSILON*FLT_EPSILON)
		{
			return vec / gm::gmSqrt(l2);
		}
		else
		{
			return n;
		}
	}
}
#endif

template <>
GMVec2 Zero()
{
	GMVec2 V;
#if GM_USE_DX11
	V.v_ = DirectX::XMVectorZero();
#else
	V.v_ = glm::zero<glm::vec2>();
#endif
	return V;
}

template <>
GMVec3 Zero()
{
	GMVec3 V;
#if GM_USE_DX11
	V.v_ = DirectX::XMVectorZero();
#else
	V.v_ = glm::zero<glm::vec3>();
#endif
	return V;
}

template <>
GMVec4 Zero()
{
	GMVec4 V;
#if GM_USE_DX11
	V.v_ = DirectX::XMVectorZero();
#else
	V.v_ = glm::zero<glm::vec4>();
#endif
	return V;
}

inline GMMat4 __getIdentityMat4()
{
	GMMat4 v;
#if GM_USE_DX11
	v.v_ = DirectX::XMMatrixIdentity();
#else
	v.v_ = glm::identity<glm::mat4>();
#endif
	return v;
}

inline GMQuat __getIdentityQuat()
{
	GMQuat v;
#if GM_USE_DX11
	v.v_ = DirectX::XMQuaternionIdentity();
#else
	v.v_ = glm::identity<glm::quat>();
#endif
	return v;
}

inline GMVec2 operator-(const GMVec2& V)
{
	GMVec2 R;
	R.v_ = -V.v_;
	return R;
}

inline GMVec3 operator-(const GMVec3& V)
{
	GMVec3 R;
	R.v_ = -V.v_;
	return R;
}

inline GMVec4 operator-(const GMVec4& V)
{
	GMVec4 R;
	R.v_ = -V.v_;
	return R;
}

inline GMVec2 operator+(const GMVec2& V1, const GMVec2& V2)
{
	GMVec2 V;
	V.v_ = V1.v_ + V2.v_;
	return V;
}

inline GMVec2 operator-(const GMVec2& V1, const GMVec2& V2)
{
	GMVec2 V;
	V.v_ = V1.v_ - V2.v_;
	return V;
}

inline GMVec3 operator+(const GMVec3& V1, const GMVec3& V2)
{
	GMVec3 V;
	V.v_ = V1.v_ + V2.v_;
	return V;
}

inline GMVec3 operator-(const GMVec3& V1, const GMVec3& V2)
{
	GMVec3 V;
	V.v_ = V1.v_ - V2.v_;
	return V;
}

inline GMVec4 operator+(const GMVec4& V1, const GMVec4& V2)
{
	GMVec4 V;
	V.v_ = V1.v_ + V2.v_;
	return V;
}

inline GMVec4 operator-(const GMVec4& V1, const GMVec4& V2)
{
	GMVec4 V;
	V.v_ = V1.v_ - V2.v_;
	return V;
}

inline GMMat4 operator*(const GMMat4& M1, const GMMat4& M2)
{
	GMMat4 result;
#if GM_USE_DX11
	result.v_ = M1.v_ * M2.v_;
#else
	// opengl为列优先，为了计算先M1变换，再M2变换，应该用M2 * M1
	result.v_ = M2.v_ * M1.v_;
#endif
	return result;
}

inline GMVec4 operator*(const GMVec4& V, const GMMat4& M)
{
	GMVec4 result;
#if GM_USE_DX11
	result.v_ = DirectX::XMVector3Transform(V.v_, M.v_);
#else
	result.v_ = M.v_ * V.v_;
#endif
	return result;
}

inline GMMat4 QuatToMatrix(const GMQuat& quat)
{
	GMMat4 mat;
#if GM_USE_DX11
	mat.v_ = DirectX::XMMatrixRotationQuaternion(quat.v_);
#else
	mat.v_ = glm::mat4_cast(quat.v_);
#endif
	return mat;
}

inline GMMat4 LookAt(const GMVec3& position, const GMVec3& center, const GMVec3& up)
{
	GMMat4 mat;
#if GM_USE_DX11
	mat.v_ = DirectX::XMMatrixLookAtLH(position.v_, center.v_, up.v_);
#else
	mat.v_ = glm::lookAt(position.v_, center.v_, up.v_);
#endif
	return mat;
}

inline gm::GMfloat Dot(const GMVec2& V1, const GMVec2& V2)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector2Dot(V1.v_, V2.v_));
#else
	return glm::dot(V1.v_, V2.v_);
#endif
}

inline gm::GMfloat Dot(const GMVec3& V1, const GMVec3& V2)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector3Dot(V1.v_, V2.v_));
#else
	return glm::dot(V1.v_, V2.v_);
#endif
}

inline gm::GMfloat Dot(const GMVec4& V1, const GMVec4& V2)
{
#if GM_USE_DX11
	return DirectX::XMVectorGetX(DirectX::XMVector4Dot(V1.v_, V2.v_));
#else
	return glm::dot(V1.v_, V2.v_);
#endif
}

inline GMVec3 Normalize(const GMVec3& V)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMVector3Normalize(V.v_);
#else
	R.v_ = glm::normalize(R.v_);
#endif
	return R;
}

inline GMVec3 FastNormalize(const GMVec3& V)
{
	GMVec3 R;
#if GM_USE_DX11
	R.v_ = DirectX::XMVector3Normalize(V.v_);
#else
	R.v_ = glm::fastNormalize(R.v_);
#endif
	return R;
}

inline GMVec3 MakeVector3(const gm::GMfloat* f)
{
	return GMVec3(f[0], f[1], f[2]);
}

inline GMMat4 Translate(const GMVec3& V)
{
	GMMat4 M;
#if GM_USE_DX11
	M.v_ = DirectX::XMMatrixTranslationFromVector(V.v_);
#else
	M.v_ = glm::translate(R.v_);
#endif
	return M;
}

inline GMMat4 Translate(const GMVec4& V)
{
	GMMat4 M;
#if GM_USE_DX11
	M.v_ = DirectX::XMMatrixTranslationFromVector(V.v_);
#else
	M.v_ = glm::translate(R.v_);
#endif
	return M;
}

inline void GetTranslationFromMatrix(const GMMat4& M, OUT GMFloat4& F)
{
	GMFloat16 f16;
	M.loadFloat16(f16);
	F[0] = f16[3][0];
	F[1] = f16[3][1];
	F[2] = f16[3][2];
}

inline void GetScalingFromMatrix(const GMMat4& M, OUT GMFloat4& F)
{
	GMFloat16 f16;
	M.loadFloat16(f16);
	F[0] = f16[0][0];
	F[1] = f16[1][1];
	F[2] = f16[2][2];
}

inline GMMat4 Ortho(gm::GMfloat left, gm::GMfloat right, gm::GMfloat bottom, gm::GMfloat top, gm::GMfloat n, gm::GMfloat f)
{
	GMMat4 M;
#if GM_USE_DX11
	M.v_ = DirectX::XMMatrixOrthographicOffCenterLH(left, right, bottom, top, n, f);
#else
	M.v_ = glm::ortho(left, right, bottom, top, n, f);
#endif
}