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
#include "ccvt.h"

using namespace std;

CameraDeviceV4L2::CameraDeviceV4L2() 
{
    isOpen=false;
    adjustColors=false;
	
	isMapped=false;
	canMapDevice=true;
	video_palette[0] = VIDEO_PALETTE_RGB24;
	video_palette[1] = VIDEO_PALETTE_RGB32;
	video_palette[2] = VIDEO_PALETTE_YUV420P;
	video_palette[3] = VIDEO_PALETTE_YUV420;
	video_palette[4] = VIDEO_PALETTE_YUYV;
	video_palette[5] = VIDEO_PALETTE_JPEG;
	video_palette[6] = VIDEO_PALETTE_MJPEG;
	map=NULL;
	
    dev_name = "dev/video0";
    buffers = NULL;
    n_buffers = 0;
    controlsSetted = false;
    pixelFormat = 0;
}

CameraDeviceV4L2::~CameraDeviceV4L2() 
{
    closeDevice();
    
    if (isOpen)
        close(fd); 
}

void CameraDeviceV4L2::closeDevice()
{
    unsigned int i;
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl (fd, VIDIOC_STREAMOFF, &type) == -1)
        perror("VIDIOC_STREAMOFF");
    
     for (i = 0; i < n_buffers; ++i)
         if (munmap (buffers[i].start, buffers[i].length) == -1)
             perror("munmap");
    
    free(buffers);
	
	if ( isOpen ) {
        close( fd );
        isOpen=false;
    }
}

bool CameraDeviceV4L2::openDevice(unsigned int a_Width, unsigned int a_Height, int a_Format) 
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
    
      //  struct v4l2_capability cap;    
     //   if (queryCapabilities(&cap))
	//	{
   //         cout<<"Using video4linux 2 API"<<endl;
  //          v4l = new VideoForLinux2();
 //           isV4L2 = true;
//        }
//        else
//		{
 //           v4l = new VideoForLinux1();
  //          isV4L2 = false;
  //      }
   // }

    mMap(a_Width, a_Height);
    return true;
}

void CameraDeviceV4L2::setAdjustColors(bool b)
{
	adjustColors = b;
}


bool CameraDeviceV4L2::init_mmap()
{
    struct v4l2_requestbuffers req;
    enum v4l2_buf_type type;

    memset(&req, 0, sizeof(struct v4l2_requestbuffers));
    
    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;
    
    if (ioctl (fd, VIDIOC_REQBUFS, &req) == -1) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s does not support "
                    "memory mapping\n", dev_name.c_str());
            return false;
        } 
        else {
            perror("VIDIOC_REQBUFS");
            return false;
        }
    }
    
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n",
                dev_name.c_str());
        return false;
    }
    
    buffers = (buffer*)calloc (req.count, sizeof (*buffers));
    if (!buffers) { 
        fprintf (stderr, "Out of memory\n");
        return false;
    }
    
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        memset(&buf, 0, sizeof(struct v4l2_buffer));

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        if (ioctl (fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("VIDIOC_QUERYBUF");
            return false;
        }

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
                mmap (NULL /* start anywhere */,
                      buf.length,
                      PROT_READ | PROT_WRITE /* required */,
                      MAP_SHARED /* recommended */,
                      fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            perror("mmap");
            return false;
        }
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;
        
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("VIDIOC_QBUF");
            return false;
        }
    }
        
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("VIDIOC_STREAMON");
        return false;
    }    
    
    isMapped = true;
    return true;
}

