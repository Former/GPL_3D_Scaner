#pragma once
#include <vector>

namespace Parser3D
{
	//////////////////////////////////////////////////////////////////////////

	typedef unsigned char ColorUnit;
	
	struct RGB
	{
		RGB();
		RGB(ColorUnit a_Red, ColorUnit a_Green, ColorUnit a_Blue);
		
		ColorUnit GetGray() const;
		
		ColorUnit red;		
		ColorUnit green;
		ColorUnit blue;		
	};

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
		CameraCollibrator();
		
		void Initialize(size_t a_Width, size_t a_Height);

		size_t GetWidth() const;
		size_t GetHeight() const;

		Point3D CalculatePointer3D(Point2D a_Point1, Point2D a_Point2, double a_RealDistance);

	protected:
		size_t m_Width;
		size_t m_Height;
	};

	//////////////////////////////////////////////////////////////////////////

	class ImageParser
	{
	public:
		typedef std::vector<ColorUnit> PixelLine;		

	public:
		ImageParser(CameraCollibrator* a_Collibrator);

		// To Private
		std::vector<PixelLine> Prepare(RGB* a_RGB_Buffer);
		
		// Из линии пикселей, выделяет наиболее яркие учас их положение.
		std::vector<double> PapseLine(const PixelLine& a_Line);

		std::vector<Point3D> Parse(RGB* a_RGB_Buffer, double a_RealDistance);

	protected:
		CameraCollibrator* 	m_Collibrator;		
		ColorUnit 			m_MaxColorUnit;
	};
}
