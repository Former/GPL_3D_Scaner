/***************************************************************************
 *   Copyright (C) 2007 by Marco Lorrai                                    *
 *   marco.lorrai@abbeynet.it                                              *
 *   Changed in 2011 by Alexei Bezborodov								   *
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

#include <set>
#include <linux/videodev2.h>
#include "errno.h"
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "pdp/pwc-ioctl.h"

#include "CameraDeviceV4L2.h"
#include "ccvt.h"

CameraDeviceV4L2::Buffer::Buffer()
{
	Start 	= MAP_FAILED;
	Length 	= 0;
}

CameraDeviceV4L2::CameraDeviceV4L2(size_t a_BufCount) 
{
	m_DeviceFileHandle 	= -1;
	m_AdjustColors 		= 0;
	m_Width 			= 0;
	m_Height 			= 0;
	m_PixelFormatID		= 0;
	m_BufCount			= a_BufCount;
}

CameraDeviceV4L2::~CameraDeviceV4L2() 
{
    CloseDevice();    
}

void CameraDeviceV4L2::UnMapBuffers()
{
	if (!IsBuffersMaped())
		return;
	
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	// Stop streaming I/O
    ioctl(m_DeviceFileHandle, VIDIOC_STREAMOFF, &type);
    
    for (size_t i = 0; i < m_Buffers.size(); ++i)
	{
		if (MAP_FAILED == m_Buffers[i].Start)
			munmap(m_Buffers[i].Start, m_Buffers[i].Length);
	}
	
	m_Buffers.clear();
}

void CameraDeviceV4L2::CloseDevice()
{
	UnMapBuffers();
	
    if (IsDeviceOpen())
	{
		close(m_DeviceFileHandle);
		m_DeviceFileHandle = -1;
	}
}

bool CameraDeviceV4L2::IsDeviceOpen()
{
	return (m_DeviceFileHandle < 0);
}

std::string CameraDeviceV4L2::GetLastError()
{
	return m_LastError;
}

void CameraDeviceV4L2::SetLastError(const std::string& a_Error)
{
	m_LastError = a_Error;
}

bool CameraDeviceV4L2::OpenDevice(std::string a_DeviceFileName)
{
    m_DeviceFileName = a_DeviceFileName;
	
    if (IsDeviceOpen())
        close(m_DeviceFileHandle);
    
    m_DeviceFileHandle = open(m_DeviceFileName.c_str(), O_RDWR);  
    if (m_DeviceFileHandle < 0)
	{
		std::string error = "Can`t open file " + m_DeviceFileName;
        SetLastError(error);
        return false;
    }
   
    if (!QueryCapabilities())
	{
        CloseDevice();
        SetLastError("Video2Linux2 not capability");
    }

    return true;
}

std::vector<CameraDeviceV4L2::PixelFormat> CameraDeviceV4L2::GetPixelFormats()
{
	std::vector<PixelFormat> result;
	
	struct v4l2_fmtdesc fmtDesc;
	memset(&fmtDesc, 0, sizeof(fmtDesc));
	fmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	for (size_t index = 0; ;index++)
	{
		fmtDesc.index = index;
		if (ioctl(m_DeviceFileHandle, VIDIOC_ENUM_FMT, &fmtDesc) == -1)
			break;
		
		if (!IsCompatibleFormat(fmtDesc.pixelformat))
			continue;
		
		PixelFormat item;
		
		item.Description 	= (char*)fmtDesc.description;
		item.FormatID 		= fmtDesc.pixelformat;
		
		result.push_back(item);
	}
	
	return result;
}

bool CameraDeviceV4L2::SetFormat(int a_Width, int a_Height, int a_PixelFormatID, size_t a_FrameRate)
{
	m_Width = a_Width;
	m_Height = a_Height;
	m_PixelFormatID = a_PixelFormatID;
	
	if (!IsCompatibleFormat(a_PixelFormatID))
	{
		SetLastError("Pixel format not compatible");
        return false;
	}
	
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
    
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = a_Width;
    fmt.fmt.pix.height      = a_Height;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.pixelformat = a_PixelFormatID;

    if (a_FrameRate)
	{
        fmt.fmt.pix.priv &= ~PWC_FPS_FRMASK;
        fmt.fmt.pix.priv |= (a_FrameRate << PWC_FPS_SHIFT);
    }

    if (ioctl(m_DeviceFileHandle, VIDIOC_S_FMT, &fmt) == -1)
	{
        SetLastError("Can`t set pixel format (VIDIOC_S_FMT)");
        return false;
    }
	
    UnMapBuffers();
    return MapBuffers();
}

bool CameraDeviceV4L2::MapBuffers()
{
    struct v4l2_requestbuffers reqBuff;
    memset(&reqBuff, 0, sizeof(reqBuff));
    
    reqBuff.count               = m_BufCount;
    reqBuff.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqBuff.memory              = V4L2_MEMORY_MMAP;
    
    if (ioctl(m_DeviceFileHandle, VIDIOC_REQBUFS, &reqBuff) == -1)
	{
		if (errno == EINVAL)
		{
			SetLastError(m_DeviceFileName + " does not support memory mapping");
			return false;
		}
		else
		{
			SetLastError("Error request buffers");
			return false;
		}
    }
    
    if (reqBuff.count < m_BufCount)
	{
		SetLastError("Insufficient buffer memory on " + m_DeviceFileName);
        return false;
    }
    
	m_Buffers.resize(reqBuff.count);
   
    for (size_t i = 0; i < m_Buffers.size(); ++i)
	{
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;

        if (ioctl(m_DeviceFileHandle, VIDIOC_QUERYBUF, &buf) == -1)
		{
            SetLastError("Error request buffer");
            UnMapBuffers();
			return false;
        }

        m_Buffers[i].Length = buf.length;
        m_Buffers[i].Start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_DeviceFileHandle, buf.m.offset);

        if (MAP_FAILED == m_Buffers[i].Start)
		{
            SetLastError("Can`t map buffer");
			UnMapBuffers();
            return false;
        }
    }
	
    for (size_t i = 0; i < m_Buffers.size(); ++i)
	{
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;
        
        if (ioctl(m_DeviceFileHandle, VIDIOC_QBUF, &buf) == -1)
		{
			SetLastError("Can`t map buffer");
			UnMapBuffers();
			return false;
        }
    }

	enum v4l2_buf_type buffType;
	buffType = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (ioctl(m_DeviceFileHandle, VIDIOC_STREAMON, &buffType) == -1)
	{
		SetLastError("Can`t start streaming I/O");
		UnMapBuffers();
		return false;
	}    

	return true;
}

void CameraDeviceV4L2::SetAdjustColors(bool a_AdjustColors)
{
	m_AdjustColors = a_AdjustColors;
}

bool CameraDeviceV4L2::IsBuffersMaped()
{
	return !m_Buffers.empty();
}

bool CameraDeviceV4L2::GetFrame(void* a_RGB_Buffer)
{
	if (!IsBuffersMaped())
	{
		SetLastError("Buffer not mapped");
		return false;
	}
    
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(m_DeviceFileHandle, VIDIOC_DQBUF, &buf) == -1)
	{
        switch (errno)
		{
            case EAGAIN:
                SetLastError("EAGAIN");
				return false;                
            case EIO:
			case EINVAL:
                /* Could ignore EIO, see spec. */                
                /* fall through */                
            default:
                perror("Can`t get buffer");
                return false;
        }
    }

	assert(buf.index < m_Buffers.size());

	unsigned char* a_Buffer = (unsigned char*)a_RGB_Buffer;
	const unsigned char* srcBuf = (const unsigned char*)m_Buffers[buf.index].Start;
	
    switch(m_PixelFormatID)
	{
	case V4L2_PIX_FMT_YUV420:
        if(m_AdjustColors)
            ccvt_420p_bgr24(m_Width, m_Height, srcBuf, a_Buffer);
        else
            ccvt_420p_rgb24(m_Width, m_Height, srcBuf, a_Buffer);
		break;

	case V4L2_PIX_FMT_UYVY:
        if(m_AdjustColors)
            ccvt_uyvy_bgr24(m_Width, m_Height, srcBuf, a_Buffer);
        else
            ccvt_uyvy_rgb24(m_Width, m_Height, srcBuf, a_Buffer);
		break;
		
	case V4L2_PIX_FMT_YUYV:
        if(m_AdjustColors)
            ccvt_yuyv_bgr24(m_Width, m_Height, srcBuf, a_Buffer);
        else
            ccvt_yuyv_rgb24(m_Width, m_Height, srcBuf, a_Buffer);
		break;

	case V4L2_PIX_FMT_SN9C10X:
		{
			std::vector <unsigned char> tmp_buffer(m_Width*m_Height*4);

			sonix_decompress(&tmp_buffer[0], srcBuf, m_Width, m_Height);
			bayer2rgb24(a_Buffer, &tmp_buffer[0], m_Width, m_Height); 
		}
		break;

	case V4L2_PIX_FMT_SBGGR8:
	case V4L2_PIX_FMT_SBGGR16:
	case V4L2_PIX_FMT_SGBRG8:
	case V4L2_PIX_FMT_SPCA561:
	case V4L2_PIX_FMT_SGRBG8:
        bayer2rgb24(a_Buffer, srcBuf, m_Width, m_Height); 
		break;

	case V4L2_PIX_FMT_BGR24:
//  @@@ ToDo:
//	case V4L2_PIX_FMT_SN9C10X:
//	case V4L2_PIX_FMT_SBGGR8:
//		if(!adjustColors)
//			convert2bgr();   
//		else
//			memcpy()
//		break;	

	default:
		assert(0); // BadFormat
	};
	
    if (ioctl(m_DeviceFileHandle, VIDIOC_QBUF, &buf) == -1)
	{
        SetLastError("can`t free buffer");
        return false;
    }
    
    return true;
}

