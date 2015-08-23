//-------------------------------------------------------------------
//							A4DataType.h
//-------------------------------------------------------------------


#ifndef __A4MATH_TYPE__H__
#define __A4MATH_TYPE__H__


#include <cstddef>
#include <limits>
#include <algorithm>
#include <iterator>
#include <vector>
#include <cassert>

//#include "wykobi_math.hpp"

	
	

   //static const char VERSION_INFORMATION[] = "Wykobi Version 0.0.4";
   //static const char AUTHOR_INFORMATION[]  = "Arash Partow";
   //static const char EPOCH_VERSION[]       = "21744C5A:44A4153E:EB32F784";

   template<typename T, std::size_t D>
   class pointnd;

   template<typename T = float>
   class point2d 
   {
   public:
      typedef T           type;
      typedef const type& const_reference;
      typedef       type& reference;

      point2d() : x(T(0.0)), y(T(0.0)){}
      point2d(const pointnd<T,2>& point) : x(point[0]), y(point[1]){}
     ~point2d(){}

      inline point2d<T>& operator=(const pointnd<T,2>& point)
      {
         x = point[0];
         y = point[1];
         return *this;
      }

      inline reference       operator()(const std::size_t& index)       { return ((0 == index)? x : y); }
      inline const_reference operator()(const std::size_t& index) const { return ((0 == index)? x : y); }

      inline reference       operator[](const std::size_t& index)       { return ((0 == index)? x : y); }
      inline const_reference operator[](const std::size_t& index) const { return ((0 == index)? x : y); }

	  inline T Length()	{	return sqrtf(x * x + y * y);		}
	  T		distance(const point2d<T>& vVec) const
		{
			return (*this - vVec).Length();
		}

      T x,y;
   };

   template<typename T = float>
   class point3d 
   {
	public:
		typedef T           Type;
		typedef const Type& const_reference;
		typedef       Type& reference;

		point3d() : x(T(0.0)), y(T(0.0)), z(T(0.0)){}
		point3d(const pointnd<T,3>& point) : x(point[0]), y(point[1]), z(point[2]){}
		point3d(const point3d<T>& vVec) : x(vVec.x), y(vVec.y), z(vVec.z){}
		point3d(const T& _x, const T& _y, const T& _z) : x(_x), y(_y), z(_z){}

		~point3d(){}

		inline point3d<T>& operator=(const pointnd<T,3>& point)
		{
			x = point[0];
			y = point[1];
			z = point[2];
			return *this;
		}

		inline reference       operator()(const std::size_t& index)       { return value(index); }
		inline const_reference operator()(const std::size_t& index) const { return value(index); }

		inline reference       operator[](const std::size_t& index)       { return value(index); }
		inline const_reference operator[](const std::size_t& index) const { return value(index); }

		inline point3d<T> operator +(const point3d<T>& vVec) const
		{
			return point3d<T>(x + vVec.x, y + vVec.y, z + vVec.z);
		}

		inline point3d<T> operator -(const point3d<T>& vVec) const
		{
			return point3d<T>(x - vVec.x, y - vVec.y, z - vVec.z);
		}

		inline point3d<T> operator *(T fScale) const
		{
			return point3d<T>(x * fScale, y * fScale, z * fScale);
		}

		inline point3d<T> operator /(T fScale) const
		{
			return point3d<T>(x / fScale, y / fScale, z / fScale);
		}



      T x,y,z;
   private:
      inline reference value(const std::size_t& index)
      {
         switch(index)
         {
            case 0  : return x;
            case 1  : return y;
            case 2  : return z;
            default : return x;
         }
      }

      inline const_reference value(const std::size_t& index) const
      {
         switch(index)
         {
            case 0  : return x;
            case 1  : return y;
            case 2  : return z;
            default : return x;
         }
      }
   };

   template<typename T, std::size_t D>
   class pointnd 
   {
   public:
      typedef const T& const_reference;
      typedef       T& reference;

      pointnd(){ clear(); }
      pointnd(const T& v0) { v[0] = v0; }
      pointnd(const T& v0, const T& v1) { v[0] = v0; v[1] = v1; }
      pointnd(const T& v0,const T& v1, const T& v2) { v[0] = v0; v[1] = v1; v[2] = v2; }
      pointnd(const T& v0,const T& v1, const T& v2, const T& v3) { v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3; }
      pointnd(const pointnd<T,D>& point)
      {
         for(std::size_t i = 0; i < D; ++i) v[i] = point.v[i];
      }

      pointnd(const point2d<T>& point)
      {
         for(std::size_t i = 0; i < D; ++i) v[i] = point[i];
      }

      pointnd(const point3d<T>& point)
      {
         for(std::size_t i = 0; i < D; ++i) v[i] = point[i];
      }

     ~pointnd(){}

      void clear()
      {
         for(std::size_t i = 0; i < D; ++i) v[i] = T(0.0);
      }

      inline pointnd<T,D>& operator=(const pointnd<T,D>& point)
      {
         if (this == &point) return *this;
         for(std::size_t i = 0; i < D; ++i) v[i] = point.v[i];
         return *this;
      }

      inline pointnd<T,D>& operator=(const point2d<T>& point)
      {
         if (D == 2)
         {
            v[0] = point.x;
            v[1] = point.y;
         }
         return *this;
      }

      inline pointnd<T,D>& operator=(const point3d<T>& point)
      {
         if (D == 3)
         {
            v[0] = point.x;
            v[1] = point.y;
            v[2] = point.z;
         }
         return *this;
      }

      inline reference       operator()(const std::size_t& index)       { return v[index]; }
      inline const_reference operator()(const std::size_t& index) const { return v[index]; }

      inline reference       operator[](const std::size_t& index)       { return v[index]; }
      inline const_reference operator[](const std::size_t& index) const { return v[index]; }

   private:
      T v[D];
   };

   template<typename T, std::size_t Dimension>
   class define_point_type      { public: typedef pointnd<T,Dimension> PointType; };

   template<typename T>
   class define_point_type<T,2> { public: typedef point2d<T> PointType; };

   template<typename T>
   class define_point_type<T,3> { public: typedef point3d<T> PointType; };


   

   

   
   /************[       Rectangle       ]************/
   template<typename T>
   class rectangle 
   {
   public:
      const static std::size_t PointCount = 2;

      rectangle(){}
     ~rectangle(){}

      typedef typename define_point_type<T,2>::PointType PointType;
      typedef const PointType& const_reference;
      typedef       PointType& reference;

   private:
      PointType _data[PointCount];

   public:
      inline reference       operator [](const std::size_t& index)       { return _data[index]; }
      inline const_reference operator [](const std::size_t& index) const { return _data[index]; }
      inline std::size_t     size       ()                         const { return PointCount;   }
   };


   
   //-----------[     Polygon Type      ]-----------/
   template<typename T, std::size_t Dimension>
   class polygon 
   {
   public:
      polygon(const std::size_t initial_size = 0) : _data(initial_size){}
     ~polygon(){}

      typedef typename define_point_type<T,Dimension>::PointType PointType;
      typedef const PointType& const_reference;
      typedef PointType& reference;

   private:
      std::vector<PointType> _data;

   public:
      typedef typename std::vector<PointType>::iterator iterator;
      typedef typename std::vector<PointType>::const_iterator const_iterator;

   public:
      inline reference       operator [](const std::size_t& index)       { return _data[index];                }
      inline const_reference operator [](const std::size_t& index) const { return _data[index];                }
      inline void            push_back  (const PointType&  value)        { _data.push_back(value);             }
      inline void            reserve    (const std::size_t amount)       { _data.reserve(amount);              }
      inline void            clear      ()                         const { _data.clear();                      }
      inline void            clear      ()                               { _data.clear();                      }
      inline void            erase      (const std::size_t index)        { _data.erase(_data.begin() + index); }
      inline std::size_t     size       ()                         const { return _data.size();                }
      inline const_iterator  begin      ()                         const { return _data.begin();               }
      inline iterator        begin      ()                               { return _data.begin();               }
      inline const_iterator  end        ()                         const { return _data.end();                 }
      inline iterator        end        ()                               { return _data.end();                 }
      inline reference       front      ()                               { return _data.front();               }
      inline const_reference front      ()                         const { return _data.front();               }
      inline reference       back       ()                               { return _data.back();                }
      inline const_reference back       ()                         const { return _data.back();                }
      inline void            reverse    ()                               { std::reverse(_data.begin(),_data.end());}
   };


   template<typename T, std::size_t D>
   class vectornd;

   template<typename T>
   class vector2d : public point2d<T>
   {
   public:
      vector2d(const T& _x = T(0.0), const T& _y = T(0.0))
      {
         point2d<T>::x = _x;
         point2d<T>::y = _y;
      }

      inline vector2d<T>& operator=(const vectornd<T,2>& vec)
      {
         point2d<T>::x = vec[0];
         point2d<T>::y = vec[1];
         return *this;
      }
   };

   template<typename T>
   class vector3d : public point3d<T>
   {
	public:
		vector3d(const T& _x = T(0.0), const T& _y = T(0.0), const T& _z = T(0.0))
		{
			point3d<T>::x = _x;
			point3d<T>::y = _y;
			point3d<T>::z = _z;
		}

		inline vector3d<T>& operator=(const vectornd<T,3>& vec)
		{
			point3d<T>::x = vec[0];
			point3d<T>::y = vec[1];
			point3d<T>::z = vec[2];
			return *this;
		}

		inline vector3d<T> operator - () const
		{
			return vector3d<T>(-x, -y, -z);
		}
		inline vector3d<T> operator -(const vectornd<T,3>& vec) const
		{
			return vector3d<T>(point3d<T>::x - vec[0], point3d<T>::y - vec[1], point3d<T>::z - vec[2]);
		}

		inline vector3d<T> operator +(const vectornd<T,3>& vec) const
		{
			return vector3d<T>(point3d<T>::x + vec[0], point3d<T>::y + vec[1], point3d<T>::z + vec[2]);
		}

		inline vector3d<T> operator *(const T fScale) const
		{
			return vector3d<T>(point3d<T>::x * fScale, point3d<T>::y * fScale, point3d<T>::z * fScale);
		}
		// overloaded operators to help Vector3
		inline friend vector3d<T> operator * ( const float fScalar, const vector3d<T>& rkVector )
		{
			return vector3d<T>(fScalar* rkVector.x,	fScalar * rkVector.y, fScalar * rkVector.z);
		}

		inline bool operator ==(const point3d<T>& vec) const
		{
			return !((fabs( point3d<T>::x - vec.x) > MATH_EPSILON) || (fabs(point3d<T>::y - vec.y) > MATH_EPSILON)
					 || (fabs(point3d<T>::z - vec.z) > MATH_EPSILON));
		}

		inline bool operator !=(const point3d<T>& vec) const
		{
			return ((fabs(point3d<T>::x - vec.x) > MATH_EPSILON) || (fabs(point3d<T>::y - vec.y) > MATH_EPSILON)
					|| (fabs(point3d<T>::z - vec.z) > MATH_EPSILON));
		}

		inline float dot(const vector3d<T> vec) const
		{
			return x*vec.x +y*vec.y +z*vec.z;
		}
		inline float normalize()
		{
			T lengthVar = length();
			if (lengthVar > 0)
			{
				T invLength = 1/lengthVar;

				x *= invLength;
				y *= invLength;
				z *= invLength;

				return lengthVar;
			}
			return 0;
		}

		inline bool isZero()
		{
			return x == 0 && y == 0 && z ==0;
		}

		inline void zero()
		{
			x = y = z = 0;
		}

		inline T	lengthOnXZ(){	return sqrtf(x * x + z * z);}
		inline T	length(){	return sqrtf(x * x + y * y + z * z);}
		inline T	lengthSqr(){ return x * x + y * y + z * z;}

		T		distance(const vector3d<T>& vVec) const
		{
			return (*this - vVec).length();
		}

		T		distanceSqr(const vector3d<T>& vVec) const
		{
			return (*this - vVec).lengthSqr();
		}

		T		distanceOnXZ(const vector3d<T>& vVec) const
		{
			return (*this - vVec).lengthOnXZ();
		}
   };

   template<typename T, std::size_t D>
   class vectornd : public pointnd<T,D>
   {
   public:
      vectornd()
      {
         pointnd<T,D>::clear();
      }

      vectornd(const T& v0)
      {
         pointnd<T,D>::v[0] = v0;
      }

      vectornd(const T& v0, const T& v1)
      {
         pointnd<T,D>::v[0] = v0;
         pointnd<T,D>::v[1] = v1;
      }

      vectornd(const T& v0,const T& v1, const T& v2)
      {
         pointnd<T,D>::v[0] = v0;
         pointnd<T,D>::v[1] = v1;
         pointnd<T,D>::v[2] = v2;
      }

      vectornd(const T& v0,const T& v1, const T& v2, const T& v3)
      {
         pointnd<T,D>::v[0] = v0;
         pointnd<T,D>::v[1] = v1;
         pointnd<T,D>::v[2] = v2;
         pointnd<T,D>::v[3] = v3;
      }

      vectornd(const vectornd<T,D>& vec)
      {
         for(std::size_t i = 0; i < D; ++i) (*this)[i] = vec[i];
      }

      vectornd(const vector2d<T>& vec)
      {
         (*this)[0] = vec.x;
         (*this)[1] = vec.y;
      }

      vectornd(const vector3d<T>& vec)
      {
         (*this)[0] = vec.x;
         (*this)[1] = vec.y;
         (*this)[2] = vec.z;
      }
   };


	class SRotation
	{
	public:
		float pitch;
		float yaw;
		float roll;

	public:
		SRotation(){}
		SRotation( float inPitch, float inYaw, float inRoll )
		{
			pitch = inPitch;
			yaw   = inYaw;
			roll  = inRoll;
		}
	};



