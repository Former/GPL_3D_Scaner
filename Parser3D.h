#pragma once

namespace Parser3D
{
	//////////////////////////////////////////////////////////////////////////

	class Point3D
	{
		Point3D();

		double x;
		double y;
		double z;
	};

	class Point2D
	{
		Point2D();

		double x;
		double y;
	};

	//////////////////////////////////////////////////////////////////////////

	class CameraCollibrator
	{
	public:
		CameraCollibrator(size_t a_Width, size_t a_Height);

		size_t GetWidth() const;
		size_t GetHeight() const;

		Point3D CalculatePointer3D(Point2D a_Point1, Point2D a_Point2, double a_RealDistance) = 0;

	protected:
		size_t m_Width;
		size_t m_Height;
	};

	//////////////////////////////////////////////////////////////////////////

	class ImageParser
	{
	public:
		ImageParser(CameraCollibrator* a_Collibrator);

		std::vector<Point3D> Parse(void* a_RGB_Buffer, double a_RealDistance);

	protected:
		CameraCollibrator* m_Collibrator;
	};
}