bool CameraDeviceV4L2::IsCompatibleFormat(int a_FormatID)
{	
	int compatibleFormats[] = 
	{
		V4L2_PIX_FMT_YUV420, 
		V4L2_PIX_FMT_UYVY, 
		V4L2_PIX_FMT_YUYV,
		V4L2_PIX_FMT_SN9C10X, 
		V4L2_PIX_FMT_SBGGR8, 
		V4L2_PIX_FMT_SBGGR16, 
		V4L2_PIX_FMT_SGBRG8, 
		V4L2_PIX_FMT_SPCA561, 
		V4L2_PIX_FMT_SGRBG8
	};
	
	for (size_t i = 0; i < (sizeof(compatibleFormats) / sizeof(compatibleFormats[0])); i++)
	{
		if (a_FormatID == compatibleFormats[i])
			return true;
	}
	
	return false;
}

bool CameraDeviceV4L2::GetResolution(size_t* a_Width, size_t* a_Height, size_t* a_FrameRate)
{
	if (IsDeviceOpen())
	{
		SetLastError("Device not open");
		return false;
	}
	
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
	
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(m_DeviceFileHandle, VIDIOC_G_FMT, &fmt) == -1)
	{
        SetLastError("Can`t get format (VIDIOC_G_FMT)");
        return false;
    }
    
	*a_Width		= fmt.fmt.pix.width;
    *a_Height		= fmt.fmt.pix.height;
    *a_FrameRate	= (fmt.fmt.pix.priv & PWC_FPS_FRMASK) >> PWC_FPS_SHIFT;

    return true;
}

