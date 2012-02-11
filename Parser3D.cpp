#include "Parser3D.h"

namespace Parser3D
{
	RGB::RGB()
	{
		red		= 0;
		green	= 0;
		blue	= 0;
	}
	
	RGB::RGB(ColorUnit a_Red, ColorUnit a_Green, ColorUnit a_Blue)
	{
		red		= a_Red;
		green	= a_Green;
		blue	= a_Blue;
	}	
	
	ColorUnit RGB::GetGray() const
	{
		return (red + green + blue) / 3;
	}	
	
	CameraCollibrator::CameraCollibrator()
	{
		m_Width		= 0;
		m_Height	= 0;
	}
	
	void CameraCollibrator::Initialize(size_t a_Width, size_t a_Height)
	{
		m_Width		= a_Width;
		m_Height	= a_Height;
	}
	
	size_t CameraCollibrator::GetWidth() const
	{
		return m_Width;
	}
	
	size_t CameraCollibrator::GetHeight() const
	{
		return m_Height;
	}

	Point3D CameraCollibrator::CalculatePointer3D(Point2D a_Point1, Point2D a_Point2, double a_RealDistance)
	{
		
	}

	//////////////////////////////////////////////////////////////////////////

	ImageParser::ImageParser(CameraCollibrator* a_Collibrator)
	{
		m_Collibrator = a_Collibrator;
	}

	std::vector<ImageParser::PixelLine> ImageParser::Prepare(RGB* a_RGB_Buffer)
	{
		const size_t width = m_Collibrator->GetWidth();
		const size_t height = m_Collibrator->GetHeight();
		std::vector<PixelLine> result;
		
		m_MaxColorUnit = 0;
		for (size_t x = 0; x < width; x++)
		{
			PixelLine line;
			for (size_t y = 0; y < height; y++)
			{
				const RGB& color = *(a_RGB_Buffer + x + y * width);
				
				ColorUnit unit = color.red;
				if (unit > m_MaxColorUnit)
					m_MaxColorUnit = unit;
				
				line.push_back(unit);
			}
			result.push_back(line);
		}	
		
		return result;
	}
	
	struct Area
	{
		size_t start;
		size_t stop;
	};
	
	// Из линии пикселей, выделяет наиболее яркие учас их положение.
	std::vector<double> ImageParser::PapseLine(const PixelLine& a_Line)
	{
		// Находим максимум
		ColorUnit max = 0;
		for (size_t i = 0; i < a_Line.size(); i++)
		{
			if( a_Line[i] > max)
				max = a_Line[i];
		}
		max = 0.75 * max + 0.25 * m_MaxColorUnit;
		
		// Выделяем все области где величина больше половины от максимума
		std::vector <Area> araes;
		const size_t maxHole = 2;
		const double boundary = 0.75;
		
		for (size_t i = 0; i < a_Line.size(); i++)
		{
			if (a_Line[i] < max * boundary)
				continue;
			
			Area curArea;
			curArea.start = i; 
	
			size_t nextIter;
			size_t holePixelCount = 0;
			for (nextIter = i; nextIter < a_Line.size(); nextIter++)
			{
				if (a_Line[nextIter] > max * boundary)
				{
					holePixelCount = 0;	
					continue;
				}
				
				if (holePixelCount > maxHole)
					break;

				holePixelCount++;
			}
			
			i = nextIter;
			curArea.stop = nextIter - holePixelCount; 
			
			araes.push_back(curArea);
		}

		// Вычисляем центр областей
		std::vector<double> result;
		
		for (size_t areaIndex = 0; areaIndex < araes.size(); areaIndex++)
		{
			const Area& curArea = araes[areaIndex];
			
			double summUnit = 0;
			double summMultiple = 0;
			for (size_t i = curArea.start; i < curArea.stop; i++)
			{
				const double unit = static_cast<double>(a_Line[i]);
				
				summMultiple += unit * i;
				summUnit += unit;
			}
			
			double center = summMultiple / summUnit;			
		//	result.push_back((curArea.start + curArea.stop) / 2.0); // center
		//	result.push_back(curArea.start);
			result.push_back(center); 
		}
		
		return result;
	}

	std::vector<Point3D> ImageParser::Parse(RGB* a_RGB_Buffer, double a_RealDistance)
	{
		
	}
}