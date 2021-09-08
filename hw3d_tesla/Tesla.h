#pragma once
#include <DirectXMath.h>
#include <fstream>
#include <sstream>
#include <vector>

namespace Tesla
{
	typedef unsigned int index_type;
	
	static constexpr double PI_D     = 3.141592653589793;
	static constexpr double twoPI_D  = 2.0 * PI_D;
	static constexpr double halfPI_D = 0.5 * PI_D;

	static constexpr float PI     = static_cast<float>(PI_D    );
	static constexpr float twoPI  = static_cast<float>(twoPI_D );
	static constexpr float halfPI = static_cast<float>(halfPI_D);

	template<typename Type>
	static constexpr Type sq(const Type& arg)
	{
		return arg * arg;
	}

	template<typename Float3>
	static constexpr Float3 FromHSV(float hueRad, float saturation = 1.0f, float value = 1.0f)
	{
		while (hueRad < 0.0f)
		{
			hueRad += twoPI;
		}
		float hue = std::fmodf(hueRad * (180.0f / PI), 360.0f);

		assert(hue >= 0.0f);
		assert(hue < 360.0f);

		const float c = value * saturation;
		const float magic = 1.0f - std::abs(std::fmodf(hue / 60.0f, 2.0f) - 1.0f);
		const float x = c * magic;
		const float m = value - c;

		Float3 out;

		if (0.0f <= hue && hue < 60.0f)
		{
			out = { c,x,0.0f };
		}
		else if (60.0f <= hue && hue < 120.0f)
		{
			out = { x,c,0.0f };
		}
		else if (120.0f <= hue && hue < 180.0f)
		{
			out = { 0.0f,c,x };
		}
		else if (180.0f <= hue && hue < 240.0f)
		{
			out = { 0.0f,x,c };
		}
		else if (240.0f <= hue && hue < 300.0f)
		{
			out = { x,0.0f,c };
		}
		else if (300.0f <= hue && hue <= 360.0f)
		{
			out = { c,0.0f,x };
		}

		return { out.x + m,out.y + m,out.z + m };
	}

