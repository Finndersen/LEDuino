#ifndef Point_h
#define  Point_h

#include <cmath>
#include "utils.h"


// Structure to represent a cartesian coordinate or vector 
struct Point {
	float x=0.0, y=0.0, z=0.0;
	
	//Initialise explicitly
	Point(float x, float y, float z): x(x), y(y), z(z)	{};
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
	
	// Negation
	Point operator-() const {return Point(-x, -y, -z); };
	
	// Euclidean norm
	float norm() const { 
		return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)); 
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
//inline bool operator< (const Point& lhs, const Point& rhs){ /* do actual comparison */ }
//inline bool operator> (const Point& lhs, const Point& rhs){return  operator< (rhs,lhs);}
//inline bool operator<=(const Point& lhs, const Point& rhs){return !operator> (lhs,rhs);}
//inline bool operator>=(const Point& lhs, const Point& rhs){return !operator< (lhs,rhs);}

// Direction vectors
Point v_x(1, 0, 0);
Point v_y(0, 1, 0);
Point v_z(0, 0, 1);

#endif