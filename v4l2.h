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

#ifndef _VideoForLinux2_H
#define	_VideoForLinux2_H

#include <unistd.h>
#include <vector>
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

#include <string>

#define HEADERFRAME1 0xaf

struct buffer {
    void* start; 
    size_t length;
};

#define VIDEO_PALETTE_JPEG 21
#define VIDEO_PALETTE_MJPEG 22

const unsigned int PALETTE_NUMBER = 7;

enum video_std {
    MODE_PAL,
    MODE_NTSC,
    MODE_SECAM,
    NONE
};

struct controls {
    int minbrightness, maxbrightness;
    int mincontrast, maxcontrast;
    int mingamma, maxgamma;
    int minsaturation, maxsaturation;
};

class VideoForLinux2
{   
public:    
    VideoForLinux2();

    ~VideoForLinux2();
    
    void closeDevice();
    bool mMap(int w, int h, int frameRate=0);
    std::vector<unsigned char> getFrame();
    bool setResolution(unsigned int width, unsigned int height, unsigned int frameRate=0);
    bool getResolution(unsigned int &width, unsigned int &height, unsigned int &frameRate);
   
    void setParameters(int fd, int format)
    {
        this->fd=fd;
        frameFormat=format;
        this->adjustColors=adjustColors;
    }
    
    void setAdjustColors(bool adjustColors)
    {
        this->adjustColors=adjustColors;
    }

    int getPixelFormat() {return pixelFormat;}
        
private:    
    bool init_mmap();
    void setControls();
    
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
    int fd;
    bool isMapped;
    bool canMapDevice;
    int mmap_size;
    int frameFormat;
    int siz;
    bool adjustColors;   
    void *map;        
};

#endif //_VideoForLinux2_H
