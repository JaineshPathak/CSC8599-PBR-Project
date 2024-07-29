#pragma once
/*
Class:Vector3
Implements:
Author:Rich Davison
Description:VERY simple Vector3 class. Students are encouraged to modify this as necessary!

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*/
#include <cmath>
#include <iostream>
#include "common.h"

class Vector3	{
public:
	Vector3(void) {
		ToZero();
	}

	Vector3(const float x, const float y, const float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	~Vector3(void){}

	float x;
	float y;
	float z;

	Vector3 Normalised() const {
		Vector3 n = *this;
		n.Normalise();
		return n;
	}

	void			Normalise() {
		float length = Length();

		if(length != 0.0f)	{
			length = 1.0f / length;
			x = x * length;
			y = y * length;
			z = z * length;
		}
	}

	void		ToZero() {
		x = y = z = 0.0f;
	}

	float			Length() const {
		return sqrt((x*x)+(y*y)+(z*z));	
	}

	void			Invert() {
		x = -x;
		y = -y;	
		z = -z;	
	}

	Vector3			Inverse() const{
		return Vector3(-x,-y,-z);
	}

	static float	Dot(const Vector3 &a, const Vector3 &b) {
		return (a.x*b.x)+(a.y*b.y)+(a.z*b.z);
	}

	static Vector3	Cross(const Vector3 &a, const Vector3 &b) {
		return Vector3((a.y*b.z) - (a.z*b.y) , (a.z*b.x) - (a.x*b.z) , (a.x*b.y) - (a.y*b.x));	
	}

	static Vector3 Lerp(const Vector3& a, const Vector3& b, float t)
	{
		Vector3 finalV;

		finalV.x = naive_lerp(a.x, b.x, t);
		finalV.y = naive_lerp(a.y, b.y, t);
		finalV.z = naive_lerp(a.z, b.z, t);

		return finalV;
	}

	static Vector3 Slerp(const Vector3& _start, const Vector3& _end, float t)
	{
		float dot = Dot(_start, _end);
		dot = Clamp(dot, -1.0f, 1.0f);

		float angle = std::acos(dot) * t;
		Vector3 relativeVec = _end - _start * dot;
		relativeVec.Normalise();

		Vector3 startFactor = _start * std::cos(angle);
		Vector3 endFactor = relativeVec * std::sin(angle);

		return startFactor + endFactor;
	}

	static float Distance(const Vector3& p1, const Vector3& p2)
	{
		float diffY = p1.y - p2.y;
		float diffX = p1.x - p2.x;
		return sqrt((diffY * diffY) + (diffX * diffX));
	}

	inline friend std::ostream& operator<<(std::ostream& o, const Vector3& v) {
		o << "Vector3(" << v.x << "," << v.y << "," << v.z <<")" << std::endl;
		return o;
	}

	inline Vector3  operator+(const Vector3  &a) const{
		return Vector3(x + a.x,y + a.y, z + a.z);
	}

	inline Vector3  operator-(const Vector3  &a) const{
		return Vector3(x - a.x,y - a.y, z - a.z);
	}

	inline Vector3  operator-() const{
		return Vector3(-x,-y,-z);
	}

	inline void operator+=(const Vector3  &a){
		x += a.x;
		y += a.y;
		z += a.z;
	}

	inline void operator-=(const Vector3  &a){
		x -= a.x;
		y -= a.y;
		z -= a.z;
	}

	inline Vector3  operator*(const float a) const{
		return Vector3(x * a,y * a, z * a);
	}

	inline Vector3  operator*(const Vector3  &a) const{
		return Vector3(x * a.x,y * a.y, z * a.z);
	}

	inline Vector3  operator/(const Vector3  &a) const{
		return Vector3(x / a.x,y / a.y, z / a.z);
	};

	inline Vector3  operator/(const float v) const{
		return Vector3(x / v,y / v, z / v);
	};

	inline bool	operator==(const Vector3 &A)const {return (A.x == x && A.y == y && A.z == z) ? true : false;};
	inline bool	operator!=(const Vector3 &A)const {return (A.x == x && A.y == y && A.z == z) ? false : true;};

	inline float GetMaxElement() const 
	{
		float v = x;
		if (y > v)
			v = y;
		if (z > v)
			v = z;

		return v;
	}

	inline float GetAbsMaxElement() const 
	{
		float v = abs(x);
		if (abs(y) > v) {
			v = abs(y);
		}
		if (abs(z) > v) {
			v = abs(z);
		}
		return v;
	}

	static const Vector3 ZERO;		//Vector3(0, 0, 0);
	static const Vector3 UP;		//Vector3(0, 1, 0);
	static const Vector3 DOWN;		//Vector3(0, -1, 0);
	static const Vector3 FORWARD;	//Vector3(0, 0, -1);
	static const Vector3 BACK;		//Vector3(0, 0, 1);
	static const Vector3 LEFT;		//Vector3(-1, 0, 0);
	static const Vector3 RIGHT;		//Vector3(1, 0, 0);
};