#ifndef Point_h
#define  Point_h

#include <float.h>
#include "utils.h"
#include "Arduino.h"


// Structure to represent a cartesian coordinate or vector 
class Point: public Printable {
	public:
		float x=0, y=0, z=0;
		
		//Initialise explicitly
		Point(float x, float y, float z): x(x), y(y), z(z)	{};
		// 2D (z default to 0)
		Point(float x, float y): Point(x, y, 0)	{};
		// Initialise from array
		Point(float* arr): Point(arr[0], arr[1], arr[2])	{};
		// Default constructor
		Point(): Point(0, 0, 0) {};
		
		// Vector Addition and subtraction
		Point& operator+=(const Point &RHS) { 
			x += RHS.x; 
			y += RHS.y; 
			z += RHS.z; 		
			return *this; 
		};
		Point& operator-=(const Point &RHS) { 
			x -= RHS.x; 
			y -= RHS.y; 
			z -= RHS.z; 
			return *this; };
		
		Point operator+(const Point &RHS) const { return Point(x + RHS.x, y + RHS.y, z + RHS.z); };
		Point operator-(const Point &RHS) const { return Point(x - RHS.x, y - RHS.y, z - RHS.z); };
		
		// Scalar addition and subtraction
		Point& operator+=(const float &RHS) { 
			x += RHS; 
			y += RHS; 
			z += RHS; 
			return *this; 
		};
		Point& operator-=(const float &RHS) { 
			x -= RHS; 
			y -= RHS; 
			z -= RHS; 
			return *this; 
		};

		Point operator+(const float &RHS) const { return Point(x+RHS, y+RHS, z+RHS);};
		Point operator-(const float &RHS) const { return Point(x-RHS, y-RHS, z-RHS);};
		
		// Scalar product and division
		template<typename T>
		Point& operator*=(const T RHS) {	
			this->x *= RHS; 
			this->y *= RHS; 
			this->z *= RHS; 
			return *this; 
		};
		template<typename T>
		Point& operator/=(const T RHS) { 
			this->x /= RHS; 
			this->y /= RHS; 
			this->z /= RHS; 
			return *this; 
		};
		
		template<typename T>
		Point operator*(const T RHS) const { return Point(*this) *= RHS; };
		template<typename T>
		Point operator/(const T RHS) const { return Point(*this) /= RHS; };
		
		// Element-wise multiplication and division
		Point hadamard_product(const Point &RHS) {return Point(this->x*RHS.x, this->y*RHS.y, this->z*RHS.z); };
		Point hadamard_divide(const Point &RHS) {return Point(this->x/RHS.x, this->y/RHS.y, this->z/RHS.z); };
		
		// Negation
		Point operator-() const {return Point(-x, -y, -z); };
		
		// Euclidean norm 
		const float norm() const { 
			return sqrt(x*x + y*y + z*z); 
		};
		
		// Calculate distance of this point from plane defined by a normal vector and point
		float distance_to_plane(Point& norm_vector, Point& plane_point)	const {
			// Calculate coefficent D of plane equation
			float D = norm_vector.x*plane_point.x + norm_vector.y*plane_point.y + norm_vector.z*plane_point.z;
			// Get numerator of distance equation
			float num = abs(norm_vector.x*x + norm_vector.y*y + norm_vector.z*z - D);
			return num / norm_vector.norm();
			
		}

		// Distance to other point
		float distance(const Point& other)	const {
			return sqrt(this->distance_squared(other)); 
		};
		
		// Square of Distance to other point (useful for doing distance comparisons and dont want to square root)
		float distance_squared(const Point& other)	const {
			return pow(other.x-this->x, 2) + pow(other.y-this->y, 2) + pow(other.z-this->z, 2); 
		};
		
		
		size_t printTo(Print& p) const {
			size_t size;
			size = p.print("(");
			size += p.print(this->x);
			size += p.print(", ");
			size += p.print(this->y);
			size += p.print(", ");
			size += p.print(this->z);
			size += p.print(")");
			return size;
		}
};
// Implement binary operators as free (non-member) functions to enable symmetry
inline bool operator==(const Point& lhs, const Point& rhs){ return lhs.x==rhs.x && lhs.y==rhs.y && lhs.z==rhs.z; }
inline bool operator!=(const Point& lhs, const Point& rhs){return !operator==(lhs,rhs);}

template<typename T>
inline Point operator/(const T lhs, const Point &rhs) { return Point(lhs/rhs.x, lhs/rhs.y, lhs/rhs.z); }
template<typename T>
inline Point operator*(const T lhs, const Point &rhs) { return rhs*lhs; }
//inline bool operator< (const Point& lhs, const Point& rhs){ /* do actual comparison */ }
//inline bool operator> (const Point& lhs, const Point& rhs){return  operator< (rhs,lhs);}
//inline bool operator<=(const Point& lhs, const Point& rhs){return !operator> (lhs,rhs);}
//inline bool operator>=(const Point& lhs, const Point& rhs){return !operator< (lhs,rhs);}

// Direction vectors
Point v_x(1, 0, 0);
Point v_y(0, 1, 0);
Point v_z(0, 0, 1);

Point undefinedPoint(FLT_MIN, FLT_MIN, FLT_MIN);

// Class to define bounding box (rectangular prism) defined by minimum (bottom left) and maximum (top right) points
class Bounds {
	public: 
		Bounds(Point min, Point max): min(min), max(max) {}
		
		// Get vector which represents magnitude of bounds in each coordinate (width, length and depth)
		Point magnitude() {
			return Point(this->max.x - this->min.x, this->max.y - this->min.y, this->max.z - this->min.z);
		}
		
		// Centre point of bounds
		Point centre() {
			return Point(this->max.x + this->min.x, this->max.y + this->min.y, this->max.z + this->min.z)/2;
		}
		
		// Whether or not point is contained in bounds
		bool contains(Point point) {
			return ((point.x <= this->max.x) && (point.x >= this->min.x) &&
					(point.y <= this->max.y) && (point.y >= this->min.y) &&
					(point.z <= this->max.z) && (point.z >= this->min.z)
			);
		}
		
		Point min, max;
};

// Get Bounds of an array of points
Bounds get_bounds_of_points(Point* points, uint16_t num_points) {
	Point max(FLT_MIN, FLT_MIN, FLT_MIN);
	Point min(FLT_MAX, FLT_MAX, FLT_MAX);
	for (uint16_t i=0; i < num_points; i++) {
		Point& point = points[i];
		if (point.x > max.x) max.x = point.x;
		if (point.y > max.y) max.y = point.y;
		if (point.z > max.z) max.z = point.z;

		if (point.x < min.x) min.x = point.x;
		if (point.y < min.y) min.y = point.y;
		if (point.z < min.z) min.z = point.z;
	}
	return Bounds(min, max);
};

#endif
