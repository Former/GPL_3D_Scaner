/***************************************************************************
 *   Copyright (C) 2007 by Marco Lorrai                                    *
 *   marco.lorrai@abbeynet.it                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#ifdef HAVE_CAMV4L 
#include <linux/videodev.h>
#else
#include <libv4l1-videodev.h>
#endif
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "pdp/pwc-ioctl.h"
#include <wx/arrstr.h>
#include <unistd.h>
#include <vector>
#include <string>

class CameraDeviceV4L2
{
public:
	struct PixelFormat
	{
		int FormatID;
		std::string Description;
	};

	CameraDeviceV4L2();
	~CameraDeviceV4L2();
	
	bool OpenDevice(std::string a_DeviceFileName);
	void CloseDevice();
	bool IsDeviceOpen();
	
	std::vector<PixelFormat> GetPixelFormats();
	bool SetFormat(int a_Width, int a_Height, int a_PixelFormatID, size_t a_FrameRate = 0);
	bool GetResolution(size_t* a_Width, size_t* a_Height, size_t* a_FrameRate);
	bool GetFrame(void* a_RGB_Buffer);

	std::string GetLastError();

	void SetAdjustColors(bool a_AdjustColors);
	bool GetResolutionList(wxArrayString &validResolution);

private:
	bool IsBuffersMaped();
	bool MapBuffers();
	void UnMapBuffers();
	
	bool QueryCapabilities();
	bool IsCompatibleFormat(int a_FormatID);

	void SetLastError(const std::string& a_Error);

	struct Buffer 
	{
		Buffer()
		{
			Start 	= MAP_FAILED;
			Length 	= 0;
		}
		
		void*	Start; 
		size_t	Length;
	};
	
	int 		m_PixelFormatID;
	bool 		m_AdjustColors;
	std::string	m_DeviceFileName;
	int 		m_DeviceFileHandle;
	size_t 		m_Width;
	size_t 		m_Height;
	std::vector<Buffer> m_Buffers;

	std::string	m_LastError;	
};