	template<typename Vertex>
	class IndexedTriangleList
	{
	public:
		IndexedTriangleList() = default;
		IndexedTriangleList(std::vector<Vertex> vertices_in, std::vector<index_type> indices_in)
			:
			indices(std::move(indices_in)),
			vertices(std::move(vertices_in))
		{
			assert(vertices.size() > 2 && "There are not enough vertices in the loaded IndexedTriangleList.");
			assert(indices.size() % 3 == 0 && "This is not an IndexedTriangleList! The Number of indices is not a multiple of 3.");
		}
		IndexedTriangleList& Transform(const DirectX::XMMATRIX transformation)
		{
			// apply the transformation matrix to every vertex position
			for (auto& v : vertices)
			{
				DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&v.pos), DirectX::XMVector3Transform(DirectX::XMLoadFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&v.pos)), transformation));
			}
			return *this;
		}
		IndexedTriangleList& MakeColored(bool join = true)
		{
			const float dPhi = twoPI / (float)vertices.size();
			float phi = 0.0f;
			for (auto& v : vertices)
			{
				v.col = FromHSV<DirectX::XMFLOAT3>(phi);
				phi += dPhi;
			}
			if (join)
			{
				for (index_type i = 0u; i < vertices.size(); i++)
				{
					for (index_type j = i + 1; j < vertices.size(); j++)
					{
						if (vertices[i].pos == vertices[j].pos)
						{
							vertices[i].col = vertices[j].col;
						}
					}
				}
			}

			return *this;
		}
	public:
		std::vector<index_type> indices;
		std::vector<Vertex> vertices;
	};

	template<typename Vertex>
	class IndexedLineList
	{
	public:
		IndexedLineList() = default;
		IndexedLineList(std::vector<Vertex> vertices_in, std::vector<index_type> indices_in)
			:
			indices(std::move(indices_in)),
			vertices(std::move(vertices_in))
		{
			assert(vertices.size() >= 2 && "There are not enough vertices in the loaded IndexedLineList.");
			assert(indices.size() >= 2 && "There are not enough indices in the loaded IndexedLineList!");
			assert(indices.size() % 2 == 0 && "This is not an IndexedLineList! The number of indices must be even");
		}
		IndexedLineList& Transform(const DirectX::XMMATRIX transformation)
		{
			// apply the transformation matrix to every vertex position
			for (auto& v : vertices)
			{
				DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&v.pos), DirectX::XMVector3Transform(DirectX::XMLoadFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(&v.pos)), transformation));
			}
			return *this;
		}
	public:
		std::vector<index_type> indices;
		std::vector<Vertex> vertices;
	};

	template<typename T>
	class Generic_Vec2
	{
	public:
		Generic_Vec2() = default;
		Generic_Vec2(T x, T y)
			:
			x(x),
			y(y)
		{}
		Generic_Vec2(const Generic_Vec2& v)
			:
			x(v.x),
			y(v.y)
		{}
		template<typename Other>
		Generic_Vec2& operator=(const Other& src)
		{
			this->x = src.x;
			this->y = src.y;
			return *this;
		}
	public:
		T GetLengthSq() const
		{
			return Dot(*this, *this);
		}
		T GetLength() const
		{
			return (T)std::sqrt(GetLengthSq());
		}
		Generic_Vec2& Normalize()
		{
			return (*this) /= GetLength();
		}
		Generic_Vec2 GetNormalized() const
		{
			return Generic_Vec2(*this).Normalize();
		}
	public:
		Generic_Vec2& operator+=(const Generic_Vec2& rhs)
		{
			this->x += rhs.x;
			this->y += rhs.y;
			return *this;
		}
		Generic_Vec2& operator-=(const Generic_Vec2& rhs)
		{
			this->x -= rhs.x;
			this->y -= rhs.y;
			return *this;
		}
		Generic_Vec2& operator*=(const T rhs)
		{
			this->x *= rhs;
			this->y *= rhs;
			return *this;
		}
		Generic_Vec2& operator/=(const T rhs)
		{
			this->x /= rhs;
			this->y /= rhs;
			return *this;
		}
	public:
		Generic_Vec2 operator+(const Generic_Vec2& rhs) const
		{
			return Generic_Vec2(*this) += rhs;
		}
		Generic_Vec2 operator-(const Generic_Vec2& rhs) const
		{
			return Generic_Vec2(*this) -= rhs;
		}
		Generic_Vec2 operator*(const T rhs) const
		{
			return Generic_Vec2(*this) *= rhs;
		}
		Generic_Vec2 operator/(const T rhs) const
		{
			return Generic_Vec2(*this) /= rhs;
		}
	public:
		Generic_Vec2 operator-() const
		{
			return Generic_Vec2(-this->x, -this->y);
		}
		T& operator[](const size_t i)
		{
			assert(i >= 0 && "Index cannot be negative!");
			assert(i <= 1 && "Index cannot be greater than 1!");
			return *(&this->x + i);
		}
		const T operator[](const size_t i) const
		{
			assert(i >= 0 && "Index cannot be negative!");
			assert(i <= 1 && "Index cannot be greater than 1!");
			return *(&this->x + i);
		}
		T operator%(const Generic_Vec2& v1) const
		{
			return Cross(*this, v1);
		}
	public:
		template<typename S>
		explicit Generic_Vec2(const Generic_Vec2<S>& other)
			:
			x((T)other.x),
			y((T)other.y)
		{}
	public:
		static T Dot(const Generic_Vec2& v0, const Generic_Vec2& v1)
		{
			return v0.x * v1.x + v0.y * v1.y;
		}
		static T Cross(const Generic_Vec2& v0, const Generic_Vec2& v1)
		{
			return v0.x * v1.y - v1.x * v0.y;
		}
	public:
		T x;
		T y;
	};

	template<typename T>
	Generic_Vec2<T> operator*(const T lhs, const Generic_Vec2<T>& rhs)
	{
		return rhs * lhs;
	}

	typedef Generic_Vec2<double> Ved2;
	typedef Generic_Vec2<float>  Vec2;
	typedef Generic_Vec2<int>    Vei2;

	template<typename T>
	class Generic_Vec3 : public Generic_Vec2<T>
	{
	public:
		Generic_Vec3() = default;
		Generic_Vec3(T x, T y, T z)
			:
			Generic_Vec2<T>(x, y),
			z(z)
		{}
		Generic_Vec3(const Generic_Vec2<T>& vec2, T z)
			:
			Generic_Vec2<T>(vec2),
			z(z)
		{}
		template<typename Other>
		Generic_Vec3& operator=(const Other& src)
		{
			this->x = src.x;
			this->y = src.y;
			this->z = src.z;
			return *this;
		}
	public:
		Generic_Vec3& operator+=(const Generic_Vec3& rhs)
		{
			this->x += rhs.x;
			this->y += rhs.y;
			this->z += rhs.z;
			return *this;
		}
		Generic_Vec3& operator-=(const Generic_Vec3& rhs)
		{
			this->x -= rhs.x;
			this->y -= rhs.y;
			this->z -= rhs.z;
			return *this;
		}
		Generic_Vec3& operator*=(const T rhs)
		{
			this->x *= rhs;
			this->y *= rhs;
			this->z *= rhs;
			return *this;
		}
		Generic_Vec3& operator/=(const T rhs)
		{
			this->x /= rhs;
			this->y /= rhs;
			this->z /= rhs;
			return *this;
		}
	public:
		Generic_Vec3 operator+(const Generic_Vec3& rhs) const
		{
			return Generic_Vec3(*this) += rhs;
		}
		Generic_Vec3 operator-(const Generic_Vec3& rhs) const
		{
			return Generic_Vec3(*this) -= rhs;
		}
		Generic_Vec3 operator*(const T rhs) const
		{
			return Generic_Vec3(*this) *= rhs;
		}
		Generic_Vec3 operator/(const T rhs) const
		{
			return Generic_Vec3(*this) /= rhs;
		}
	public:
		bool operator==(const Generic_Vec3& rhs) const
		{
			return this->x == rhs.x && this->y == rhs.y & this->z == rhs.z;
		}
		bool operator!=(const Generic_Vec3& rhs) const
		{
			return !(*this == rhs);
		}
		Generic_Vec3 operator-() const
		{
			return Generic_Vec3(-this->x, -this->y, -this->z);
		}
		T& operator[](const size_t i)
		{
			assert(i >= 0 && "Index cannot be negative!");
			assert(i <= 2 && "Index cannot be greater than 2!");
			return *(&this->x + i);
		}
		const T operator[](const size_t i) const
		{
			assert(i >= 0 && "Index cannot be negative!");
			assert(i <= 2 && "Index cannot be greater than 2!");
			return *(&this->x + i);
		}
	public:
		static T Dot(const Generic_Vec3& v0, const Generic_Vec3& v1)
		{
			return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
		}
		static Generic_Vec3 Cross(const Generic_Vec3& v0, const Generic_Vec3& v1)
		{
			return Generic_Vec3(
				v0.y * v1.z - v0.z * v1.y,
				v0.z * v1.x - v0.x * v1.z,
				v0.x * v1.y - v0.y * v1.x
			);
		}
		static Generic_Vec3 Reflect(const Generic_Vec3& v, const Generic_Vec3& normal)
		{
			return v - ((normal * Dot(v, normal)) * 2.0f);
		}
		T GetLengthSq() const
		{
			return Dot(*this, *this);
		}
		T GetLength() const
		{
			return (T)std::sqrt(GetLengthSq());
		}
		Generic_Vec3& Normalize()
		{
			return (*this) /= GetLength();
		}
		Generic_Vec3 GetNormalized() const
		{
			return Generic_Vec3(*this).Normalize();
		}
		// Saturate the components above 1
		Generic_Vec3& Saturate()
		{
			this->x = std::min(this->x, (T)1.0);
			this->y = std::min(this->y, (T)1.0);
			this->z = std::min(this->z, (T)1.0);
			return *this;
		}
		// Saturated components above 1
		Generic_Vec3 GetSaturated() const
		{
			return Generic_Vec3(*this).Saturate();
		}
		// Saturate the components above 255
		Generic_Vec3& Saturate255()
		{
			this->x = std::min(this->x, (T)255.0);
			this->y = std::min(this->y, (T)255.0);
			this->z = std::min(this->z, (T)255.0);
			return *this;
		}
		// Saturated components above 255
		Generic_Vec3 GetSaturated255() const
		{
			return Generic_Vec3(*this).Saturate255();
		}
		// Get the Hadamard product
		Generic_Vec3& Hadamard(const Generic_Vec3& rhs)
		{
			this->x *= rhs.x;
			this->y *= rhs.y;
			this->z *= rhs.z;
			return *this;
		}
		Generic_Vec3& GetHadamard(const Generic_Vec3& rhs) const
		{
			return Generic_Vec3(*this).Hadamard(rhs);
		}
	public:
		template<typename S>
		explicit Generic_Vec3(const Generic_Vec3<S>& other)
			:
			Generic_Vec3((T)other.x, (T)other.y),
			z((T)other.z)
		{}
	public:
		T z;
	};

	template<typename T>
	Generic_Vec3<T> operator*(const T lhs, const Generic_Vec3<T>& rhs)
	{
		return rhs * lhs;
	}

	typedef Generic_Vec3<double> Ved3;
	typedef Generic_Vec3<float>  Vec3;
	typedef Generic_Vec3<int>    Vei3;

	template<typename T>
	class Generic_Vec4 : public Generic_Vec3<T>
	{
	public:
		Generic_Vec4() = default;
		Generic_Vec4(T x, T y, T z, T w)
			:
			Generic_Vec3<T>(x, y, z),
			w(w)
		{}
		Generic_Vec4(const Generic_Vec2<T>& vec2, T z, T w)
			:
			Generic_Vec3<T>(vec2, z),
			w(w)
		{}
		Generic_Vec4(const Generic_Vec3<T>& vec3, T w = (T)1.0)
			:
			Generic_Vec3<T>(vec3),
			w(w)
		{}
		template<typename Other>
		Generic_Vec4& operator=(const Other& src)
		{
			this->x = src.x;
			this->y = src.y;
			this->z = src.z;
			this->w = src.w;
			return *this;
		}
	public:
		Generic_Vec4& operator+=(const Generic_Vec4& rhs)
		{
			this->x += rhs.x;
			this->y += rhs.y;
			this->z += rhs.z;
			this->w += rhs.w;
			return *this;
		}
		Generic_Vec4& operator-=(const Generic_Vec4& rhs)
		{
			this->x -= rhs.x;
			this->y -= rhs.y;
			this->z -= rhs.z;
			this->w -= rhs.w;
			return *this;
		}
		Generic_Vec4& operator*=(const T rhs)
		{
			this->x *= rhs;
			this->y *= rhs;
			this->z *= rhs;
			this->w *= rhs;
			return *this;
		}
		Generic_Vec4& operator/=(const T rhs)
		{
			this->x /= rhs;
			this->y /= rhs;
			this->z /= rhs;
			this->w /= rhs;
			return *this;
		}
	public:
		Generic_Vec4 operator+(const Generic_Vec4& rhs) const
		{
			return Generic_Vec4(*this) += rhs;
		}
		Generic_Vec4 operator-(const Generic_Vec4& rhs) const
		{
			return Generic_Vec4(*this) -= rhs;
		}
		Generic_Vec4 operator*(const T rhs) const
		{
			return Generic_Vec4(*this) *= rhs;
		}
		Generic_Vec4 operator/(const T rhs) const
		{
			return Generic_Vec4(*this) /= rhs;
		}
	public:
		Generic_Vec4 operator-() const
		{
			return Generic_Vec4(-this->x, -this->y, -this->z, -this->w);
		}
		T& operator[](const size_t i)
		{
			assert(i >= 0 && "Index cannot be negative!");
			assert(i <= 3 && "Index cannot be greater than 3");
			return *(&this->x + i);
		}
		const T operator[](const size_t i) const
		{
			assert(i >= 0 && "Index cannot be negative!");
			assert(i <= 3 && "Index cannot be greater than 3");
			return *(&this->x + i);
		}
	public:
		static constexpr T Dot(const Generic_Vec4& v0, const Generic_Vec4& v1)
		{
			return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
		}
		T GetLengthSq() const
		{
			return Dot(*this, *this);
		}
		T GetLength() const
		{
			return std::sqrtf(GetLengthSq());
		}
		Generic_Vec4& Normalize()
		{
			return (*this) /= GetLength();
		}
		Generic_Vec4 GetNormalized() const
		{
			return Generic_Vec4(*this).Normalize();
		}
	public:
		template<typename S>
		explicit Generic_Vec4(const Generic_Vec4<S>& other)
			:
			Generic_Vec3((T)other.x, (T)other.y, (T)other.z),
			w((T)other.w)
		{}
	public:
		T w;
	};

	template<typename T>
	Generic_Vec4<T> operator*(const T lhs, const Generic_Vec4<T>& rhs)
	{
		return rhs * lhs;
	}

	typedef Generic_Vec4<double> Ved4;
	typedef Generic_Vec4<float>  Vec4;
	typedef Generic_Vec4<int>    Vei4;

	template<typename T>
	class Generic_Mat2
	{
	public:
		constexpr Generic_Vec2<T> operator*(const Generic_Vec2<T>& v)
		{
			return Mul(*this, v);
		}
		constexpr Generic_Mat2 operator*(const Generic_Mat2& rhs)
		{
			return Mul(*this, rhs);
		}
	public:
		// Multiply 2x2 matrix by 2D vector
		static constexpr Generic_Vec2<T> Mul(const Generic_Mat2& lhs, const Generic_Vec2<T>& rhs)
		{
			return
			{
				lhs.elements[0][0] * rhs[0] + lhs.elements[0][1] * rhs[1],
				lhs.elements[1][0] * rhs[0] + lhs.elements[1][1] * rhs[1]
			};
		}
		// Multiply 2x2 matrices
		static constexpr Generic_Mat2 Mul(const Generic_Mat2& lhs, const Generic_Mat2& rhs)
		{
			Generic_Mat2<T> res;
			for (unsigned int j = 0; j < 2; j++)
			{
				for (unsigned int i = 0; i < 2; i++)
				{
					T sum = (T)0.0;
					for (unsigned int k = 0; k < 2; k++)
					{
						sum += lhs.elements[i][k] * rhs.elements[k][j];
					}
					res.elements[i][j] = sum;
				}
			}
			return res;
		}
	public:
		// The 2D identity Matrix
		static constexpr Generic_Mat2 Identity()
		{
			return
			{
				(T)1.0, (T)0.0,
				(T)0.0, (T)1.0
			};
		}
		// The standard 2D rotation Matrix
		static constexpr Generic_Mat2 Rotation(const T angle)
		{
			const T cost = (T)std::cos(angle);
			const T sint = (T)std::sin(angle);
			return
			{
				cost, -sint,
				sint,  cost
			};
		}
		// Returns the scaling Matrix
		static constexpr Generic_Mat2 Scaling(const T factor)
		{
			return
			{
				factor, (T)0.0,
				(T)0.0, factor
			};
		}
	public:
		T elements[2][2];
	};

	typedef Generic_Mat2<double> Mad2;
	typedef Generic_Mat2<float>  Mat2;
	typedef Generic_Mat2<int>    Mai2;


	template<typename T>
	class Generic_Mat3
	{
	public:
		constexpr Generic_Vec3<T> operator * (const Generic_Vec3<T>& v)
		{
			return Mul(*this, v);
		}
		constexpr Generic_Mat3 operator * (const Generic_Mat3& rhs)
		{
			return Mul(*this, rhs);
		}
	public:
		static constexpr Generic_Vec3<T> Mul(const Generic_Mat3& A, const Generic_Vec3<T>& v)
		{
			return
			{
				A.elements[0][0] * v[0] + A.elements[0][1] * v[1] + A.elements[0][2] * v[2],
				A.elements[1][0] * v[0] + A.elements[1][1] * v[1] + A.elements[1][2] * v[2],
				A.elements[2][0] * v[0] + A.elements[2][1] * v[1] + A.elements[2][2] * v[2]
			};
		}
		static constexpr Generic_Mat3 Mul(const Generic_Mat3& A, const Generic_Mat3& B)
		{
			Generic_Mat3<T> res;
			for (unsigned int j = 0; j < 3; j++)
			{
				for (unsigned int i = 0; i < 3; i++)
				{
					T sum = (T)0.0;
					for (unsigned int k = 0; k < 3; k++)
					{
						sum += A.elements[i][k] * B.elements[k][j];
					}
					res.elements[i][j] = sum;
				}
			}
			return res;
		}
	public:
		static constexpr Generic_Mat3 Identity()
		{
			return
			{
				(T)1.0, (T)0.0, (T)0.0,
				(T)0.0, (T)1.0, (T)0.0,
				(T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat3 Scaling(const T factor)
		{
			return
			{
				factor, (T)0.0, (T)0.0,
				(T)0.0, factor, (T)0.0,
				(T)0.0, (T)0.0, factor
			};
		}
		static constexpr Generic_Mat3 RotationZ(const T angle)
		{
			const T cost = (T)std::cos(angle);
			const T sint = (T)std::sin(angle);
			return
			{
				  cost,  -sint, (T)0.0,
				  sint,   cost, (T)0.0,
				(T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat3 RotationX(const T angle)
		{
			const T cost = (T)std::cos(angle);
			const T sint = (T)std::sin(angle);
			return
			{
				(T)1.0, (T)0.0, (T)0.0,
				(T)0.0,   cost,  -sint,
				(T)0.0,   sint,   cost
			};
		}
		static constexpr Generic_Mat3 RotationY(const T angle)
		{
			const T cost = (T)std::cos(angle);
			const T sint = (T)std::sin(angle);
			return
			{
				cost  , (T)0.0,  -sint,
				(T)0.0, (T)1.0, (T)0.0,
				sint  , (T)0.0,   cost
			};
		}
	public:
		T elements[3][3];
	};

	typedef Generic_Mat3<double> Mad3;
	typedef Generic_Mat3<float>  Mat3;
	typedef Generic_Mat3<int>    Mai3;

	template<typename T>
	class Generic_Mat4
	{
	public:
		constexpr Generic_Vec4<T> operator*(const Generic_Vec4<T>& v)
		{
			return Mul(*this, v);
		}
		constexpr Generic_Mat4 operator*(const Generic_Mat4& rhs)
		{
			return Mul(*this, rhs);
		}
	public:
		static constexpr Generic_Vec4<T> Mul(const Generic_Mat4& A, const Generic_Vec4<T>& v)
		{
			return
			{
				A.elements[0][0] * v[0] + A.elements[0][1] * v[1] + A.elements[0][2] * v[2] + A.elements[0][3] * v[3],
				A.elements[1][0] * v[0] + A.elements[1][1] * v[1] + A.elements[1][2] * v[2] + A.elements[1][3] * v[3],
				A.elements[2][0] * v[0] + A.elements[2][1] * v[1] + A.elements[2][2] * v[2] + A.elements[2][3] * v[3],
				A.elements[3][0] * v[0] + A.elements[3][1] * v[1] + A.elements[3][2] * v[2] + A.elements[3][3] * v[3]
			};
		}
		static constexpr Generic_Mat4 Mul(const Generic_Mat4& lhs, const Generic_Mat4& rhs)
		{
			Generic_Mat4<T> res;
			for (unsigned int j = 0; j < 4; j++)
			{
				for (unsigned int i = 0; i < 4; i++)
				{
					T sum = (T)0.0;
					for (unsigned int k = 0; k < 4; k++)
					{
						sum += lhs.elements[i][k] * rhs.elements[k][j];
					}
					res.elements[i][j] = sum;
				}
			}
			return res;
		}
	public:
		Generic_Mat4 GetTransposed() const
		{
			Generic_Mat4 res;

			for (unsigned int j = 0; j < 4; j++)
			{
				for (unsigned int i = 0; i < 4; i++)
				{
					res.elements[i][j] = elements[j][i];
				}
			}

			return res;
		}
	public:
		static constexpr Generic_Mat4 Identity()
		{
			return
			{
				(T)1.0, (T)0.0, (T)0.0, (T)0.0,
				(T)0.0, (T)1.0, (T)0.0, (T)0.0,
				(T)0.0, (T)0.0, (T)1.0, (T)0.0,
				(T)0.0, (T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat4 Scaling(const T fx, const T fy, const T fz)
		{
			return 
			{
				fx    , (T)0.0, (T)0.0, (T)0.0,
				(T)0.0, fy    , (T)0.0, (T)0.0,
				(T)0.0, (T)0.0, fz    , (T)0.0,
				(T)0.0, (T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat4 Scaling(const T factor)
		{
			return
			{
				factor, (T)0.0, (T)0.0, (T)0.0,
				(T)0.0, factor, (T)0.0, (T)0.0,
				(T)0.0, (T)0.0, factor, (T)0.0,
				(T)0.0, (T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat4 RotationZ(const T angle)
		{
			const T cost = (T)std::cos(angle);
			const T sint = (T)std::sin(angle);
			return
			{
				  cost,  -sint, (T)0.0, (T)0.0,
				  sint,   cost, (T)0.0, (T)0.0,
				(T)0.0, (T)0.0, (T)1.0, (T)0.0,
				(T)0.0, (T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat4 RotationX(const T angle)
		{
			const T cost = (T)std::cos(angle);
			const T sint = (T)std::sin(angle);
			return
			{
				(T)1.0, (T)0.0, (T)0.0, (T)0.0,
				(T)0.0,   cost,  -sint, (T)0.0,
				(T)0.0,   sint,   cost, (T)0.0,
				(T)0.0, (T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat4 RotationY(const T angle)
		{
			const T cost = (T)std::cos(angle);
			const T sint = (T)std::sin(angle);
			return
			{
				cost  , (T)0.0,  -sint, (T)0.0,
				(T)0.0, (T)1.0, (T)0.0, (T)0.0,
				sint  , (T)0.0,   cost, (T)0.0,
				(T)0.0, (T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat4 Translation(const T dx, const T dy, const T dz)
		{
			return
			{
				(T)1.0, (T)0.0, (T)0.0, (T)dx,
				(T)0.0, (T)1.0, (T)0.0, (T)dy,
				(T)0.0, (T)0.0, (T)1.0, (T)dz,
				(T)0.0, (T)0.0, (T)0.0, (T)1.0
			};
		}
		static constexpr Generic_Mat4 Translation(const Generic_Vec3<T>& delta)
		{
			return Translation(delta.x, delta.y, delta.z);
		}
		// When applied to a Vec4 it does the projection, it maps the z from [near_z, far_r] to [0, 1] and it puts in w the perspective division factor
		static constexpr Generic_Mat4 PerspectiveRH(const T proj_w, const T proj_h, const T near_z, const T far_z)
		{
			assert(far_z > near_z);
			assert(near_z > 0.0f);
			const T a11 = (T)2.0 * near_z / proj_w;
			const T a22 = (T)2.0 * near_z / proj_h;
			const T a33 =  far_z / (far_z - near_z);
			const T a43 = -far_z * near_z / (far_z - near_z);
			return
			{
				a11   , (T)0.0, (T)0.0, (T)0.0,
				(T)0.0,    a22, (T)0.0, (T)0.0,
				(T)0.0, (T)0.0,    a33,    a43,
				(T)0.0, (T)0.0, (T)1.0, (T)0.0
			};
		}
		// When applied to a Vec4 it does the projection, it maps the z from [near_z, far_r] to [0, 1] and it puts in w the perspective division factor
		static constexpr Generic_Mat4 PerspectiveHFOV(const T fovDeg = (T)90.0, const T near_z = (T)0.1, const T far_z = (T)200.0, const T ar = (T)(16.0 / 9.0))
		{
			const T fov = fovDeg * (T)PI_D / (T)180.0;
			const T a11 = (T)1.0f / std::tan(fov / (T)2.0);
			const T a22 =      ar / std::tan(fov / (T)2.0);
			const T a33 = far_z / (far_z - near_z);
			const T a43 = -far_z * near_z / (far_z - near_z);
			return {
				   a11,   (T)0.0, (T)0.0, (T)0.0,
				(T)0.0,	     a22, (T)0.0, (T)0.0,
				(T)0.0,	  (T)0.0,    a33,    a43,
				(T)0.0,	  (T)0.0, (T)1.0, (T)0.0
			};
		}
public:
		T elements[4][4];
	};

	typedef Generic_Mat4<double> Mad4;
	typedef Generic_Mat4<float>  Mat4;
	typedef Generic_Mat4<int>    Mai4;

	namespace Geometry
	{
		class Cube
		{
		public:
			template<typename Vertex>
			static IndexedTriangleList<Vertex> Make()
			{
				IndexedTriangleList<Vertex> cube;
				cube.vertices.resize(8u);

				static constexpr float size = 0.5f;

				cube.vertices[0].pos = { size, size, size };
				cube.vertices[1].pos = { size, size,-size };
				cube.vertices[2].pos = { size,-size, size };
				cube.vertices[3].pos = { size,-size,-size };
				cube.vertices[4].pos = { -size, size, size };
				cube.vertices[5].pos = { -size, size,-size };
				cube.vertices[6].pos = { -size,-size, size };
				cube.vertices[7].pos = { -size,-size,-size };

				cube.indices = {
					0,2,1,
					1,2,3,
					5,1,3,
					5,3,7,
					4,5,6,
					6,5,7,
					0,4,6,
					0,6,2,
					7,3,6,
					6,3,2,
					5,4,1,
					1,4,0
				};

				return cube;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTex()
			{
				IndexedTriangleList<Vertex> cube;
				cube = MakeIndependent<Vertex>();

				const DirectX::XMFLOAT2 tex[] =
				{
					{ 0.0f,0.0f },
					{ 0.0f,1.0f },
					{ 1.0f,0.0f },
					{ 1.0f,1.0f }
				};

				cube.vertices[0].tex = tex[0];
				cube.vertices[1].tex = tex[1];
				cube.vertices[2].tex = tex[2];
				cube.vertices[3].tex = tex[3];
				cube.vertices[4].tex = tex[0];
				cube.vertices[5].tex = tex[1];
				cube.vertices[6].tex = tex[2];
				cube.vertices[7].tex = tex[3];
				cube.vertices[8].tex = tex[0];
				cube.vertices[9].tex = tex[1];
				cube.vertices[10].tex = tex[2];
				cube.vertices[11].tex = tex[3];
				cube.vertices[12].tex = tex[0];
				cube.vertices[13].tex = tex[1];
				cube.vertices[14].tex = tex[2];
				cube.vertices[15].tex = tex[3];
				cube.vertices[16].tex = tex[0];
				cube.vertices[17].tex = tex[1];
				cube.vertices[18].tex = tex[2];
				cube.vertices[19].tex = tex[3];
				cube.vertices[20].tex = tex[0];
				cube.vertices[21].tex = tex[1];
				cube.vertices[22].tex = tex[2];
				cube.vertices[23].tex = tex[3];

				return cube;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeNor()
			{
				IndexedTriangleList<Vertex> cube;
				cube = MakeIndependent<Vertex>();

				const DirectX::XMFLOAT3 n[] = {
					{ 0.0f, 0.0f,-1.0f },
					{ 1.0f, 0.0f, 0.0f },
					{-1.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f, 1.0f },
					{ 0.0f,-1.0f, 0.0f },
					{ 0.0f, 1.0f, 0.0f },
				};

				cube.vertices[0].n = n[0];
				cube.vertices[1].n = n[0];
				cube.vertices[2].n = n[0];
				cube.vertices[3].n = n[0];
				cube.vertices[4].n = n[1];
				cube.vertices[5].n = n[1];
				cube.vertices[6].n = n[1];
				cube.vertices[7].n = n[1];
				cube.vertices[8].n = n[2];
				cube.vertices[9].n = n[2];
				cube.vertices[10].n = n[2];
				cube.vertices[11].n = n[2];
				cube.vertices[12].n = n[3];
				cube.vertices[13].n = n[3];
				cube.vertices[14].n = n[3];
				cube.vertices[15].n = n[3];
				cube.vertices[16].n = n[4];
				cube.vertices[17].n = n[4];
				cube.vertices[18].n = n[4];
				cube.vertices[19].n = n[4];
				cube.vertices[20].n = n[5];
				cube.vertices[21].n = n[5];
				cube.vertices[22].n = n[5];
				cube.vertices[23].n = n[5];

				return cube;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTexNor()
			{
				IndexedTriangleList<Vertex> cube;
				cube = MakeTex<Vertex>();

				const DirectX::XMFLOAT3 n[] = {
					{ 0.0f, 0.0f,-1.0f },
					{ 1.0f, 0.0f, 0.0f },
					{-1.0f, 0.0f, 0.0f },
					{ 0.0f, 0.0f, 1.0f },
					{ 0.0f,-1.0f, 0.0f },
					{ 0.0f, 1.0f, 0.0f },
				};

				cube.vertices[0].n = n[0];
				cube.vertices[1].n = n[0];
				cube.vertices[2].n = n[0];
				cube.vertices[3].n = n[0];
				cube.vertices[4].n = n[1];
				cube.vertices[5].n = n[1];
				cube.vertices[6].n = n[1];
				cube.vertices[7].n = n[1];
				cube.vertices[8].n = n[2];
				cube.vertices[9].n = n[2];
				cube.vertices[10].n = n[2];
				cube.vertices[11].n = n[2];
				cube.vertices[12].n = n[3];
				cube.vertices[13].n = n[3];
				cube.vertices[14].n = n[3];
				cube.vertices[15].n = n[3];
				cube.vertices[16].n = n[4];
				cube.vertices[17].n = n[4];
				cube.vertices[18].n = n[4];
				cube.vertices[19].n = n[4];
				cube.vertices[20].n = n[5];
				cube.vertices[21].n = n[5];
				cube.vertices[22].n = n[5];
				cube.vertices[23].n = n[5];

				return cube;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTexNorTang()
			{
				auto cube = MakeTexNor<Vertex>();

				const DirectX::XMFLOAT3 tangent[] = {
					{ 1.0f, 0.0f, 0.0f }, // Front
					{ 0.0f, 0.0f, 1.0f }, // Right
					{ 0.0f, 0.0f,-1.0f }, // Left
					{-1.0f, 0.0f, 0.0f }, // Back
					{ 1.0f, 0.0f, 0.0f }, // Bottom
					{-1.0f, 0.0f, 0.0f }, // Top
				};

				const DirectX::XMFLOAT3 bitangent[] = {
					{ 0.0f,-1.0f, 0.0f }, // Front
					{ 0.0f,-1.0f, 0.0f }, // Right
					{ 0.0f,-1.0f, 0.0f }, // Left
					{ 0.0f,-1.0f, 0.0f }, // Back
					{ 0.0f, 0.0f, 1.0f }, // Bottom
					{ 0.0f, 0.0f, 1.0f }, // Top
				};

				for (unsigned int i = 0u; i < 24u; i++)
				{
					cube.vertices[i].tangent   = tangent[i / 4];
					cube.vertices[i].bitangent = bitangent[i / 4];
				}

				return cube;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeIndependent()
			{
				IndexedTriangleList<Vertex> cube;

				cube.vertices.resize(24);

				static constexpr float size = 0.5f;
				const DirectX::XMFLOAT3 pos[] = {
					{ size, size, size },
					{ size, size,-size },
					{ size,-size, size },
					{ size,-size,-size },
					{-size, size, size },
					{-size, size,-size },
					{-size,-size, size },
					{-size,-size,-size }
				};

				cube.vertices[0].pos = pos[7];
				cube.vertices[1].pos = pos[5];
				cube.vertices[2].pos = pos[3];
				cube.vertices[3].pos = pos[1];
				cube.vertices[4].pos = pos[3];
				cube.vertices[5].pos = pos[1];
				cube.vertices[6].pos = pos[2];
				cube.vertices[7].pos = pos[0];
				cube.vertices[8].pos = pos[6];
				cube.vertices[9].pos = pos[4];
				cube.vertices[10].pos = pos[7];
				cube.vertices[11].pos = pos[5];
				cube.vertices[12].pos = pos[2];
				cube.vertices[13].pos = pos[0];
				cube.vertices[14].pos = pos[6];
				cube.vertices[15].pos = pos[4];
				cube.vertices[16].pos = pos[6];
				cube.vertices[17].pos = pos[7];
				cube.vertices[18].pos = pos[2];
				cube.vertices[19].pos = pos[3];
				cube.vertices[20].pos = pos[0];
				cube.vertices[21].pos = pos[1];
				cube.vertices[22].pos = pos[4];
				cube.vertices[23].pos = pos[5];

				// Indices creation
				cube.indices =
				{
					 0, 1, 2,
					 1, 3, 2,
					 4, 5, 6,
					 5, 7, 6,
					 8, 9,10,
					 9,11,10,
					12,13,14,
					13,15,14,
					16,17,18,
					17,19,18,
					20,21,22,
					21,23,22
				};

				return cube;
			}
		};

		class Grid
		{
		public:
			template<typename Vertex>
			static IndexedTriangleList<Vertex> Make(const index_type width, const index_type height)
			{
				IndexedTriangleList<Vertex> grid;

				const unsigned int nVerts = (width + 1u) * (height + 1u);
				const unsigned long int nIndices = 6u * width * height;
				assert(nIndices < 65536u && "There are too many indices for index_type to contain.");

				grid.vertices.resize(nVerts);

				// generate vertices
				index_type ix = 0u;
				for (index_type j = 0; j <= height; j++)
				{
					for (index_type i = 0; i <= width; i++, ix++)
					{
						grid.vertices[ix].pos = { (float)i, (float)j, 0.0f };
					}
				}

				// utility
				auto index = [&](const index_type i, const index_type j)
				{
					return i + (width + 1) * j;
				};

				// generate indices
				for (index_type j = 0; j < height; j++)
				{
					for (index_type i = 0; i < width; i++)
					{
						// Triangle1
						grid.indices.push_back(index(i + 0u, j + 0u));
						grid.indices.push_back(index(i + 0u, j + 1u));
						grid.indices.push_back(index(i + 1u, j + 0u));

						// Triangle2
						grid.indices.push_back(index(i + 1u, j + 0u));
						grid.indices.push_back(index(i + 0u, j + 1u));
						grid.indices.push_back(index(i + 1u, j + 1u));
					}
				}

				// Offset to the barycenter
				for (auto& v : grid.vertices)
				{
					v.pos.x -= width / 2.0f;
					v.pos.y -= height / 2.0f;
				}

				return grid;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTex(const index_type width, const index_type height)
			{
				IndexedTriangleList<Vertex> grid;

				const index_type nVerts = 4u * width * height;
				grid.vertices.resize(nVerts);

				std::vector<DirectX::XMFLOAT3> verts;
				for (index_type j = 0; j <= height; j++)
				{
					for (index_type i = 0; i <= width; i++)
					{
						verts.push_back({ (float)i, (float)j, 0.0f });
					}
				}

				// utility
				auto index = [&](const index_type i, const index_type j)
				{
					return i + (width + 1) * j;
				};

				for (index_type j = 0; j < height; j++)
				{
					for (index_type i = 0; i < width; i++)
					{
						const size_t vCount = 4u * ((size_t)width * j + i);

						grid.vertices[vCount + 0u].pos = verts[index(i + 0u, j + 0u)];
						grid.vertices[vCount + 1u].pos = verts[index(i + 0u, j + 1u)];
						grid.vertices[vCount + 2u].pos = verts[index(i + 1u, j + 0u)];
						grid.vertices[vCount + 3u].pos = verts[index(i + 1u, j + 1u)];

						grid.vertices[vCount + 0u].tex = { 0.0f,0.0f };
						grid.vertices[vCount + 1u].tex = { 0.0f,1.0f };
						grid.vertices[vCount + 2u].tex = { 1.0f,0.0f };
						grid.vertices[vCount + 3u].tex = { 1.0f,1.0f };

						grid.indices.push_back((index_type)vCount + 0u);
						grid.indices.push_back((index_type)vCount + 1u);
						grid.indices.push_back((index_type)vCount + 2u);

						grid.indices.push_back((index_type)vCount + 2u);
						grid.indices.push_back((index_type)vCount + 1u);
						grid.indices.push_back((index_type)vCount + 3u);
					}
				}

				// offset to the barycenter
				for (auto& v : grid.vertices)
				{
					v.pos.x -= width / 2.0f;
					v.pos.y -= height / 2.0f;
				}

				return grid;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeNor(const index_type width, const index_type height)
			{
				IndexedTriangleList<Vertex> grid = Make<Vertex>(width, height);

				for (auto& v : grid.vertices)
				{
					v.n = { 0.0f,0.0f,-1.0f };
				}

				return grid;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTexNor(const index_type width, const index_type height)
			{
				IndexedTriangleList<Vertex> grid = MakeTex<Vertex>(width, height);

				for (auto& v : grid.vertices)
				{
					v.n = { 0.0f,0.0f,-1.0f };
				}

				return grid;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTexNorTang(const index_type width, const index_type height)
			{
				IndexedTriangleList<Vertex> grid = MakeTexNor<Vertex>(width, height);

				for (auto& v : grid.vertices)
				{
					v.tangent = { 1.0f,0.0f,0.0f };
					v.bitangent = { 0.0f,1.0f,0.0f };
				}

				return grid;
			}
		};

		class Plane
		{
		public:
			template<typename Vertex>
			static IndexedTriangleList<Vertex> Make(const index_type nTessellations = 1u)
			{
				IndexedTriangleList<Vertex> plane;

				const index_type nVerts = (nTessellations + 1u) * (nTessellations + 1u);
				plane.vertices.resize(nVerts);

				const float step = 1.0f / (float)nTessellations;

				index_type index = 0u;
				for (index_type j = 0; j <= nTessellations; j++)
				{
					for (index_type i = 0; i <= nTessellations; i++, index++)
					{
						plane.vertices[index].pos = { step * (float)i, step * (float)j, 0.0f };
						if (i < nTessellations && j < nTessellations)
						{
							plane.indices.push_back(j * (nTessellations + 1u) + i);
							plane.indices.push_back(j * (nTessellations + 1u) + i + 1u);
							plane.indices.push_back(j * (nTessellations + 1u) + i + 2u + nTessellations);
							plane.indices.push_back(j * (nTessellations + 1u) + i);
							plane.indices.push_back(j * (nTessellations + 1u) + i + 2u + nTessellations);
							plane.indices.push_back(j * (nTessellations + 1u) + i + 1u + nTessellations);
						}
					}
				}

				for (auto& v : plane.vertices)
				{
					v.pos.x -= 0.5f;
					v.pos.y -= 0.5f;
					v.pos.x = -v.pos.x;
				}

				return plane;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTex(const index_type nTessellations = 1u)
			{
				IndexedTriangleList<Vertex> plane = Make<Vertex>(nTessellations);

				const float step = 1.0f / (float)nTessellations;

				index_type index = 0u;
				for (index_type j = 0; j <= nTessellations; j++)
				{
					for (index_type i = 0; i <= nTessellations; i++, index++)
					{
						plane.vertices[index].tex = { 1.0f - step * (float)i, step * (float)j };
					}
				}

				return plane;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeNor(const index_type nTessellations = 1u)
			{
				IndexedTriangleList<Vertex> plane = Make<Vertex>(nTessellations);

				for (auto& v : plane.vertices)
				{
					v.n = { 0.0f,0.0f,-1.0f };
				}

				return plane;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTexNor(const index_type nTessellations = 1u)
			{
				IndexedTriangleList<Vertex> plane = MakeTex<Vertex>(nTessellations);

				for (auto& v : plane.vertices)
				{
					v.n = { 0.0f,0.0f,-1.0f };
				}

				return plane;
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeTexNorTang(const index_type nTessellations = 1u)
			{
				IndexedTriangleList<Vertex> plane = MakeTexNor<Vertex>(nTessellations);

				for (auto& v : plane.vertices)
				{
					v.tangent   = { 1.0f,0.0f,0.0f };
					v.bitangent = { 0.0f,1.0f,0.0f };
				}

				return plane;
			}
		};

		class Triangle
		{
		public:
			template<typename Vertex>
			static IndexedTriangleList<Vertex> Make()
			{
				IndexedTriangleList<Vertex> triangle;

				triangle.vertices.resize(3u);

				float phi = 0.0f;
				float dPhi = twoPI / 3.0f;

				for (unsigned int i = 0u; i < 3u; i++)
				{
					triangle.vertices[i].pos = { std::cos(phi),std::sin(-phi),0.0f };
					phi += dPhi;
				}

				triangle.indices = { 0u,1u,2u };

				return triangle;
			}
		
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeNor()
			{
				IndexedTriangleList<Vertex> triangle = Make<Vertex>();

				

				for (auto& v : triangle.vertices)
				{
					v.n = { 0.0f,0.0f,-1.0f };
				}

				return triangle;
			}

			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeNor2(const float dz = 0.0f)
			{
				IndexedTriangleList<Vertex> tri1 = Make<Vertex>();
				IndexedTriangleList<Vertex> tri2 = Make<Vertex>();

				std::swap(tri2.vertices[0], tri2.vertices[1]);

				IndexedTriangleList<Vertex> doubleTriangle;

				for (auto& v : tri1.vertices)
				{
					v.pos.z -= dz;
					v.n = { 0.0f,0.0f,-1.0f };
					doubleTriangle.vertices.push_back(v);
				}
				for (auto& v : tri2.vertices)
				{
					v.pos.z += dz;
					v.n = { 0.0f,0.0f,1.0f };
					doubleTriangle.vertices.push_back(v);
				}

				doubleTriangle.indices = { 0u,1u,2u, 3u,4u,5u };

				return doubleTriangle;
			}
		};

		class Sphere
		{
		public:
			template<typename Vertex>
			static IndexedTriangleList<Vertex> Make(const index_type nLatSubd = 18u, const index_type nLonSubd = 36u)
			{
				assert(nLatSubd >= 4u);
				assert(nLonSubd >= 3u);
				assert("Too many tessellations for index_type indices" && (6ul * (unsigned long)nLonSubd * ((unsigned long)nLatSubd - 1ul) < 65536ul));

				// calculate the number of vertices and the number of indices
				const index_type nVerts = 2u + nLonSubd * (nLatSubd - 1u);
				const index_type nIndices = 6u * nLonSubd * (nLatSubd - 1u);

				// generate a vector of vertices and a vector of indices with the proper size
				std::vector<Vertex> vertices(nVerts);
				std::vector<index_type> indices(nIndices);

				// Generation of a Sphere with radius 1.0f. 
				// Polar coordinates: (phi, theta) --> (latitude, longitude)
				// phi goes from 0 to PI	
				// theta goes from 0 to 2PI

				// utility lambda
				auto fromPolar = [](const float phi, const float theta)
				{
					const float x = std::sinf(phi) * std::cosf(theta);
					const float y = std::sinf(phi) * std::sinf(theta);
					const float z = std::cosf(phi);
					return DirectX::XMFLOAT3(x, y, z);
				};

				// The angle steps for the choosen subdivisions
				const float   phiStep = PI / (float)nLatSubd;
				const float thetaStep = 2.0f * PI / (float)nLonSubd;

				// Generation of the vertices excluding the North and South poles
				for (index_type iLat = 1u, vCount = 0u; iLat < nLatSubd; iLat++)
				{
					for (index_type iLon = 0u; iLon < nLonSubd; iLon++, vCount++)
					{
						const float theta = (float)iLon * thetaStep;
						const float phi = (float)iLat * phiStep;
						vertices[vCount].pos = fromPolar(phi, theta);
					}
				}
				// Adding the North and South poles
				const index_type northIndex = nVerts - 2u;
				const index_type southIndex = nVerts - 1u;
				vertices[northIndex].pos = { 0.0f, 0.0f, 1.0f };  // North
				vertices[southIndex].pos = { 0.0f, 0.0f,-1.0f };  // South

				// Now we have every vertex in the sphere based on AC (Adrian Convention).
				// Now let's triangulate all the faces with indices from North pole to South pole.

				// Making of the first strate. You figure it out ;)
				for (index_type i = 0u; i < nLonSubd - 1u; i++)
				{
					indices.push_back(northIndex);
					indices.push_back(i);
					indices.push_back(i + 1u);
				}
				indices.push_back(northIndex);
				indices.push_back(nLonSubd - 1u);
				indices.push_back(0u);

				// Making the body part. You figure it out ;)
				for (index_type j = 0u; j < nLatSubd - 2u; j++)
				{
					for (index_type i = 0u; i < nLonSubd - 1u; i++)
					{
						const index_type iStart = i + j * nLonSubd;
						indices.push_back(iStart);
						indices.push_back(iStart + nLonSubd);
						indices.push_back(iStart + 1u);
						indices.push_back(iStart + 1u);
						indices.push_back(iStart + nLonSubd);
						indices.push_back(iStart + nLonSubd + 1u);
					}
					const index_type iShift = j * nLonSubd;
					indices.push_back(iShift + nLonSubd - 1u);
					indices.push_back(iShift + 2u * nLonSubd - 1u);
					indices.push_back(iShift);
					indices.push_back(iShift);
					indices.push_back(iShift + 2u * nLonSubd - 1u);
					indices.push_back(iShift + nLonSubd);
				}

				// Making the last strate. You figure it out ;)
				const index_type i0 = southIndex - nLonSubd - 1u;
				for (index_type i = 0u; i < nLonSubd - 1u; i++)
				{
					indices.push_back(southIndex);
					indices.push_back(i + i0 + 1u);
					indices.push_back(i + i0);
				}
				indices.push_back(southIndex);
				indices.push_back(i0);
				indices.push_back(i0 + nLonSubd - 1u);

				// return an indexed triangle list with the calculated vertices and indices
				return { std::move(vertices), std::move(indices) };
			}
			template<typename Vertex>
			static IndexedTriangleList<Vertex> MakeNor(const index_type nLatSubd = 18u, const index_type nLonSubd = 36u)
			{
				auto sphere = Make<Vertex>(nLatSubd, nLonSubd);

				for (auto& v : sphere.vertices)
				{
					v.n = v.pos;
				}

				return sphere;
			}
		};

		class Room
		{
		public:
			template<typename Vertex>
			static std::vector<IndexedTriangleList<Vertex>> Make(const index_type width = 4u, const index_type height = 3u, const index_type depth = 5u)
			{
				IndexedTriangleList<Vertex> floor   = Grid::Make<Vertex>(width, depth);
				IndexedTriangleList<Vertex> ceiling = Grid::Make<Vertex>(width, depth);
				IndexedTriangleList<Vertex> left    = Grid::Make<Vertex>(depth, height);
				IndexedTriangleList<Vertex> right   = Grid::Make<Vertex>(depth, height);
				IndexedTriangleList<Vertex> front   = Grid::Make<Vertex>(width, height);
				IndexedTriangleList<Vertex> back    = Grid::Make<Vertex>(width, height);

				// We need to transform every plane to it's proper position. Now they are just overlapping

				// transform the floor
				const Mat3 f = Mat3::RotationX(-PI / 2.0f);
				for (auto& v : floor.vertices)
				{
					v.pos = Mat3::Mul(f, v.pos);
					v.pos.y += height / 2.0f;
				}

				// transform the ceiling
				const Mat3 c = Mat3::RotationX(PI / 2.0f);
				for (auto& v : ceiling.vertices)
				{
					v.pos = Mat3::Mul(c, v.pos);
					v.pos.y -= height / 2.0f;
				}

				// transform the left wall
				const Mat3 l = Mat3::RotationY(PI / 2.0f);
				for (auto& v : left.vertices)
				{
					v.pos = Mat3::Mul(l, v.pos);
					v.pos.x -= width / 2.0f;
				}

				// transform the right wall
				const Mat3 r = Mat3::RotationY(-PI / 2.0f);
				for (auto& v : right.vertices)
				{
					v.pos = Mat3::Mul(r, v.pos);
					v.pos.x += width / 2.0f;
				}

				// transform the front wall
				const Mat3 fr = Mat3::RotationY(PI);
				for (auto& v : front.vertices)
				{
					v.pos = Mat3::Mul(fr, v.pos);
					v.pos.z -= depth / 2.0f;
				}

				// transform the back wall
				for (auto& v : back.vertices)
				{
					v.pos.z += depth / 2.0f;
				}

				// finally, build the room
				std::vector<IndexedTriangleList<Vertex>> room(6);

				room.push_back(floor);
				room.push_back(ceiling);
				room.push_back(left);
				room.push_back(right);
				room.push_back(front);
				room.push_back(back);

				return room;
			}
			
			template<typename Vertex>
			static std::vector<IndexedTriangleList<Vertex>> MakeTex(const index_type width = 4u, const index_type height = 3u, const index_type depth = 5u)
			{
				IndexedTriangleList<Vertex> floor   = Grid::MakeTex<Vertex>(width, depth);
				IndexedTriangleList<Vertex> ceiling = Grid::MakeTex<Vertex>(width, depth);
				IndexedTriangleList<Vertex> left    = Grid::MakeTex<Vertex>(depth, height);
				IndexedTriangleList<Vertex> right   = Grid::MakeTex<Vertex>(depth, height);
				IndexedTriangleList<Vertex> front   = Grid::MakeTex<Vertex>(width, height);
				IndexedTriangleList<Vertex> back    = Grid::MakeTex<Vertex>(width, height);

				// We need to transform every plane to it's proper position. Now they are just overlapping

				// transform the floor
				const Mat3 f = Mat3::RotationX(-PI / 2.0f);
				for (auto& v : floor.vertices)
				{
					v.pos = Mat3::Mul(f, v.pos);
					v.pos.y += height / 2.0f;
				}

				// transform the ceiling
				const Mat3 c = Mat3::RotationX(PI / 2.0f);
				for (auto& v : ceiling.vertices)
				{
					v.pos = Mat3::Mul(c, v.pos);
					v.pos.y -= height / 2.0f;
				}

				// transform the left wall
				const Mat3 l = Mat3::RotationY(PI / 2.0f);
				for (auto& v : left.vertices)
				{
					v.pos = Mat3::Mul(l, v.pos);
					v.pos.x -= width / 2.0f;
				}

				// transform the right wall
				const Mat3 r = Mat3::RotationY(-PI / 2.0f);
				for (auto& v : right.vertices)
				{
					v.pos = Mat3::Mul(r, v.pos);
					v.pos.x += width / 2.0f;
				}

				// transform the front wall
				const Mat3 fr = Mat3::RotationY(PI);
				for (auto& v : front.vertices)
				{
					v.pos = Mat3::Mul(fr, v.pos);
					v.pos.z -= depth / 2.0f;
				}

				// transform the back wall
				for (auto& v : back.vertices)
				{
					v.pos.z += depth / 2.0f;
				}

				// finally, build the room
				std::vector<IndexedTriangleList<Vertex>> room(6);

				room.push_back(floor);
				room.push_back(ceiling);
				room.push_back(left);
				room.push_back(right);
				room.push_back(front);
				room.push_back(back);

				return room;
			}

			template<typename Vertex>
			static std::vector<IndexedTriangleList<Vertex>> MakeNor(const index_type width = 4u, const index_type height = 3u, const index_type depth = 5u)
			{
				IndexedTriangleList<Vertex> floor   = Grid::MakeNor<Vertex>(width, depth);
				IndexedTriangleList<Vertex> ceiling = Grid::MakeNor<Vertex>(width, depth);
				IndexedTriangleList<Vertex> left    = Grid::MakeNor<Vertex>(depth, height);
				IndexedTriangleList<Vertex> right   = Grid::MakeNor<Vertex>(depth, height);
				IndexedTriangleList<Vertex> front   = Grid::MakeNor<Vertex>(width, height);
				IndexedTriangleList<Vertex> back    = Grid::MakeNor<Vertex>(width, height);

				// We need to transform every plane to it's proper position. Now they are just overlapping

				// transform the floor
				const Mat3 f = Mat3::RotationX(-PI / 2.0f);
				for (auto& v : floor.vertices)
				{
					v.pos = Mat3::Mul(f, v.pos);
					v.n   = Mat3::Mul(f, v.n);
					v.pos.y += height / 2.0f;
				}

				// transform the ceiling
				const Mat3 c = Mat3::RotationX(PI / 2.0f);
				for (auto& v : ceiling.vertices)
				{
					v.pos = Mat3::Mul(c, v.pos);
					v.n   = Mat3::Mul(c, v.n);
					v.pos.y -= height / 2.0f;
				}

				// transform the left wall
				const Mat3 l = Mat3::RotationY(PI / 2.0f);
				for (auto& v : left.vertices)
				{
					v.pos = Mat3::Mul(l, v.pos);
					v.n   = Mat3::Mul(l, v.n);
					v.pos.x -= width / 2.0f;
				}

				// transform the right wall
				const Mat3 r = Mat3::RotationY(-PI / 2.0f);
				for (auto& v : right.vertices)
				{
					v.pos = Mat3::Mul(r, v.pos);
					v.n   = Mat3::Mul(r, v.n);
					v.pos.x += width / 2.0f;
				}

				// transform the front wall
				const Mat3 fr = Mat3::RotationY(PI);
				for (auto& v : front.vertices)
				{
					v.pos = Mat3::Mul(fr, v.pos);
					v.n   = Mat3::Mul(fr, v.n);
					v.pos.z -= depth / 2.0f;
				}

				// transform the back wall
				for (auto& v : back.vertices)
				{
					v.pos.z += depth / 2.0f;
				}

				// finally, build the room
				std::vector<IndexedTriangleList<Vertex>> room(6);

				room.push_back(floor);
				room.push_back(ceiling);
				room.push_back(left);
				room.push_back(right);
				room.push_back(front);
				room.push_back(back);

				return room;
			}
		
			template<typename Vertex>
			static std::vector<IndexedTriangleList<Vertex>> MakeTexNor(const index_type width = 4u, const index_type height = 3u, const index_type depth = 5u)
			{
				IndexedTriangleList<Vertex> floor   = Grid::MakeTexNor<Vertex>(width, depth);
				IndexedTriangleList<Vertex> ceiling = Grid::MakeTexNor<Vertex>(width, depth);
				IndexedTriangleList<Vertex> left    = Grid::MakeTexNor<Vertex>(depth, height);
				IndexedTriangleList<Vertex> right   = Grid::MakeTexNor<Vertex>(depth, height);
				IndexedTriangleList<Vertex> front   = Grid::MakeTexNor<Vertex>(width, height);
				IndexedTriangleList<Vertex> back    = Grid::MakeTexNor<Vertex>(width, height);

				// We need to transform every plane to it's proper position. Now they are just overlapping

				// transform the floor
				const Mat3 f = Mat3::RotationX(-PI / 2.0f);
				for (auto& v : floor.vertices)
				{
					v.pos = Mat3::Mul(f, v.pos);
					v.n   = Mat3::Mul(f, v.n);
					v.pos.y += height / 2.0f;
				}

				// transform the ceiling
				const Mat3 c = Mat3::RotationX(PI / 2.0f);
				for (auto& v : ceiling.vertices)
				{
					v.pos = Mat3::Mul(c, v.pos);
					v.n   = Mat3::Mul(c, v.n);
					v.pos.y -= height / 2.0f;
				}

				// transform the left wall
				const Mat3 l = Mat3::RotationY(PI / 2.0f);
				for (auto& v : left.vertices)
				{
					v.pos = Mat3::Mul(l, v.pos);
					v.n   = Mat3::Mul(l, v.n);
					v.pos.x -= width / 2.0f;
				}

				// transform the right wall
				const Mat3 r = Mat3::RotationY(-PI / 2.0f);
				for (auto& v : right.vertices)
				{
					v.pos = Mat3::Mul(r, v.pos);
					v.n   = Mat3::Mul(r, v.n);
					v.pos.x += width / 2.0f;
				}

				// transform the front wall
				const Mat3 fr = Mat3::RotationY(PI);
				for (auto& v : front.vertices)
				{
					v.pos = Mat3::Mul(fr, v.pos);
					v.n   = Mat3::Mul(fr, v.n);
					v.pos.z -= depth / 2.0f;
				}

				// transform the back wall
				for (auto& v : back.vertices)
				{
					v.pos.z += depth / 2.0f;
				}

				// finally, build the room
				std::vector<IndexedTriangleList<Vertex>> room(6);

				room[0] = floor;
				room[1] = ceiling;
				room[2] = left;
				room[3] = right;
				room[4] = front;
				room[5] = back;

				return room;
			}

		};

		class OBJModel
		{
		public:
			OBJModel(const std::string& filename)
			{
				// Open the specified file
				std::ifstream file;
				file.open(filename.c_str());

				if (file.is_open())
				{
					// Every line will be stored here
					std::string line;
					std::stringstream stream;

					// A new mesh is encountered if a vertex is loaded after a face.
					bool previous_was_a_face = false;
					nMeshes = 0;

					// Now iterate through every line of the file
					while (std::getline(file, line))
					{
						// Skip if invalid line
						if (line.size() <= 2)
						{
							continue;
						}

						// Transfer the line into the stream, for easy numeric reading
						stream = std::stringstream(line);

						// In an OBJ file the first two characters gives information
						// about the type of data you will find.
						const char l0 = line[0];
						const char l1 = line[1];
						if ((l0 == 'v') && (l1 == ' '))
						{
							if (previous_was_a_face)
							{
								nMeshes++;
								previous_was_a_face = false;
							}
							LoadPosition(stream); // v 1 2 3
						}
						if ((l0 == 'v') && (l1 == 't'))
						{
							LoadTexCoord(stream); // vt 0.2 0.7
							hasTexCoords = true;
						}
						if ((l0 == 'v') && (l1 == 'n'))
						{
							LoadNormal(stream);   // vn 0.12 -3.5 2.6023
							hasNormals = true;
						}
						if (l0 == 'f')
						{
							previous_was_a_face = true;
							LoadFace(stream);     
						}
					}
				}
				else
				{
					throw std::exception(("Couldn't open the specified file: " + filename).c_str());
				}
			}
		private:
			void LoadPosition(std::stringstream& stream)
			{
				// The position (also called vertex) is in the form "v x y z"
				Vec3 pos;

				// Junk, for ignoring the first character
				char skip;

				// Fill the position and accumulate it to the set of positions
				stream >> skip >> pos.x >> pos.y >> pos.z;
				positions.push_back(pos);
			}
			void LoadTexCoord(std::stringstream& stream)
			{
				// The texture coordinate is in the form "vt u v"
				Vec2 tc;

				// Junk, for ignoring characters
				char skip;

				// Read the texture coordinate and accumulate it to the set of texture coordinares
				stream >> skip >> skip >> tc.x >> tc.y >> skip;
				texCoords.push_back(tc);
			}
			void LoadNormal(std::stringstream& stream)
			{
				// The normal is in the form "vn x y z"
				Vec3 n;

				// Junk, for ignoring characters
				char skip;

				// Fill the normal and accumulate it to the set of normals
				stream >> skip >> skip >> n.x >> n.y >> n.z;
				normals.push_back(n);
			}
			void LoadFace(std::stringstream& stream)
			{
				// The loading of the indices is more tricky. Its form depends on
				// which kind of data are present in the obj file, and there are four cases.
				if (!hasNormals && !hasTexCoords)
				{
					HandleNoNorNoTex(stream);
				}
				if (!hasNormals && hasTexCoords)
				{
					HandleNoNorTex(stream);
				}
				if (hasNormals && !hasTexCoords)
				{
					HandleNorNoTex(stream);
				}
				if (hasNormals && hasTexCoords)
				{
					HandleNorTex(stream);
				}
			}
			void HandleNoNorNoTex(std::stringstream& stream)
			{
				// First case: No textures and no Normals (pos)    

				// Prepare the container for the indices
				index_type i[3];

				// Junk, for ignoring characters
				char skip;

				// Read the three indices  f 0 1 2  (pos) 
				stream >> skip >> i[0] >> i[1] >> i[2];

				// Accumulate the indices in the set of position indices. 
				// Subtract one because they start from one in obj files (why??)
				posIndices.push_back(i[0] - 1u);
				posIndices.push_back(i[1] - 1u);
				posIndices.push_back(i[2] - 1u);
			}
			void HandleNorNoTex(std::stringstream& stream)
			{
				// Second case: Only normals 
				// f 0//1 2//3 4//5  (pos//nor)

				// Prepare the container for the indices
				index_type i[6];

				// Junk, for ignoring characters
				char skip;

				// Fill the three position indices and normal indices  f 0//1 2//3 4//5
				stream >> skip >> i[0] >> skip >> skip >> i[1] >> i[2] >> skip >> skip >> i[3] >> i[4] >> skip >> skip >> i[5];

				// Accumulate to the set of position indices
				// Subtract one because they start from one in obj files (why??)
				posIndices.push_back(i[0] - 1u);
				posIndices.push_back(i[2] - 1u);
				posIndices.push_back(i[4] - 1u);

				// Accumulate to the set of normal indices
				norIndices.push_back(i[1] - 1u);
				norIndices.push_back(i[3] - 1u);
				norIndices.push_back(i[5] - 1u);
			}
			void HandleNoNorTex(std::stringstream& stream)
			{
				// Third case: Only textures 
				// f 0/1 2/3 4/5  (pos/tex) 

				// Prepare the container for the indices
				index_type i[6];

				// Junk, for ignoring characters
				char skip;

				// Fill the three position and texture coordinates indices  f 0/1 2/3 4/5
				stream >> skip >> i[0] >> skip >> i[1] >> i[2] >> skip >> i[3] >> i[4] >> skip >> i[5];

				// Accumulate to the set of position indices
				// Subtract one because they start from one in obj files (why??)
				posIndices.push_back(i[0] - 1u);
				posIndices.push_back(i[2] - 1u);
				posIndices.push_back(i[4] - 1u);

				// Accumulate to the set of texture coordinates indices (same)
				texIndices.push_back(i[1] - 1u);
				texIndices.push_back(i[3] - 1u);
				texIndices.push_back(i[5] - 1u);
			}
			void HandleNorTex(std::stringstream& stream)
			{
				// Forth case
				// f 0/1/2 3/4/5 6/7/8  textures and normals (pos/tex/nor)
				index_type i[9];

				// Junk, for ignoring characters
				char skip;

				// Fill the three position indices, the three normal indices and the three texture coordinate indices
				// f 0/1/2 3/4/5 6/7/8
				stream >> skip >> i[0] >> skip >> i[1] >> skip >> i[2] >> i[3] >> skip >> i[4] >> skip >> i[5] >> i[6] >> skip >> i[7] >> skip >> i[8];

				// Position indices
				posIndices.push_back(i[0] - 1u);
				posIndices.push_back(i[3] - 1u);
				posIndices.push_back(i[6] - 1u);

				// Texture coordinates indices
				texIndices.push_back(i[1] - 1u);
				texIndices.push_back(i[4] - 1u);
				texIndices.push_back(i[7] - 1u);

				// Normal indices
				norIndices.push_back(i[2] - 1u);
				norIndices.push_back(i[5] - 1u);
				norIndices.push_back(i[8] - 1u);
			}
		public:
			// Data
			std::vector<Vec3> positions;
			std::vector<Vec3> normals;
			std::vector<Vec2> texCoords;
		public:
			// Indices into Data
			std::vector<index_type> posIndices;
			std::vector<index_type> norIndices;
			std::vector<index_type> texIndices;
		public:
			// Info
			bool hasNormals   = false;
			bool hasTexCoords = false;
			unsigned int nMeshes = 0u;
		};

		class Import
		{
		public:
			// Requires Vertex that have pos attribute.
			template<typename Vertex>
			static IndexedTriangleList<Vertex> FromFile(const std::string& filename)
			{
				std::vector<Vertex> vertices;
				std::vector<index_type> indices;

				OBJModel mesh{ filename };

				// Now let's use the aquired data to build our Vertex type.

				for (unsigned int i = 0; i < mesh.positions.size(); i++)
				{
					Vertex v;
					v.pos = mesh.positions[i];
					vertices.push_back(v);
				}

				indices = mesh.posIndices;

				return { std::move(vertices),std::move(indices) };
			}
	
			// Requires Vertex that have pos and nor attributes.
			template<typename Vertex>
			static IndexedTriangleList<Vertex> FromFileNor(const std::string& filename)
			{
				std::vector<Vertex> vertices;
				std::vector<index_type> indices;

				OBJModel mesh{ filename };

				if (mesh.hasNormals)
				{
					Vertex v0;
					Vertex v1;
					Vertex v2;
					// Iterate through every triangle and read the indexed data
					for (unsigned int i = 0u; i < mesh.posIndices.size(); i += 3u)
					{
						// Fill the vertex positions from the previously loaded indices
						v0.pos = mesh.positions[mesh.posIndices[(size_t)i + 0u]];
						v1.pos = mesh.positions[mesh.posIndices[(size_t)i + 1u]];
						v2.pos = mesh.positions[mesh.posIndices[(size_t)i + 2u]];

						// Read vertex the normals from the previously loaded indices
						v0.n = mesh.normals[mesh.norIndices[(size_t)i + 0u]];
						v1.n = mesh.normals[mesh.norIndices[(size_t)i + 1u]];
						v2.n = mesh.normals[mesh.norIndices[(size_t)i + 2u]];

						// Push the three vertices
						vertices.push_back(v0);
						vertices.push_back(v1);
						vertices.push_back(v2);

						// Push the corresponding indices
						indices.push_back(i + 0u);
						indices.push_back(i + 1u);
						indices.push_back(i + 2u);
					}
				}
				else
				{
					throw std::exception((std::string("The loaded file doesn't have normals! ") + filename).c_str());
				}

				return { std::move(vertices),std::move(indices) };
			}
	
			// Requires Vertex that have pos and tex attributes.
			template<typename Vertex>
			static IndexedTriangleList<Vertex> FromFileTex(const std::string& filename)
			{
				std::vector<Vertex> vertices;
				std::vector<index_type> indices;

				OBJModel mesh{ filename };

				if (mesh.hasTexCoords)
				{
					Vertex v0;
					Vertex v1;
					Vertex v2;
					// Iterate through every triangle and read the indexed data
					for (unsigned int i = 0u; i < mesh.posIndices.size(); i += 3u)
					{
						// Fill the vertex mesh.positions from the previously loaded indices
						v0.pos = mesh.positions[mesh.posIndices[(size_t)i + 0u]];
						v1.pos = mesh.positions[mesh.posIndices[(size_t)i + 1u]];
						v2.pos = mesh.positions[mesh.posIndices[(size_t)i + 2u]];

						// Read the vertex texture coordinates from the previously loaded indices
						v0.tex = mesh.texCoords[mesh.texIndices[(size_t)i + 0u]];
						v1.tex = mesh.texCoords[mesh.texIndices[(size_t)i + 1u]];
						v2.tex = mesh.texCoords[mesh.texIndices[(size_t)i + 2u]];

						// Push the three vertices
						vertices.push_back(v0);
						vertices.push_back(v1);
						vertices.push_back(v2);

						// Push the corresponding indices
						indices.push_back(i + 0u);
						indices.push_back(i + 1u);
						indices.push_back(i + 2u);
					}
				}
				else
				{
					throw std::exception((std::string("The loaded file doesn't have texture coordinates! ") + filename).c_str());
				}
			}
	
			// Requires Vertex that have pos, tex and nor attributes.
			template<typename Vertex>
			static IndexedTriangleList<Vertex> FromFileTexNor(const std::string& filename)
			{
				std::vector<Vertex> vertices;
				std::vector<index_type> indices;

				OBJModel mesh{ filename };

				if (mesh.hasNormals && mesh.hasTexCoords)
				{
					Vertex v0;
					Vertex v1;
					Vertex v2;
					// Iterate through every triangle and read the indexed data
					for (unsigned int i = 0u; i < mesh.posIndices.size(); i += 3u)
					{
						// Fill the vertex mesh.positions from the previously loaded indices
						v0.pos = mesh.positions[mesh.posIndices[(size_t)i + 0u]];
						v1.pos = mesh.positions[mesh.posIndices[(size_t)i + 1u]];
						v2.pos = mesh.positions[mesh.posIndices[(size_t)i + 2u]];

						// Read the vertex texture coordinates from the previously loaded indices
						v0.tex = mesh.texCoords[mesh.texIndices[(size_t)i + 0u]];
						v1.tex = mesh.texCoords[mesh.texIndices[(size_t)i + 1u]];
						v2.tex = mesh.texCoords[mesh.texIndices[(size_t)i + 2u]];

						// Read the normal coodinates from the previously loaded indices
						v0.n = mesh.normals[mesh.norIndices[(size_t)i + 0u]];
						v1.n = mesh.normals[mesh.norIndices[(size_t)i + 1u]];
						v2.n = mesh.normals[mesh.norIndices[(size_t)i + 2u]];

						// Push the three vertices
						vertices.push_back(v0);
						vertices.push_back(v1);
						vertices.push_back(v2);

						// Push the corresponding indices (stupid ascending order)
						indices.push_back(i + 0u);
						indices.push_back(i + 1u);
						indices.push_back(i + 2u);
					}
				}
				else
				{
					if (!mesh.hasNormals && !mesh.hasTexCoords)
					{
						throw std::exception((std::string("The loaded file doesn't have normals and texture coordinates! ") + filename).c_str());
					}
					if (!mesh.hasNormals && mesh.hasTexCoords)
					{
						throw std::exception((std::string("The loaded file doesn't have normals! ") + filename).c_str());
					}
					if (mesh.hasNormals && !mesh.hasTexCoords)
					{
						throw std::exception((std::string("The loaded file doesn't have texture coordinates! ") + filename).c_str());
					}
				}
				return { std::move(vertices),std::move(indices) };
			}
		};

		class Circle
		{
		public:
			template<typename Vertex>
			static IndexedLineList<Vertex> Make(const unsigned int nTessellations = 40u)
			{
				IndexedLineList<Vertex> polyline;

				assert(nTessellations > 2u && "The number of subdivisions for a circle must be at least 2");

				polyline.vertices.resize(nTessellations);
				polyline.indices.resize((size_t)2u * nTessellations);

				for (unsigned int i = 0u; i < nTessellations; i++)
				{
					const float t = i * twoPI / (float)nTessellations;
					polyline.vertices[i].pos = { std::cos(t),-std::sin(t),0.0f };
				}

				for (unsigned int i = 0u; i < 2u * nTessellations - 1u; i++)
				{
					polyline.indices[i] = (i + 1) / 2u;
				}
				polyline.indices[(size_t)2u * nTessellations - 1u] = 0;

				return polyline;
			}
		
			template<typename Vertex>
			static IndexedLineList<Vertex> MakeCol(const unsigned int nTessellations = 40u)
			{
				IndexedLineList<Vertex> polyline = Make<Vertex>(nTessellations);

				// Apply random colors to every vertex
				float phi = 0.0f;
				const float dphi = twoPI / (float)nTessellations;
				for (auto& v : polyline.vertices)
				{
					v.col = FromHSV<Vec3>(phi, 1.0f, 1.0f);
					phi += dphi;
				}

				return polyline;
			}
		
			template<typename Vertex>
			static IndexedLineList<Vertex> MakeNor(const unsigned int nTessellations = 40u)
			{
				IndexedLineList<Vertex> polyline = Make<Vertex>(nTessellations);

				for (auto& v : polyline.vertices)
				{
					v.n = { 0.0f,0.0f,-1.0f };
				}

				return polyline;
			}

			template<typename Vertex>
			static IndexedLineList<Vertex> MakeColNor(const unsigned int nTessellations = 40u)
			{
				IndexedLineList<Vertex> polyline = MakeCol<Vertex>(nTessellations);

				for (auto& v : polyline.vertices)
				{
					v.n = { 0.0f,0.0f,-1.0f };
				}

				return polyline;
			}
		};

		class Line
		{
		public:
			template<typename Vertex>
			static IndexedLineList<Vertex> Make(const unsigned int nTessellations = 1u)
			{
				IndexedLineList<Vertex> line;

				line.vertices.resize((size_t)nTessellations + 1u);
				line.indices.resize((size_t)nTessellations * 2u);

				float x = -1.0f;
				const float dx = 2.0f / (float)nTessellations;
				for (unsigned int i = 0u; i < nTessellations + 1u; i++)
				{
					line.vertices[i].pos = { x,0.0f,0.0f };
					x += dx;
				}

				for (unsigned int i = 0u; i < nTessellations * 2u; i++)
				{
					line.indices[i] = (i + 1) / 2;
				}

				// { 0,1 };  // n = 1  N = 2
				// { 0,1,1,2 };  // n = 2  N = 4
				// { 0,1,1,2,2,3 };  // n = 3  N = 6
				// { 0,1,1,2,2,3,3,4 };  // n = 4  N = 8
				// { 0,1,1,2,2,3,3,4,4,5 };  // n = 5  N = 10
				// { 0,1,1,2,2,3,3,4,4,5,5,6 };  // n = 6  N = 12

				return line;
			}

			template<typename Vertex>
			static IndexedLineList<Vertex> MakeCol(const unsigned int nTessellations = 1u)
			{
				IndexedLineList<Vertex> line = Make<Vertex>(nTessellations);

				float hue = 0.0f;
				const float dHue = twoPI / (float)nTessellations;
				for (auto& v : line.vertices)
				{
					v.col = FromHSV<Vec3>(hue, 1.0f, 1.0f);
					hue += dHue;
				}

				return line;
			}
		};
	}
}