bool CameraDeviceV4L2::mMap(int w, int h, int frameRate)
{
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;
            
    if(!controlsSetted)
        setControls();
    
    memset(&fmt, 0, sizeof(struct v4l2_format));
    memset(&cropcap, 0, sizeof(struct v4l2_cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    
    if (!ioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */
        
        if (ioctl(fd, VIDIOC_S_CROP, &crop) == -1) {
            switch (errno) {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    }
    else {
                /* Errors ignored. */
    } 

    
    if(frameFormat==VIDEO_PALETTE_RGB24) {
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24; 
        pixelFormat = V4L2_PIX_FMT_BGR24;
        std::cout<<"Using V4L2_PIX_FMT_BGR24 pixel format"<<std::endl;
    }
    else if(frameFormat==VIDEO_PALETTE_YUYV) {
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        pixelFormat = V4L2_PIX_FMT_YUYV;
        std::cout<<"Using V4L2_PIX_FMT_YUYV pixel format"<<std::endl;
    }
    else if(frameFormat==VIDEO_PALETTE_YUV420P) {
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
        pixelFormat = V4L2_PIX_FMT_YUV420;
        std::cout<<"Using V4L2_PIX_FMT_YUV420 pixel format"<<std::endl;
    }
    else if(frameFormat==VIDEO_PALETTE_JPEG) {
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
        pixelFormat = V4L2_PIX_FMT_JPEG;
        std::cout<<"Using V4L2_PIX_FMT_JPEG pixel format"<<std::endl;
    }
    else if(frameFormat==VIDEO_PALETTE_MJPEG) {
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
        pixelFormat = V4L2_PIX_FMT_MJPEG;
        std::cout<<"Using V4L2_PIX_FMT_MJPEG pixel format"<<std::endl;
    }
    else { //AUTO or YUV420
        struct v4l2_fmtdesc argp;
        memset(&argp, 0, sizeof(struct v4l2_fmtdesc));
        argp.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        std::cout<<"Determining pixel format..."<<std::endl;
        for(int index=0; index < 16; index++) {            
            argp.index=index;
            if (ioctl(fd, VIDIOC_ENUM_FMT, &argp) == -1) {
                //perror("VIDIOC_ENUM_FMT");
                break;
            }
            else {
                std::string description = (char*)argp.description;
                std::cout<<"pixel format: "<<description<<std::endl;
                if(argp.pixelformat == V4L2_PIX_FMT_YUV420) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
                    pixelFormat = V4L2_PIX_FMT_YUV420;
                    std::cout<<"Found V4L2_PIX_FMT_YUV420 pixel format"<<std::endl;
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_YUYV) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
                    pixelFormat = V4L2_PIX_FMT_YUYV;
                    std::cout<<"Found V4L2_PIX_FMT_YUYV pixel format"<<std::endl;
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_JPEG ) 
				{                    
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
                    pixelFormat = V4L2_PIX_FMT_JPEG;
                    std::cout<<"Found V4L2_PIX_FMT_JPEG pixel format"<<std::endl;                     
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_MJPEG) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
                    pixelFormat = V4L2_PIX_FMT_MJPEG;
                    std::cout<<"Found V4L2_PIX_FMT_MJPEG pixel format"<<std::endl;                     
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_SN9C10X) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SN9C10X;
                    pixelFormat = V4L2_PIX_FMT_SN9C10X;
                    std::cout<<"Found V4L2_PIX_FMT_SN9C10X pixel format"<<std::endl;                    
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_SBGGR8) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR8;
                    pixelFormat = V4L2_PIX_FMT_SBGGR8;
                    std::cout<<"Found V4L2_PIX_FMT_SBGGR8 pixel format"<<std::endl;                    
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_SBGGR16) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR16;
                    pixelFormat = V4L2_PIX_FMT_SBGGR16;
                    std::cout<<"Found V4L2_PIX_FMT_SBGGR16 pixel format"<<std::endl;                    
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_SGBRG8) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SGBRG8;
                    pixelFormat = V4L2_PIX_FMT_SGBRG8;
                    std::cout<<"Found V4L2_PIX_FMT_SGBRG8 pixel format"<<std::endl;                    
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_SGRBG8) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SGRBG8;
                    pixelFormat = V4L2_PIX_FMT_SGRBG8;
                    std::cout<<"Found V4L2_PIX_FMT_SGRBG8 pixel format"<<std::endl;                    
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_SPCA561) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SPCA561;
                    pixelFormat = V4L2_PIX_FMT_SPCA561;
                    std::cout<<"Found V4L2_PIX_FMT_SPCA561 pixel format"<<std::endl;                    
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_BGR24) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
                    pixelFormat = V4L2_PIX_FMT_BGR24;
                    std::cout<<"Found V4L2_PIX_FMT_BGR24 pixel format"<<std::endl;
                }
                else if(argp.pixelformat == V4L2_PIX_FMT_UYVY) {
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
                    pixelFormat = V4L2_PIX_FMT_UYVY;
                    std::cout<<"Found V4L2_PIX_FMT_UYVY pixel format"<<std::endl;
                }
				break;
            }                
        }
    }
    
    if(!pixelFormat) {
        std::cout<<"Could not determine pixel format or pixel format not yet supported"<<std::endl;
        return false;
    }    
        
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = w;
    fmt.fmt.pix.height      = h;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    //setting frame rate
    if(frameRate) {
        fmt.fmt.pix.priv &= ~PWC_FPS_FRMASK;
        fmt.fmt.pix.priv |= ( frameRate << PWC_FPS_SHIFT );
    }

    if (isMapped) {
        closeDevice();
        close(fd);

        std::string device = "/dev/video0";
        fd = open(device.c_str(), O_RDWR); // open video device
    }

    if (ioctl (fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("VIDIOC_S_FMT");
        return false;
    }
    
    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    width = w;
    height = h;
    
    return init_mmap();
}

