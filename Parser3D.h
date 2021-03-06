#pragma once
#include <vector>
#include "Points.h"

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

	//////////////////////////////////////////////////////////////////////////
	
	// Calculate from pixel -> real 3D pointers 
	class CameraCollibrator
	{
	public:
		CameraCollibrator();
		
		void Initialize(size_t a_Width, size_t a_Height);

		size_t GetWidth() const;
		size_t GetHeight() const;

		virtual Point3D CalculatePointer3D(Point2D a_Point1, Point2D a_Point2) = 0;

	protected:
		size_t m_Width;
		size_t m_Height;
	};

	// Простой каллибратор, не дает большой точности, но легко настраивается.
	class SimpleCameraCollibrator : public CameraCollibrator
	{
	public:
		SimpleCameraCollibrator(double a_VerticalTanAngle, double a_HorisontalTanAngle, double a_RealBeamDistance);
		
		virtual Point3D CalculatePointer3D(Point2D a_Point1, Point2D a_Point2);

	protected:
		double m_VerticalTanAngle;     // Угол от центральной оси камеры до края видимости по вртикали
		double m_HorisontalTanAngle;	// Угол от центральной оси камеры до края видимости по горизонтали
		double m_RealBeamDistance; 	// Расстояние в пикселях между лучами лазера на расстоянии 1 метра от камеры
	};

	
	//////////////////////////////////////////////////////////////////////////

	class ImageParser
	{
	public:
		typedef std::vector<ColorUnit> PixelLine;		

	public:
		ImageParser(CameraCollibrator* a_Collibrator);

		// To Private
		static std::vector<PixelLine> Prepare(RGB* a_RGB_Buffer);
		
		// Из линии пикселей, выделяет наиболее яркие учас их положение.
		std::vector<double> PapseLine(const PixelLine& a_Line);

		std::vector<Point3D> Parse(RGB* a_RGB_Buffer);

	protected:
		static PixelLine MergeLines(const PixelLine* a_Lines, const size_t a_LinesCount);

		CameraCollibrator* 	m_Collibrator;		
		ColorUnit 			m_MaxColorUnit;
	};
}
