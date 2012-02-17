#include "Points.h"

Point2D::Point2D()
{
	x = 0.0;
	y = 0.0;
}

Point2D::Point2D(double a_x, double a_y)
{
	x = a_x;
	y = a_y;
}

const Point2D& Point2D::operator + (const Point2D& a_Value1, const Point2D& a_Value2) const
{
	return Point2D(a_Value1.x + a_Value2.x, a_Value1.y + a_Value2.y);
}

const Point2D& Point2D::operator - (const Point2D& a_Value1, const Point2D& a_Value2) const
{
	return Point2D(a_Value1.x - a_Value2.x, a_Value1.y - a_Value2.y);
}

double Point2D::operator * (const Point2D& a_Value1, const Point2D& a_Value2) const
{
	return (a_Value1.x * a_Value2.x + a_Value1.y * a_Value2.y);
}

const Point2D& Point2D::operator + (const double& a_Value) const
{
	return Point2D(a_Value1.x + a_Value, a_Value1.y + a_Value);
}

const Point2D& Point2D::operator - (const double& a_Value) const
{
	return Point2D(a_Value1.x - a_Value, a_Value1.y - a_Value);
}

const Point2D& Point2D::operator * (const double& a_Value) const
{
	return Point2D(a_Value1.x * a_Value, a_Value1.y * a_Value);
}

const Point2D& Point2D::operator / (const double& a_Value) const
{
	return Point2D(a_Value1.x / a_Value, a_Value1.y / a_Value);
}

double Point2D::length() const
{
	return sqrt(length2());
}

double Point2D::length2() const
{
	const Point2D& thisPoint = *this;
	return (thisPoint * thisPoint);
}

//////////////////////////////////////////////////////////////////////////

Point3D::Point3D()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

Point3D::Point3D(double a_x, double a_y, double a_z)
{
	x = a_x;
	y = a_y;
	z = a_z;
}

const Point3D& Point3D::operator + (const Point3D& a_Value1, const Point3D& a_Value2) const
{
	return Point3D(a_Value1.x + a_Value2.x, a_Value1.y + a_Value2.y, a_Value1.z + a_Value2.z);
}

const Point3D& Point3D::operator - (const Point3D& a_Value1, const Point3D& a_Value2) const
{
	return Point3D(a_Value1.x - a_Value2.x, a_Value1.y - a_Value2.y, a_Value1.z - a_Value2.z);
}

double Point3D::operator * (const Point3D& a_Value1, const Point3D& a_Value2) const
{
	return (a_Value1.x * a_Value2.x + a_Value1.y * a_Value2.y + a_Value1.z * a_Value2.z);
}

const Point3D& Point3D::operator + (const double& a_Value) const
{
	return Point3D(a_Value1.x + a_Value, a_Value1.y + a_Value, a_Value1.z + a_Value);
}

const Point3D& Point3D::operator - (const double& a_Value) const
{
	return Point3D(a_Value1.x - a_Value, a_Value1.y - a_Value, a_Value1.z - a_Value);
}

const Point3D& Point3D::operator * (const double& a_Value) const
{
	return Point3D(a_Value1.x * a_Value, a_Value1.y * a_Value, a_Value1.z * a_Value);
}

const Point3D& Point3D::operator / (const double& a_Value) const
{
	return Point3D(a_Value1.x / a_Value, a_Value1.y / a_Value, a_Value1.z / a_Value);
}

double Point3D::length() const
{
	return sqrt(length2());
}

double Point3D::length2() const
{
	const Point3D& thisPoint = *this;
	return (thisPoint * thisPoint);
}