std::vector<unsigned char> CameraDeviceV4L2::getFrame()
{
   unsigned int i;
    
    struct v4l2_buffer buf;
    
    if(!isMapped) {
        std::cout<<"Device not mapped"<<std::endl;
        return std::vector<unsigned char>();
    }
    
    memset(&buf, 0, sizeof(struct v4l2_buffer));
    
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    //buf.index  = n_buffers;

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        switch (errno) {
            case EAGAIN:
                return std::vector<unsigned char>();
                
            case EIO:
                /* Could ignore EIO, see spec. */
                
                /* fall through */
                
            default:
                perror("VIDIOC_DQBUF");
                return std::vector<unsigned char>();
        }
    }

    assert(buf.index < n_buffers);       

	m_Buffer.resize(width * height * 3);
	unsigned char* a_Buffer = &m_Buffer[0];

    if(pixelFormat == V4L2_PIX_FMT_YUV420)
	{
        if(adjustColors)
            ccvt_420p_bgr24( width, height, ( const unsigned char* ) buffers[buf.index].start, ( unsigned char* ) a_Buffer );
        else
            ccvt_420p_rgb24( width, height, ( const unsigned char* ) buffers[buf.index].start, ( unsigned char* ) a_Buffer );             
    }
    else if(pixelFormat == V4L2_PIX_FMT_UYVY)
	{
        if(adjustColors)
            ccvt_uyvy_bgr24( width, height, ( const unsigned char* ) buffers[buf.index].start, ( unsigned char* ) a_Buffer );
        else
            ccvt_uyvy_rgb24( width, height, ( const unsigned char* ) buffers[buf.index].start, ( unsigned char* ) a_Buffer );             
    }
    else if(pixelFormat == V4L2_PIX_FMT_YUYV)
	{
        if(adjustColors)
            ccvt_yuyv_bgr24( width, height, ( const unsigned char* ) buffers[buf.index].start, ( unsigned char* ) a_Buffer );
        else
            ccvt_yuyv_rgb24( width, height, ( const unsigned char* ) buffers[buf.index].start, ( unsigned char* ) a_Buffer );
    }
    else if(pixelFormat == V4L2_PIX_FMT_MJPEG || pixelFormat == V4L2_PIX_FMT_JPEG)
	{
        //std::cout<<"Using mjpeg..."<<std::endl;
        if (buf.bytesused <= HEADERFRAME1) {
            // Prevent crash on empty image
            std::cout << "Ignoring empty buffer ..." <<std::endl;
        }
        else {
            unsigned char *tmp_buffer = (unsigned char *)malloc(width*height*3/2);
  //          mjpegtoyuv420p(( const unsigned char* ) buffers[buf.index].start, tmp_buffer, width, height, buf.length);
            if(adjustColors)
                ccvt_420p_bgr24( width, height, tmp_buffer, ( unsigned char* ) a_Buffer );
            else
                ccvt_420p_rgb24( width, height, tmp_buffer, ( unsigned char* ) a_Buffer );
            free (tmp_buffer);
        }
    }
    else if(pixelFormat == V4L2_PIX_FMT_SN9C10X)
	{  
        unsigned char *tmp_buffer = (unsigned char *)malloc(width*height*4);
        sonix_decompress(tmp_buffer, ( unsigned char* ) buffers[buf.index].start, width, height);
        bayer2rgb24((unsigned char*)a_Buffer, tmp_buffer, width, height); 
        free(tmp_buffer);
    }
    else if(pixelFormat == V4L2_PIX_FMT_SBGGR8 || 
		pixelFormat == V4L2_PIX_FMT_SBGGR16 || 
		pixelFormat == V4L2_PIX_FMT_SGBRG8 ||
		pixelFormat == V4L2_PIX_FMT_SPCA561 || 
		pixelFormat ==  V4L2_PIX_FMT_SGRBG8)
	{         
        bayer2rgb24((unsigned char*)a_Buffer, ( unsigned char* ) buffers[buf.index].start, width, height); 
    }
    else if(pixelFormat == V4L2_PIX_FMT_BGR24 || pixelFormat == V4L2_PIX_FMT_SN9C10X
            || pixelFormat == V4L2_PIX_FMT_SBGGR8)
	{
 //       if(!adjustColors)
 //           convert2bgr( (unsigned char*)*buffer, width*height*3);   
    }
    else
	{
        std::cout<<"Unknown pixel format"<<std::endl;
        return std::vector<unsigned char>();
    }   
    
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
	{
        perror("VIDIOC_QBUF");
        return std::vector<unsigned char>();
    }
    
    return m_Buffer;
}