typedef vector3d<float> vector3;
//typedef vector3 vector3;			// Chamsol : vector3과 SPosition은 이미 혼용해서 사용하고 있음. 이렇게 같은 형이라는 것을 확실히 보여주는 것이 좋을듯
typedef vector2d<float> vector2;

inline bool is_point_in_cube(const vector3& point, const vector3& center,  const vector3& size)
{
  //assert(from.x[0]<to.x[0] && from.x[1]<to.x[1] && from.x[2]<to.x[2]);
	vector3 from, to;
	from.x = center.x -size.x*0.5f;
	from.y = center.y -size.y*0.5f;
	from.z = center.z -size.z*0.5f;

	to.x = from.x + size.x;
	to.y = from.y + size.y;
	to.z = from.z + size.z;

	return (point.x<=to.x && point.y<=to.y && point.z<=to.z &&
			point.x>=from.x && point.y>=from.y && point.z>=from.z);
}

inline bool CheckSphereRayCollision(vector3& rayPos, vector3& rayDir, const vector3& sphereCenter, float r, float& t)
{
    //Compute A, B and C coefficients
	vector3 o = rayPos -sphereCenter;
    float a = rayDir.dot(rayDir);
    float b = 2 * rayDir.dot(o);
    float c = o.dot(o) - (r * r);

    //Find discriminant
    float disc = b * b - 4 * a * c;
    
    // if discriminant is negative there are no real roots, so return 
    // false as ray misses sphere
    if (disc < 0)
        return false;

    // compute q as described above
    float distSqrt = sqrtf(disc);
    float q;
    if (b < 0)
        q = (-b - distSqrt)/2.0f;
    else
        q = (-b + distSqrt)/2.0f;

    // compute t0 and t1
    float t0 = q / a;
    float t1 = c / q;

    // make sure t0 is smaller than t1
    if (t0 > t1)
    {
        // if t0 is bigger than t1 swap them around
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // if t1 is less than zero, the object is in the ray's negative direction
    // and consequently the ray misses the sphere
    if (t1 < 0)
        return false;

    // if t0 is less than zero, the intersection point is at t1
    if (t0 < 0)
    {
        t = t1;
    }
    // else the intersection point is at t0
    else
    {
        t = t0;
    }

    return true;
}

inline vector3 GetCatmullromCurvePoint(vector3& p1, vector3& p2, vector3& p3, vector3& p4, float t)
{
	float tt = t * t;
	float ttt = tt * t;

	return (0.5f * ((-p1 + 3 * p2 - 3 * p3 + p4) * ttt + (2 * p1 - 5 * p2 + 4 * p3 - p4) * tt + (-p1 + p3) * t + 2 * p2)); 
}


#endif 