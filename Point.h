#ifndef Point_h
#define  Point_h

#include <cmath>

#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__); Serial.flush(); //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__); Serial.flush();   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif


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
	
	Point operator+(const Point &RHS) { return Point(*this) += RHS; };
	Point operator-(const Point &RHS) { return Point(*this) -= RHS; };
	
	// Scalar addition and subtraction
	Point& operator+=(const double &RHS) { x += RHS; y += RHS; z += RHS; return *this; };
	Point& operator-=(const double &RHS) { x -= RHS; y -= RHS; z -= RHS; return *this; };

	Point operator+(const double &RHS) { return Point(*this) += RHS; };
	Point operator-(const double &RHS) { return Point(*this) -= RHS; };
	
	// Scalar product and division
	Point& operator*=(const double &RHS) { x *= RHS; y *= RHS; z *= RHS; return *this; };
	Point& operator/=(const double &RHS) { x /= RHS; y /= RHS; z /= RHS; return *this; };

	Point operator*(const double &RHS) { return Point(*this) *= RHS; };
	Point operator/(const double &RHS) { return Point(*this) /= RHS; };
	
	// Negation
	Point operator-() {return Point(-x, -y, -z); };
	
	// Euclidean norm
	double norm() { 
		return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)); 
	};
	

	// Distance to other point
	double distance(const Point& other)	{
		return std::sqrt(std::pow(other.x-x, 2) + std::pow(other.y-y, 2) + std::pow(other.z-z, 2)); 
	};
};
#endif