bool CameraDeviceV4L2::setResolution(unsigned int width, unsigned int height, unsigned int frameRate)
{
	return mMap(width, height, frameRate);
}

bool CameraDeviceV4L2::getResolution(unsigned int &width, unsigned int &height, unsigned int &frameRate)
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(struct v4l2_format));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    frameRate=0; /*ignored*/
    if (ioctl (fd, VIDIOC_G_FMT, &fmt) == -1) {
        perror("VIDIOC_G_FMT");
        return false;
    }
    width = fmt.fmt.pix.width;
    height = fmt.fmt.pix.height;
    frameRate = ( fmt.fmt.pix.priv & PWC_FPS_FRMASK ) >> PWC_FPS_SHIFT;
    printf("width: %d, height %d\n", width, height);
    return true;
}

void CameraDeviceV4L2::setControls()
{
    struct v4l2_queryctrl queryctrl;

    controlsSetted = true;
    //enumerateControls();
    
    /*BRIGHTNESS*/
    memset (&queryctrl, 0, sizeof (queryctrl));
    queryctrl.id = V4L2_CID_BRIGHTNESS;

    if (ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl) ==-1) {
        if (errno != EINVAL) {
                perror ("VIDIOC_QUERYCTRL");
                //return false;
        } 
        else {
            printf ("V4L2_CID_BRIGHTNESS is not supported\n");
        }
    } 
    else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        printf ("V4L2_CID_BRIGHTNESS is not supported\n");
    }    
    
    controls.minbrightness = queryctrl.minimum;
    controls.maxbrightness = queryctrl.maximum;
    
    /*CONTRAST*/
    memset (&queryctrl, 0, sizeof (queryctrl));
    queryctrl.id = V4L2_CID_CONTRAST;

    if (ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl) ==-1) {
        if (errno != EINVAL) {
                perror ("VIDIOC_QUERYCTRL");
                //return false;
        } 
        else {
            printf ("V4L2_CID_CONTRAST is not supported\n");
        }
    } 
    else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        printf ("V4L2_CID_CONTRAST is not supported\n");
    } 
    
    controls.mincontrast = queryctrl.minimum;
    controls.maxcontrast = queryctrl.maximum;
    
    /*GAMMA*/
    memset (&queryctrl, 0, sizeof (queryctrl));
    queryctrl.id = V4L2_CID_GAMMA;

    if (ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl) ==-1) {
        if (errno != EINVAL) {
                perror ("VIDIOC_QUERYCTRL");
                //return false;
        } 
        else {
            printf ("V4L2_CID_GAMMA is not supported\n");
        }
    } 
    else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        printf ("V4L2_CID_GAMMA is not supported\n");
    } 
    
    controls.mingamma = queryctrl.minimum;
    controls.maxgamma = queryctrl.maximum;
    
    /*SATURATION*/
    memset (&queryctrl, 0, sizeof (queryctrl));
    queryctrl.id = V4L2_CID_SATURATION;

    if (ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl) ==-1) {
        if (errno != EINVAL) {
                perror ("VIDIOC_QUERYCTRL");
                //return false;
        } 
        else {
            printf ("V4L2_CID_SATURATION is not supported\n");
        }
    } 
    else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
        printf ("V4L2_CID_SATURATION is not supported\n");
    } 
    
    controls.minsaturation = queryctrl.minimum;
    controls.maxsaturation = queryctrl.maximum;
    
}

bool CameraDeviceV4L2::queryCapabilities(struct v4l2_capability *cap) /*VIDIOC_QUERYCAP*/
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

bool CameraDeviceV4L2::getResolutionList(wxArrayString &validResolution) 
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
            fmt.fmt.pix.pixelformat = pixelFormat;

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
            fmt.fmt.pix.pixelformat = pixelFormat;

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
            fmt.fmt.pix.pixelformat = pixelFormat;

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
            fmt.fmt.pix.pixelformat = pixelFormat;

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

