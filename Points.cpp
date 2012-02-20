#include "Points.h"
#include <math.h>

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

const Point2D Point2D::operator + (const Point2D& a_Value) const
{
	return Point2D(x + a_Value.x, y + a_Value.y);
}

const Point2D Point2D::operator - (const Point2D& a_Value) const
{
	return Point2D(x - a_Value.x, y - a_Value.y);
}

double Point2D::operator * (const Point2D& a_Value) const
{
	return (x * a_Value.x + y * a_Value.y);
}

const Point2D Point2D::operator + (const double& a_Value) const
{
	return Point2D(x + a_Value, y + a_Value);
}

const Point2D Point2D::operator - (const double& a_Value) const
{
	return Point2D(x - a_Value, y - a_Value);
}

const Point2D Point2D::operator * (const double& a_Value) const
{
	return Point2D(x * a_Value, y * a_Value);
}

const Point2D Point2D::operator / (const double& a_Value) const
{
	return Point2D(x / a_Value, y / a_Value);
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

const Point3D Point3D::operator + (const Point3D& a_Value) const
{
	return Point3D(x + a_Value.x, y + a_Value.y, z + a_Value.z);
}

const Point3D Point3D::operator - (const Point3D& a_Value) const
{
	return Point3D(x - a_Value.x, y - a_Value.y, z - a_Value.z);
}

double Point3D::operator * (const Point3D& a_Value) const
{
	return (x * a_Value.x + y * a_Value.y + z * a_Value.z);
}

const Point3D Point3D::operator + (const double& a_Value) const
{
	return Point3D(x + a_Value, y + a_Value, z + a_Value);
}

const Point3D Point3D::operator - (const double& a_Value) const
{
	return Point3D(x - a_Value, y - a_Value, z - a_Value);
}

const Point3D Point3D::operator * (const double& a_Value) const
{
	return Point3D(x * a_Value, y * a_Value, z * a_Value);
}

const Point3D Point3D::operator / (const double& a_Value) const
{
	return Point3D(x / a_Value, y / a_Value, z / a_Value);
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
