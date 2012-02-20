#pragma once

class Point2D
{	
public:
	Point2D();
	Point2D(double a_x, double a_y);

	const Point2D 	operator + (const Point2D& a_Value) const;
	const Point2D 	operator - (const Point2D& a_Value) const;
	double 			operator * (const Point2D& a_Value) const;

	const Point2D 	operator + (const double& a_Value) const;
	const Point2D 	operator - (const double& a_Value) const;
	const Point2D 	operator * (const double& a_Value) const;
	const Point2D 	operator / (const double& a_Value) const;
	
	double length() const;
	double length2() const; // length * length
	
	double x;
	double y;
};

class Point3D
{
public:
	Point3D();
	Point3D(double a_x, double a_y, double a_z);
	
	const Point3D 	operator + (const Point3D& a_Value) const;
	const Point3D 	operator - (const Point3D& a_Value) const;
	double 			operator * (const Point3D& a_Value) const;

	const Point3D 	operator + (const double& a_Value) const;
	const Point3D 	operator - (const double& a_Value) const;
	const Point3D 	operator * (const double& a_Value) const;
	const Point3D 	operator / (const double& a_Value) const;
	
	double length() const;
	double length2() const; // length * length
	
	double x;
	double y;
	double z;
};