bool CameraDeviceV4L2::QueryCapabilities()
{
	v4l2_capability cap;
	if (ioctl (m_DeviceFileHandle, VIDIOC_QUERYCAP, &cap) == -1)
		return false;
    
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		return false;

	return true;
}

struct ResolutionForSet
{
	bool operator < (ResolutionForSet a_OtherRes) const;
	
	size_t Width;
	size_t Height;
};

bool ResolutionForSet::operator < (ResolutionForSet a_OtherRes) const
{
	// First in ResolutionSet must be bigger
	if (Width == a_OtherRes.Width)
		return Height > a_OtherRes.Height;
	return Width > a_OtherRes.Width;
}


std::vector<CameraDeviceV4L2::Resolution> GetAllPossibleResolutions()
{ 
	typedef std::set<ResolutionForSet> ResolutionSet;
	ResolutionSet resultSet;
	
	double possileScale[] = {1.5, 2.0};
	double possileAspectRatio[] = {4.0/3.0, 16.0/9.0};
	
	for (size_t scaleIndex = 0; scaleIndex < sizeof(possileScale)/sizeof(possileScale[0]); scaleIndex++)
	{
		for (size_t aspectIndex = 0; aspectIndex < sizeof(possileAspectRatio)/sizeof(possileAspectRatio[0]); aspectIndex++)
		{
			size_t width = 160;
			size_t height = 120;
			
			while (width < 5000)
			{
				ResolutionForSet res;
				
				res.Width = width;
				res.Height = height;
				resultSet.insert(res);
				
				height = height * possileScale[scaleIndex];
				width = height * possileAspectRatio[aspectIndex];
			}
		}
	}
	
	std::vector<CameraDeviceV4L2::Resolution> result;
	for (ResolutionSet::iterator it = resultSet.begin(); it != resultSet.end(); ++it)
	{
		CameraDeviceV4L2::Resolution res;
		
		res.Width = it->Width;
		res.Height = it->Height;
		result.push_back(res);
	}
	
	return result;
}

std::vector<CameraDeviceV4L2::Resolution> CameraDeviceV4L2::GetResolutions(int a_PixelFormatID)
{
	std::vector<Resolution> result;
	std::vector<Resolution> allPossible = GetAllPossibleResolutions();
	
	for (size_t i = 0; i < allPossible.size(); i++)
	{
		struct v4l2_format fmt;
		
		memset(&fmt, 0, sizeof(struct v4l2_format));
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		fmt.fmt.pix.width = allPossible[i].Width;
		fmt.fmt.pix.height = allPossible[i].Height;
		fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
		fmt.fmt.pix.pixelformat = a_PixelFormatID;

		if (-1 == ioctl(m_DeviceFileHandle, VIDIOC_TRY_FMT, &fmt))
			continue;
		
		if ((fmt.fmt.pix.width == allPossible[i].Width) && (fmt.fmt.pix.height == allPossible[i].Height)) 
			result.push_back(allPossible[i]);

	}

	return result;
}

