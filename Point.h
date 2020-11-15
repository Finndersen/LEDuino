#ifndef Point_h
#define  Point_h

#include <cmath>
#include <float.h>
#include "utils.h"


// Structure to represent a cartesian coordinate or vector 
struct Point {
	float x=0.0, y=0.0, z=0.0;
	
	//Initialise explicitly
	Point(float x, float y, float z): x(x), y(y), z(z)	{};
	// 2D (z default to 0)
	Point(float x, float y): x(x), y(y), z(0)	{};
	// Initialise from array
	Point(float* arr): x(arr[0]), y(arr[1]), z(arr[2])	{};
	// Default constructor
	Point(): x(0), y(0), z(0) {};
	
	// Vector Addition and subtraction
	Point& operator+=(const Point &RHS) { x += RHS.x; y += RHS.y; z += RHS.z; return *this; };
	Point& operator-=(const Point &RHS) { x -= RHS.x; y -= RHS.y; z -= RHS.z; return *this; };
	
	Point operator+(const Point &RHS) const { return Point(*this) += RHS; };
	Point operator-(const Point &RHS) const { return Point(*this) -= RHS; };
	
	// Scalar addition and subtraction
	Point& operator+=(const float &RHS) { x += RHS; y += RHS; z += RHS; return *this; };
	Point& operator-=(const float &RHS) { x -= RHS; y -= RHS; z -= RHS; return *this; };

	Point operator+(const float &RHS) const { return Point(*this) += RHS; };
	Point operator-(const float &RHS) const { return Point(*this) -= RHS; };
	
	// Scalar product and division
	Point& operator*=(const float &RHS) { x *= RHS; y *= RHS; z *= RHS; return *this; };
	Point& operator/=(const float &RHS) { x /= RHS; y /= RHS; z /= RHS; return *this; };

	Point operator*(const float &RHS) { return Point(*this) *= RHS; };
	Point operator/(const float &RHS) { return Point(*this) /= RHS; };
	
	// Element-wise multiplication
	Point hadamard(const Point &RHS) {return Point(this->x*RHS.x, this->y*RHS.y, this->z*RHS.z); };
	// Negation
	Point operator-() const {return Point(-x, -y, -z); };
	
	// Euclidean norm
	float norm() const { 
		return std::sqrt(x*x + y*y + z*z); 
	};
	
	// Calculate distance of this point from plane defined by a normal vector and point
	float distance_to_plane(Point& norm_vector, Point& plane_point)	const {
		// Calculate coefficent D of plane equation
		float D = -(norm_vector.x*plane_point.x + norm_vector.y*plane_point.y + norm_vector.z*plane_point.z);
		// Get numerator of distance equation
		float num = abs(norm_vector.x*x + norm_vector.y*y + norm_vector.z*z + D);
		return num / norm_vector.norm();
		
	}

	// Distance to other point
	float distance(const Point& other)	const {
		return std::sqrt(std::pow(other.x-x, 2) + std::pow(other.y-y, 2) + std::pow(other.z-z, 2)); 
	};
};
// Implement binary operators as free (non-member) functions to enable symmetry
inline bool operator==(const Point& lhs, const Point& rhs){ return lhs.x==rhs.x && lhs.y==rhs.y && lhs.z==rhs.z; }
inline bool operator!=(const Point& lhs, const Point& rhs){return !operator==(lhs,rhs);}
inline Point operator/(const float &lhs, const Point &rhs) { return Point(lhs/rhs.x, lhs/rhs.y, lhs/rhs.z); }
inline Point operator*(const float &lhs, Point &rhs) { return rhs*lhs; }
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


#endif
