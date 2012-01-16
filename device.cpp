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

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <linux/videodev2.h>
#include "errno.h"
#include "device.h"
#include "v4l2.h"

using namespace std;

Device::Device() 
{
    isOpen=false;
    adjustColors=false;
}

Device::~Device() 
{
    if (v4l)
        delete v4l;
    if (isOpen)
        close(fd); 
}

bool Device::openDevice(unsigned int a_Width, unsigned int a_Height, int a_Format) 
{
    frameFormat = a_Format;
    v.frame = 0;
    
    if (isOpen)
        close(fd);
    
	// open video device
    std::string device = "/dev/video0";
    fd=open( device.c_str(), O_RDWR );  
    if (fd < 0)
	{
        isOpen = false;
        perror(device.c_str());
        return false;
    }
    
    isOpen = true;
    
    std::string driver = "Video4Linux2";
	if(driver == "Video4Linux2")
	{
        v4l = new VideoForLinux2();
        isV4L2 = true;
    }
    else
	{
        struct v4l2_capability cap;    
        if (queryCapabilities(&cap))
		{
            cout<<"Using video4linux 2 API"<<endl;
            v4l = new VideoForLinux2();
            isV4L2 = true;
        }
//        else
//		{
 //           v4l = new VideoForLinux1();
  //          isV4L2 = false;
  //      }
    }

    v4l->setParameters(fd, a_Format);
    mMap(a_Width, a_Height);
    return true;
}

void Device::setAdjustColors(bool b)
{
    if(v4l)
        v4l->setAdjustColors(b);    
}

bool Device::mMap(int w, int h)
{
    if(v4l)
        return v4l->mMap(w, h, 0);
    else
        return false;
}

std::vector<unsigned char> Device::getFrame()
{
    if(v4l)
        return v4l->getFrame();
    else
        return std::vector<unsigned char>();
}

bool Device::setResolution(unsigned int width, unsigned int height, unsigned int frameRate)
{
    if(v4l)
        return v4l->setResolution(width, height, frameRate);
    else
        return false;
}

bool Device::getResolution(unsigned int &width, unsigned int &height, unsigned int &frameRate)
{
    if(v4l)
        return v4l->getResolution(width, height, frameRate);
    else
        return false;
}

bool Device::queryCapabilities(struct v4l2_capability *cap) /*VIDIOC_QUERYCAP*/
{
    std::string dev_name = "dev/video0"; //Setting::GetInstance()->GetDeviceFile();
    if (ioctl (fd, VIDIOC_QUERYCAP, cap) == -1)
	{
        if (EINVAL == errno)
		{
            fprintf (stderr, "%s is not a V4L2 device\n",
                     dev_name.c_str());
            return false;
        } 
        else
		{
            perror("VIDIOC_QUERYCAP");
            return false;
        }
    }
    
    if (!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf (stderr, "%s is no video capture device\n",
                dev_name.c_str());
        return false;
    }
    return true;
}

bool Device::getResolutionList(wxArrayString &validResolution) 
{
 {
        int resw, resh;        
        struct v4l2_format fmt;

        std::stringstream ss;
        std::string str;

        resw = 160;
        resh = 120;        

        while(resw < 3000) {
            memset(&fmt, 0, sizeof(struct v4l2_format));
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            fmt.fmt.pix.width = resw;
            fmt.fmt.pix.height = resh;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
            fmt.fmt.pix.pixelformat = static_cast<VideoForLinux2*>(v4l)->getPixelFormat();

            if (ioctl (fd, VIDIOC_TRY_FMT, &fmt) == -1) {
                perror("VIDIOC_TRY_FMT");
                printf("Resolution %dx%d not valid\n", resw, resh);
            } else {
                if ((fmt.fmt.pix.width == resw) && (fmt.fmt.pix.height == resh)) {
                    printf("Resolution %dx%d valid\n", fmt.fmt.pix.width, fmt.fmt.pix.height);                    
                    ss.str("");
                    ss << resw << "x" << resh;
                    str = ss.str();
                    validResolution.Add(wxString(str.c_str(), wxConvUTF8));
                }
            }            
            resh = resh * 1.5;
            resw = resh * 4 / (float)3;
        }

        resw = 160;
        resh = 120;

        while(resw < 3000) {
            memset(&fmt, 0, sizeof(struct v4l2_format));
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            fmt.fmt.pix.width = resw;
            fmt.fmt.pix.height = resh;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
            fmt.fmt.pix.pixelformat = static_cast<VideoForLinux2*>(v4l)->getPixelFormat();

            if (ioctl (fd, VIDIOC_TRY_FMT, &fmt) == -1) {
                perror("VIDIOC_TRY_FMT");
                printf("Resolution %dx%d not valid\n", resw, resh);
            } else {
                if ((fmt.fmt.pix.width == resw) && (fmt.fmt.pix.height == resh)) {
                    printf("Resolution %dx%d valid\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
                    ss.str("");
                    ss << resw << "x" << resh;
                    str = ss.str();
                    validResolution.Add(wxString(str.c_str(), wxConvUTF8));
                }
            }
            resh = resh * 2;
            resw = resh * 4 / (float)3;
        }


        resw = 160;
        resh = 90;

        while(resw < 3000) {
            memset(&fmt, 0, sizeof(struct v4l2_format));
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            fmt.fmt.pix.width = resw;
            fmt.fmt.pix.height = resh;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
            fmt.fmt.pix.pixelformat = static_cast<VideoForLinux2*>(v4l)->getPixelFormat();

            if (ioctl (fd, VIDIOC_TRY_FMT, &fmt) == -1) {
                perror("VIDIOC_TRY_FMT");
                printf("Resolution %dx%d not valid\n", resw, resh);
            } else {
                if ((fmt.fmt.pix.width == resw) && (fmt.fmt.pix.height == resh)) {
                    printf("Resolution %dx%d valid\n", fmt.fmt.pix.width, fmt.fmt.pix.height);                    
                    ss.str("");
                    ss << resw << "x" << resh;
                    str = ss.str();
                    validResolution.Add(wxString(str.c_str(), wxConvUTF8));
                }
            }
            resh = resh * 1.5;
            resw = resh * 16 / (float)9;
        }

        resw = 160;
        resh = 90;

        while(resw < 3000) {
            memset(&fmt, 0, sizeof(struct v4l2_format));
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            fmt.fmt.pix.width = resw;
            fmt.fmt.pix.height = resh;
            fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
            fmt.fmt.pix.pixelformat = static_cast<VideoForLinux2*>(v4l)->getPixelFormat();

            if (ioctl (fd, VIDIOC_TRY_FMT, &fmt) == -1) {
                perror("VIDIOC_TRY_FMT");
                printf("Resolution %dx%d not valid\n", resw, resh);
            } else {
                if ((fmt.fmt.pix.width == resw) && (fmt.fmt.pix.height == resh)) {
                    printf("Resolution %dx%d valid\n", fmt.fmt.pix.width, fmt.fmt.pix.height);
                    ss.str("");
                    ss << resw << "x" << resh;
                    str = ss.str();
                    validResolution.Add(wxString(str.c_str(), wxConvUTF8));
                }
            }
            resh = resh * 2;
            resw = resh * 16 / (float)9;
        }
    }
}

void Device::closeDevice() 
{
    if ( isOpen ) {
        close( fd );
        isOpen=false;
    }
}
