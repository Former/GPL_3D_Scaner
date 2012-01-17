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

#ifndef _device_H
#define	_device_H

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
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>

#define HEADERFRAME1 0xaf

struct buffer {
    void* start; 
    size_t length;
};

#define VIDEO_PALETTE_JPEG 21
#define VIDEO_PALETTE_MJPEG 22

const unsigned int PALETTE_NUMBER = 7;

struct controls {
    int minbrightness, maxbrightness;
    int mincontrast, maxcontrast;
    int mingamma, maxgamma;
    int minsaturation, maxsaturation;
};

class CameraDeviceV4L2
{
    public:
        CameraDeviceV4L2();
        ~CameraDeviceV4L2();
        
        void closeDevice();
        bool openDevice(unsigned int a_Width, unsigned int a_Height, int a_Format);

        std::vector<unsigned char> getFrame();
        bool setResolution(unsigned int width, unsigned int height, unsigned int frameRate=0);
        bool getResolution(unsigned int &width, unsigned int &height, unsigned int &frameRate);

        bool queryCapabilities(struct v4l2_capability *cap); /*VIDIOC_QUERYCAPP*/
        bool getResolutionList(wxArrayString &validResolution);
        bool isPWC();
        void setAdjustColors(bool);
		
		bool mMap(int w, int h, int frameRate=0);
        
    private:
        std::string numpal2string(int num);
		
		bool init_mmap();
		void setControls();		
    
    private:
        static int gain;
        static int shutter;
        bool adjustColors;
        bool isV4L2;
        int frameFormat;
        bool isOpen;
		int fd;
        struct video_mmap v;
	
	private:    
		bool controlsSetted;
		std::string dev_name;
		struct buffer* buffers;
		unsigned int n_buffers;
		int pixelFormat;
		unsigned int width, height;
		struct controls controls;
			
		std::vector<unsigned char> m_Buffer;
		
		int video_palette[PALETTE_NUMBER];
		bool isMapped;
		bool canMapDevice;
		int mmap_size;
		int siz;
		void *map;        
};

#endif	/* _device_H */
