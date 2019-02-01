#pragma once
#include <iostream>
#include "GeneralMath.h"

namespace Happic { namespace Math {

	template<typename T, unsigned int D>
	class Vector
	{
	public:
		inline Vector() {}
		inline Vector(const Vector<T, D>& vec)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] = vec[i];
			}
		}

		inline Vector(const Vector<T, D - 1>& vec, T lastComponent = T(0))
		{
			for (unsigned int i = 0; i < D - 1; i++)
			{
				m_values[i] = vec[i];
			}

			m_values[D - 1] = lastComponent
		}

		inline Vector(T value)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] = value;
			}
		}

		T Dot(const Vector<T, D>& vec) const
		{
			T value = T(0);

			for (unsigned int i = 0; i < D; i++)
			{
				value += m_values[i] * vec[i];
			}

			return value;
		}

		inline T Length() const { return sqrt(Dot(*this)); }

		inline Vector<T, D> Normalized() const
		{
			T length = Length();
			Vector<T, D> normalized;

			for (unsigned int i = 0; i < D; i++)
			{
				normalized[i] = m_values[i] / length;
			}

			return normalized;
		}

		inline Vector<T, D> GetNegative() const
		{
			Vector<T, D> negative;

			for (unsigned int i = 0; i < D; i++)
			{
				negative[i] = -m_values[i];
			}

			return negative;
		}

		inline void Set(const Vector<T, D>& vec)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] = vec[i];
			}
		}

		inline T Min() const
		{
			T min = m_values[0];

			for (unsigned int i = 1; i < D; i++)
			{
				if (m_values[i] < min)
				{
					min = m_values[i];
				}
			}

			return min;
		}

		inline T Max() const
		{
			T max = m_values[0];

			for (unsigned int i = 1; i < D; i++)
			{
				if (m_values[i] > max)
				{
					max = m_values[i];
				}
			}

			return max;
		}

		inline Vector<T, D> Min(const Vector<T, D>& vec) const
		{
			Vector<T, D> min;

			for (unsigned int i = 0; i < D; i++)
			{
				min[i] = MATH_MIN(m_values[i], vec[i]);
			}

			return min;
		}

		inline Vector<T, D> Max(const Vector<T, D>& vec) const
		{
			Vector<T, D> max;

			for (unsigned int i = 0; i < D; i++)
			{
				max[i] = MATH_MAX(m_values[i], vec[i]);
			}

			return max;
		}

		inline Vector<T, D> Lerp(const Vector<T, D>& vec, float t) const
		{
			return (*this * t + vec * (1.0f - t));
		}

		inline Vector<T, D> Lerp(float vec, float t) const
		{
			return (*this * t + vec * (1.0f - t));
		}

		inline Vector<T, D> Floor() const
		{
			Vector<T, D> floor;

			for (unsigned int i = 0; i < D; i++)
			{
				floor[i] = floorf(m_values[i]);
			}

			return floor;
		}

		inline Vector<T, D> Ceil() const
		{
			Vector<T, D> ceil;

			for (unsigned int i = 0; i < D; i++)
			{
				ceil[i] = ceilf(m_values[i]);
			}

			return ceil;
		}

		inline Vector<T, D> Round() const
		{
			Vector<T, D> round;

			for (unsigned int i = 0; i < D; i++)
			{
				round[i] = roundf(m_values[i]);
			}

			return round;
		}

		inline Vector<T, D> operator+(const Vector<T, D>& vec) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] + vec[i];
			}

			return res;
		}

		inline Vector<T, D> operator-(const Vector<T, D>& vec) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] - vec[i];
			}

			return res;
		}

		inline Vector<T, D> operator*(const Vector<T, D>& vec) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] * vec[i];
			}

			return res;
		}

		inline Vector<T, D> operator/(const Vector<T, D>& vec) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] / vec[i];
			}

			return res;
		}

		inline Vector<T, D> operator+(T value) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] + value;
			}

			return res;
		}

		inline Vector<T, D> operator-(T value) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] - value;
			}

			return res;
		}

		inline Vector<T, D> operator*(T value) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] * value;
			}

			return res;
		}

		inline Vector<T, D> operator/(T value) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = m_values[i] / value;
			}

			return res;
		}

		inline Vector<T, D>& operator+=(const Vector<T, D>& vec)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] += vec[i];
			}

			return *this;
		}

		inline Vector<T, D>& operator-=(const Vector<T, D>& vec)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] -= vec[i];
			}

			return *this;
		}

		inline Vector<T, D>& operator*=(const Vector<T, D>& vec)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] *= vec[i];
			}

			return *this;
		}

		inline Vector<T, D>& operator/=(const Vector<T, D>& vec)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] /= vec[i];
			}

			return *this;
		}

		inline Vector<T, D>& operator+=(T value)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] += value;
			}

			return *this;
		}

		inline Vector<T, D>& operator-=(T value)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] -= value;
			}

			return *this;
		}

		inline Vector<T, D>& operator*=(T value)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] *= value;
			}

			return *this;
		}

		inline Vector<T, D>& operator/=(T value)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				m_values[i] /= value;
			}

			return *this;
		}

		inline bool operator==(const Vector<T, D>& vec) const
		{
			for (unsigned int i = 0; i < D; i++)
			{
				if (m_values[i] != vec[i])
				{
					return false;
				}
			}

			return true;
		}

		inline bool operator!=(const Vector<T, D>& vec) const
		{
			return !operator==(vec);
		}

		friend std::ostream& operator<< (std::ostream& output, const Vector<T, D>& vec)
		{
			output << "Vec" << T(D) << "(" << vec[0];

			for (unsigned int i = 1; i < D; i++)
			{
				output << ", " << vec[i];
			}

			output << ")";

			return output;
		}

		T& operator [] (unsigned int i) { return m_values[i]; }
		T operator [] (unsigned int i) const { return m_values[i]; }
	private:
		T m_values[D];
	};

	template<typename T>
	class Vector2 : public Vector<T, 2>
	{
	public:
		inline Vector2()
		{
			SetX(0);
			SetY(0);
		}

		inline Vector2(T x, T y)
		{
			SetX(x);
			SetY(y);
		}

		inline Vector2(const Vector<T, 2>& vec)
		{
			SetX(vec[0]);
			SetY(vec[1]);
		}

		inline T GetX() const { return (*this)[0]; }
		inline T GetY() const { return (*this)[1]; }
		inline void SetX(T x) { (*this)[0] = x; }
		inline void SetY(T y) { (*this)[1] = y; }
		inline void Set(T x, T y) { SetX(x); SetY(y); }
	};

	template <typename T>
	class Vector3 : public Vector<T, 3>
	{
	public:
		inline Vector3()
		{
			SetX(0);
			SetY(0);
			SetZ(0);
		}

		inline Vector3(T x, T y, T z)
		{
			SetX(x);
			SetY(y);
			SetZ(z);
		}

		inline Vector3(const Vector<T, 3>& vec)
		{
			SetX(vec[0]);
			SetY(vec[1]);
			SetZ(vec[2]);
		}

		inline Vector3<T> Cross(const Vector3<T>& vec) const
		{
			Vector3<float>(GetY() * vec.GetZ() - GetZ() * vec.GetY(),
				GetZ() * vec.GetX() - GetX() * vec.GetZ(),
				GetX() * vec.GetY() - GetY() * vec.GetX());
		}

		inline Vector3<T> Rotate(const Vector3<T>& axis, T angle) const
		{
			const T sinAngle = sin(-angle);
			const T cosAngle = cos(-angle);

			return this->Cross(axis * sinAngle) +        //Rotation on local X
				(*this * cosAngle) +                     //Rotation on local Z
				axis * this->Dot(axis * (T(1) - cosAngle)); //Rotation on local Y
		}

		inline T GetX() const { return (*this)[0]; }
		inline T GetY() const { return (*this)[1]; }
		inline T GetZ() const { return (*this)[2]; }
		inline Vector2<T> GetXY() const { return Vector2<T>(GetX(), GetY()); }
		inline void SetX(T x) { (*this)[0] = x; }
		inline void SetY(T y) { (*this)[1] = y; }
		inline void SetZ(T z) { (*this)[2] = z; }
		inline void Set(T x, T y, T z) { SetX(x); SetY(y); SetZ(z); }
		inline void Set(const Vector3<T>& vec) { Set(vec.GetX(), vec.GetY(), vec.GetZ()); }
	};

	template<typename T>
	class Vector4 : public Vector<T, 4>
	{
	public:
		inline Vector4()
		{
			SetX(0);
			SetY(0);
			SetZ(0);
			SetW(0);
		}

		inline Vector4(T x, T y, T z, T w)
		{
			SetX(x);
			SetY(y);
			SetZ(z);
			SetW(w);
		}

		inline Vector4(const Vector<T, 4>& vec)
		{
			SetX(vec[0]);
			SetY(vec[1]);
			SetZ(vec[2]);
			SetW(vec[3]);
		}

		inline T GetX() const { return (*this)[0]; }
		inline T GetY() const { return (*this)[1]; }
		inline T GetZ() const { return (*this)[2]; }
		inline T GetW() const { return (*this)[3]; }
		inline void SetX(T x) { (*this)[0] = x; }
		inline void SetY(T y) { (*this)[1] = y; }
		inline void SetZ(T z) { (*this)[2] = z; }
		inline void SetW(T w) { (*this)[3] = w; }
		inline void Set(T x, T y, T z, T w) { SetX(x); SetY(y); SetZ(z); SetW(w); }
		inline void Set(const Vector4<float>& vec) { Set(vec.GetX(), vec.GetY(), vec.GetZ(), vec.GetW()); }
	};

	class Quaternion;

	class Vector3f : public Vector3<float>
	{
	public:
		Vector3f(float x = 0.0f, float y = 0.0f, float z = 0.0f)
		{
			Set(x, y, z);
		}
		Vector3f(const Vector<float, 3>& vec)
		{
			Set(vec[0], vec[1], vec[2]);
		}

		Vector3f Rotate(const Quaternion& quat) const;
	};

	class Attenuation : public Vector<float, 3>
	{
	public:
		inline Attenuation(float constant = 0.0f, float linear = 0.0f, float exponent = 1.0f)
		{
			Set(constant, linear, exponent);
		}

		inline float GetConstant() const { return (*this)[0]; }
		inline float GetLinear() const { return (*this)[1]; }
		inline float GetQuadratic() const { return (*this)[2]; }

		inline void SetConstant(float constant) { (*this)[0] = constant; }
		inline void SetLinear(float linear) { (*this)[1] = linear; }
		inline void SetQuadratic(float quadratic) { (*this)[2] = quadratic; }

		inline void Set(float constant, float linear, float quadratic)
		{
			SetConstant(constant);
			SetLinear(linear);
			SetQuadratic(quadratic);
		}

		inline void Set(const Attenuation& attenuation)
		{
			SetConstant(attenuation.GetConstant());
			SetLinear(attenuation.GetLinear());
			SetQuadratic(attenuation.GetQuadratic());
		}
	};

	typedef Vector2<float> Vector2f;
	typedef Vector4<float> Vector4f;

	typedef Vector2<double> Vector2d;
	typedef Vector3<double> Vector3d;
	typedef Vector4<double> Vector4d;

	typedef Vector2<int> Vector2i;
	typedef Vector3<int> Vector3i;
	typedef Vector4<int> Vector4i;

	template<typename T, unsigned int D>
	class Matrix
	{
	public:
		inline Matrix() {}

		inline Matrix(const Matrix<T, D>& mat)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				for (unsigned int j = 0; j < D; j++)
				{
					m_values[i][j] = mat[i][j];
				}
			}
		}

		inline Matrix(T diagonal)
		{
			for (unsigned int i = 0; i < D; i++)
			{
				for (unsigned int j = 0; j < D; j++)
				{
					m_values[i][j] = i == j ? diagonal : T(0);
				}
			}
		}

		inline Matrix<T, D> GetTransposed() const
		{
			Matrix<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				for (unsigned int j = 0; j < D; j++)
				{
					res[i][j] = m_values[j][i];
				}
			}

			return res;
		}

		inline Matrix<T, D>& InitIdentity()
		{
			for (unsigned int i = 0; i < D; i++)
			{
				for (unsigned int j = 0; j < D; j++)
				{
					m_values[i][j] = i == j ? T(1) : T(0);
				}
			}

			return *this;
		}

		inline T GetDeterminate() const
		{
			for (unsigned int i = 0; i < D; i++)
			{

			}

			return T(0);
		}

		inline Matrix<T, D> GetInverse() const
		{
			Matrix<T, D> res;
			return res;
		}

		inline Matrix<T, D> operator*(const Matrix<T, D>& mat) const
		{
			Matrix<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				for (unsigned int j = 0; j < D; j++)
				{
					res[i][j] = T(0);
					for (unsigned int k = 0; k < D; k++)
					{
						res[i][j] += m_values[i][k] * mat[k][j];
					}
				}
			}

			return res;
		}

		inline Vector<T, D> operator*(const Vector<T, D>& vec) const
		{
			Vector<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				res[i] = T(0);

				for (unsigned int j = 0; j < D; j++)
				{
					res[i] += m_values[i][j] * vec[j];
				}
			}

			return res;
		}

		inline Vector<T, D - 1> operator*(const Vector<T, D - 1>& vec) const
		{
			Vector<T, D - 1> res;

			for (unsigned int i = 0; i < D - 1; i++)
			{
				res[i] = T(0);

				for (unsigned int j = 0; j < D; j++)
				{
					if (j == D - 1)
					{
						res[i] += m_values[i][j] * 1;
						break;
					}
					res[i] += m_values[i][j] * vec[j];
				}
			}

			return res;
		}

		inline Matrix<T, D> operator+(const Matrix<T, D>& mat) const
		{
			Matrix<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				for (unsigned int j = 0; j < D; j++)
				{
					res[i][j] = m_values[i][j] + mat[i][j];
				}
			}

			return res;
		}

		inline Matrix<T, D> operator-(const Matrix<T, D>& mat) const
		{
			Matrix<T, D> res;

			for (unsigned int i = 0; i < D; i++)
			{
				for (unsigned int j = 0; j < D; j++)
				{
					res[i][j] = m_values[i][j] - mat[i][j];
				}
			}

			return res;
		}

		inline const T* operator[](int index) const { return m_values[index]; }
		inline T* operator[](int index) { return m_values[index]; }
	private:
		T m_values[D][D];
	};

	template<typename T>
	class Matrix2 : public Matrix<T, 2>
	{
	public:
		inline Matrix2() {}

		inline Matrix2(T m00, T m01, T m10, T m11)
		{
			(*this)[0][0] = m00; (*this)[0][1] = m01;
			(*this)[1][0] = m10; (*this)[1][1] = m11;
		}

		inline Matrix2(const Matrix<T, 2>& mat)
		{
			for (unsigned int i = 0; i < 2; i++)
			{
				for (unsigned int j = 0; j < 2; j++)
				{
					(*this)[i][j] = mat[i][j];
				}
			}
		}

		inline Matrix2<T>& InitRotation(T deg)
		{
			T rad = MATH_DEG_TO_RAD(deg);
			T _cos = cos(rad);
			T _sin = sin(rad);

			(*this)[0][0] = _cos; (*this)[0][1] = -_sin;
			(*this)[1][0] = _sin; (*this)[1][1] = _cos;

			return *this;
		}

		inline Matrix2<T>& InitScale(const Vector2<T>& scale)
		{
			(*this)[0][0] = scale.GetX(); (*this)[0][1] = T(0);
			(*this)[1][0] = T(0);		  (*this)[1][1] = scale.GetY();

			return *this;
		}
	};

	template<typename T>
	class Matrix3 : public Matrix<T, 3>
	{
	public:
		inline Matrix3() {}

		inline Matrix3(T m00, T m01, T m02,
			T m10, T m11, T m12,
			T m20, T m21, T m22)
		{
			(*this)[0][0] = m00; (*this)[0][1] = m01; (*this)[0][2] = m02;
			(*this)[1][0] = m10; (*this)[1][1] = m11; (*this)[1][2] = m12;
			(*this)[2][0] = m20; (*this)[2][1] = m21; (*this)[2][2] = m22;
		}

		inline Matrix3(const Matrix<T, 3>& mat)
		{
			for (unsigned int i = 0; i < 3; i++)
			{
				for (unsigned int j = 0; j < 3; j++)
				{
					(*this)[i][j] = mat[i][j];
				}
			}
		}

		inline Matrix3<T>& InitTranslation(const Vector2<T>& translation)
		{
			(*this)[0][0] = 1; (*this)[0][1] = 0; (*this)[0][2] = translation.GetX();
			(*this)[1][0] = 0; (*this)[1][1] = 1; (*this)[1][2] = translation.GetY();
			(*this)[2][0] = 0; (*this)[2][1] = 0; (*this)[2][2] = 1;

			return *this;
		}

		inline Matrix3<T>& InitScale(const Vector2<T>& scale)
		{
			(*this)[0][0] = scale.GetX(); (*this)[0][1] = T(0);			(*this)[0][2] = T(0);
			(*this)[1][0] = T(0);		  (*this)[1][1] = scale.GetY(); (*this)[1][2] = T(0);
			(*this)[2][0] = T(0);		  (*this)[2][1] = T(0);			(*this)[2][2] = T(1);

			return *this;
		}

		inline Matrix3<T>& InitRotation(T deg)
		{
			T rad = MATH_DEG_TO_RAD(deg);
			T _cos = cos(-rad);
			T _sin = sin(-rad);

			(*this)[0][0] = _cos; (*this)[0][1] = -_sin; (*this)[0][2] = T(0);
			(*this)[1][0] = _sin; (*this)[1][1] = _cos;  (*this)[1][2] = T(0);
			(*this)[2][0] = T(0); (*this)[2][1] = T(0);   (*this)[2][2] = T(1);

			return *this;
		}
	};

	template<typename T>
	class Matrix4 : public Matrix<T, 4>
	{
	public:
		inline Matrix4() {}

		inline Matrix4(T m00, T m01, T m02, T m03,
			T m10, T m11, T m12, T m13,
			T m20, T m21, T m22, T m23,
			T m30, T m31, T m32, T m33)
		{
			(*this)[0][0] = m00; (*this)[0][1] = m01; (*this)[0][2] = m02; (*this)[0][3] = m03;
			(*this)[1][0] = m10; (*this)[1][1] = m11; (*this)[1][2] = m12; (*this)[1][3] = m13;
			(*this)[2][0] = m20; (*this)[2][1] = m21; (*this)[2][2] = m22; (*this)[2][3] = m23;
			(*this)[3][0] = m30; (*this)[3][1] = m31; (*this)[3][2] = m32; (*this)[3][3] = m33;
		}

		inline Matrix4(const Matrix<T, 4>& mat)
		{
			for (unsigned int i = 0; i < 4; i++)
			{
				for (unsigned int j = 0; j < 4; j++)
				{
					(*this)[i][j] = mat[i][j];
				}
			}
		}

		inline Matrix4<T>& InitTranslation(const Vector3<T>& translation)
		{
			(*this)[0][0] = T(1); (*this)[0][1] = T(0); (*this)[0][2] = T(0); (*this)[0][3] = translation.GetX();
			(*this)[1][0] = T(0); (*this)[1][1] = T(1); (*this)[1][2] = T(0); (*this)[1][3] = translation.GetY();
			(*this)[2][0] = T(0); (*this)[2][1] = T(0); (*this)[2][2] = T(1); (*this)[2][3] = translation.GetZ();
			(*this)[3][0] = T(0); (*this)[3][1] = T(0); (*this)[3][2] = T(0); (*this)[3][3] = T(1);

			return *this;
		}

		inline Matrix4<T>& InitScale(const Vector3<T>& scale)
		{
			(*this)[0][0] = scale.GetX(); (*this)[0][1] = T(0);			(*this)[0][2] = T(0);		  (*this)[0][3] = T(0);
			(*this)[1][0] = T(0);		  (*this)[1][1] = scale.GetY(); (*this)[1][2] = T(0);		  (*this)[1][3] = T(0);
			(*this)[2][0] = T(0);		  (*this)[2][1] = T(0);			(*this)[2][2] = scale.GetZ(); (*this)[2][3] = T(0);
			(*this)[3][0] = T(0);		  (*this)[3][1] = T(0);			(*this)[3][2] = T(0);		  (*this)[3][3] = T(1);

			return *this;
		}

		inline Matrix4<T>& InitRotationX(T deg)
		{
			T rad = MATH_DEG_TO_RAD(degrees);

			T cos = cosf(rad);
			T sin = sinf(rad);

			(*this)[0][0] = 1; (*this)[0][1] = 0;   (*this)[0][2] = 0;    (*this)[0][3] = 0;
			(*this)[1][0] = 0; (*this)[1][1] = cos; (*this)[1][2] = -sin; (*this)[1][3] = 0;
			(*this)[2][0] = 0; (*this)[2][1] = sin; (*this)[2][2] = cos;  (*this)[2][3] = 0;
			(*this)[3][0] = 0; (*this)[3][1] = 0;   (*this)[3][2] = 0;    (*this)[3][3] = 1;

			return *this;
		}

		inline Matrix4<T>& InitRotationY(T deg)
		{
			float rad = MATH_DEG_TO_RAD(degrees);

			float cos = cosf(rad);
			float sin = sinf(rad);

			(*this)[0][0] = cos;  (*this)[0][1] = 0; (*this)[0][2] = sin; (*this)[0][3] = 0;
			(*this)[1][0] = 0;    (*this)[1][1] = 1; (*this)[1][2] = 0;   (*this)[1][3] = 0;
			(*this)[2][0] = -sin; (*this)[2][1] = 0; (*this)[2][2] = cos; (*this)[2][3] = 0;
			(*this)[3][0] = 0;    (*this)[3][1] = 0; (*this)[3][2] = 0;   (*this)[3][3] = 1;

			return *this;
		}

		inline Matrix4<T>& InitRotationZ(T deg)
		{
			float rad = MATH_DEG_TO_RAD(deg);

			float cos = cosf(rad);
			float sin = sinf(rad);

			(*this)[0][0] = cos; (*this)[0][1] = -sin; (*this)[0][2] = 0; (*this)[0][3] = 0;
			(*this)[1][0] = sin; (*this)[1][1] = cos;  (*this)[1][2] = 0; (*this)[1][3] = 0;
			(*this)[2][0] = 0;   (*this)[2][1] = 0;    (*this)[2][2] = 1; (*this)[2][3] = 0;
			(*this)[3][0] = 0;   (*this)[3][1] = 0;    (*this)[3][2] = 0; (*this)[3][3] = 1;

			return *this;
		}

		inline Matrix4<T>& InitRotation(const Vector3<T>& rotation)
		{
			Matrix4<T> rotX = Matrix4<T>().InitRotationX(rotation.GetX());
			Matrix4<T> rotY = Matrix4<T>().InitRotationY(rotation.GetY());
			Matrix4<T> rotZ = Matrix4<T>().InitRotationZ(rotation.GetZ());

			*this = rotX * (rotY * rotZ);

			return *this;
		}

		inline Matrix4<T>& InitPerspective(T fov, T width, T height, T znear, T zfar)
		{
			T ar = width / height;
			T halfTanFOV = (T)tan(MATH_DEG_TO_RAD(fov) / 2);
			T zrange = znear - zfar;

			(*this)[0][0] = 1.0f / (halfTanFOV * ar); (*this)[0][1] = 0;				 (*this)[0][2] = 0;						   (*this)[0][3] = 0;
			(*this)[1][0] = 0;						  (*this)[1][1] = 1.0f / halfTanFOV; (*this)[1][2] = 0;						   (*this)[1][3] = 0;
			(*this)[2][0] = 0;						  (*this)[2][1] = 0;				 (*this)[2][2] = (-znear - zfar) / zrange; (*this)[2][3] = 2.0f * zfar * znear / zrange;
			(*this)[3][0] = 0;						  (*this)[3][1] = 0;			     (*this)[3][2] = 1.0f;                     (*this)[3][3] = 0;

			return *this;;
		}

		inline Matrix4<T> InitOrthographic(T left, T right, T top, T bottom, T near, T far)
		{
			(*this)[0][0] = 2.0f / (right - left); (*this)[0][1] = T(0);					  (*this)[0][2] = T(0);					(*this)[0][3] = -((right + left) / (right - left));
			(*this)[1][0] = T(0);				   (*this)[1][1] = 2.0f / (top - bottom);	  (*this)[1][2] = T(0);					(*this)[1][3] = -((top + bottom) / (top - bottom));
			(*this)[2][0] = T(0);				   (*this)[2][1] = T(0);					  (*this)[2][2] = -2.0f / (far - near); (*this)[2][3] = -((far + near) / (far - near));
			(*this)[3][0] = T(0);				   (*this)[3][1] = T(0);					  (*this)[3][2] = T(0);					(*this)[3][3] = T(1);

			return *this;
		}

		inline Matrix4<T>& InitRotation(const Vector3<T>& forward, const Vector3<T>& up, const Vector3<T>& right)
		{
			(*this)[0][0] = right.GetX();   (*this)[0][1] = right.GetY();   (*this)[0][2] = right.GetZ();   (*this)[0][3] = 0;
			(*this)[1][0] = up.GetX();      (*this)[1][1] = up.GetY();      (*this)[1][2] = up.GetZ();	     (*this)[1][3] = 0;
			(*this)[2][0] = forward.GetX(); (*this)[2][1] = forward.GetY(); (*this)[2][2] = forward.GetZ(); (*this)[2][3] = 0;
			(*this)[3][0] = 0;				(*this)[3][1] = 0;				(*this)[3][2] = 0;				(*this)[3][3] = 1;

			return *this;
		}

		inline Matrix4<T>& InitRotation(const Vector3<T>& forward, const Vector3<T>& up)
		{
			Vector3<T> right = up.Cross(forward);
			Vector3<T> newUp = forward.Cross(right);

			return InitRotation(forward, newUp, right);
		}
	};

	typedef Matrix2<float> Matrix2f;
	typedef Matrix3<float> Matrix3f;
	typedef Matrix4<float> Matrix4f;

	typedef Matrix2<double> Matrix2d;
	typedef Matrix3<double> Matrix3d;
	typedef Matrix4<double> Matrix4d;

	typedef Matrix2<int> Matrix2i;
	typedef Matrix3<int> Matrix3i;
	typedef Matrix4<int> Matrix4i;

	class Complex : public Vector<float, 2>
	{
	public:
		Complex(float real, float imaginary)
		{
			(*this)[0] = real;
			(*this)[1] = imaginary;
		}



		inline float GetReal() const { return (*this)[0]; }
		inline float GetImaginary() const { return (*this)[1]; }
		inline void SetReal(float real) { (*this)[0] = real; }
		inline void SetImaginary(float imaginary) { (*this)[1] = imaginary; }
		inline void Set(float real, float imaginary) { SetReal(real); SetImaginary(imaginary); }
		inline void Set(const Complex& complex) { SetReal(complex.GetReal()); SetImaginary(complex.GetImaginary()); }
	};

	class Quaternion : public Vector4f
	{
	public:
		inline Quaternion(float x = 0, float y = 0, float z = 0, float w = 1)
		{
			Set(x, y, z, w);
		}

		inline Quaternion(const Vector3f& axis, float angle)
		{
			float sinHalfAngle = (float)sin(MATH_DEG_TO_RAD(angle) / 2.0f);
			float cosHalfAngle = (float)cos(MATH_DEG_TO_RAD(angle) / 2.0f);

			SetX(axis.GetX() * sinHalfAngle);
			SetY(axis.GetY() * sinHalfAngle);
			SetZ(axis.GetZ() * sinHalfAngle);
			SetW(cosHalfAngle);
		}

		inline Quaternion(const Vector4<float>& vec)
		{
			Set(vec.GetX(), vec.GetY(), vec.GetZ(), vec.GetW());
		}

		inline Quaternion(const Matrix4f& rotationMatrix)
		{
			SetW(sqrtf(1.0 + rotationMatrix[0][0] + rotationMatrix[1][1] + rotationMatrix[2][2]) / 2.0f);
			float w4 = GetW() * 4;
			SetX((rotationMatrix[2][1] - rotationMatrix[1][2]) / w4);
			SetY((rotationMatrix[0][2] - rotationMatrix[2][0]) / w4);
			SetZ((rotationMatrix[1][0] - rotationMatrix[0][1]) / w4);
		}

		inline Quaternion GetConjugate() const
		{
			return Quaternion(-GetX(), -GetY(), -GetZ(), GetW());
		}

		inline Quaternion Slerp(const Quaternion& quat, float t) const
		{
			Quaternion q3;
			float dot = this->Dot(quat);

			if (dot < 0.0f)
			{
				dot = -dot;
				q3 = quat * -1.0f;
			}
			else
			{
				q3 = quat;
			}

			if (dot < 0.95f)
			{
				float angle = acosf(dot);
				return (*this * sinf(angle * (1.0f - t)) + q3 * sinf(angle * t)) / sinf(angle);
			}
			else
			{
				return this->Lerp(q3, t);
			}
		}


		inline Quaternion operator*(const Quaternion& quat) const
		{
			return Quaternion(GetX() * quat.GetW() + GetW() * quat.GetX() + GetY() * quat.GetZ() - GetZ() * quat.GetY(),
				GetY() * quat.GetW() + GetW() * quat.GetY() + GetZ() * quat.GetX() - GetX() * quat.GetZ(),
				GetZ() * quat.GetW() + GetW() * quat.GetZ() + GetX() * quat.GetY() - GetY() * quat.GetX(),
				GetW() * quat.GetW() - GetX() * quat.GetX() - GetY() * quat.GetY() - GetZ() * quat.GetZ());
		}

		inline Quaternion operator*(const Vector3f& vec) const
		{
			return Quaternion(GetW() * vec.GetX() + GetY() * vec.GetZ() - GetZ() * vec.GetY(),
				GetW() * vec.GetY() + GetZ() * vec.GetX() - GetX() * vec.GetZ(),
				GetW() * vec.GetZ() + GetX() * vec.GetY() - GetY() * vec.GetX(),
				-GetX() * vec.GetX() - GetY() * vec.GetY() - GetZ() * vec.GetZ());
		}

		inline Quaternion operator*(float scale) const
		{
			return Quaternion(GetX() * scale, GetY() * scale, GetZ() * scale, GetW() * scale);
		}

		inline Matrix4f CreateRotationMatrix() const
		{
			Vector3f forward(2.0f * (GetX()*GetZ() - GetW()*GetY()), 2.0f * (GetY()*GetZ() + GetW()*GetX()), 1.0f - 2.0f * (GetX()*GetX() + GetY()*GetY()));
			Vector3f up(2.0f * (GetX()*GetY() + GetW()*GetZ()), 1.0f - 2.0f * (GetX()*GetX() + GetZ()*GetZ()), 2.0f * (GetY()*GetZ() - GetW()*GetX()));
			Vector3f right(1.0f - 2.0f * (GetY()*GetY() + GetZ()*GetZ()), 2.0f * (GetX()*GetY() - GetW()*GetZ()), 2.0f * (GetX()*GetZ() + GetW()*GetY()));

			return Matrix4f().InitRotation(forward, up, right);
		}

		inline void operator=(const Quaternion& quat)
		{
			Set(quat.GetX(), quat.GetY(), quat.GetZ(), quat.GetW());
		}

		inline void operator=(const Vector4f& vec)
		{
			Set(vec.GetX(), vec.GetY(), vec.GetZ(), vec.GetW());
		}

		inline Vector3f GetForward()	const { return Vector3f(0, 0, 1).Rotate(*this); }
		inline Vector3f GetBack()		const { return Vector3f(0, 0, -1).Rotate(*this); }
		inline Vector3f GetUp()			const { return Vector3f(0, 1, 0).Rotate(*this); }
		inline Vector3f GetDown()		const { return Vector3f(0, -1, 0).Rotate(*this); }
		inline Vector3f GetLeft()		const { return Vector3f(-1, 0, 0).Rotate(*this); }
		inline Vector3f GetRight()		const { return Vector3f(1, 0, 0).Rotate(*this); }
	};

} }

