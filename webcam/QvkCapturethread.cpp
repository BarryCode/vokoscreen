/*
 * Copyright (c) Özkan Pakdil <ozkan.pakdil@gmail.com>
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "QvkCapturethread.h"

#define CLEAR(x)

void CaptureThread::xioctl(int fh, int request, void *arg) {
	QFile file( dev_name );
	if ( not file.exists() )
	{
	  devam = false;
	  quit();
	  return;  
	}

	int r;
	do {
             r = v4l2_ioctl(fh, request, arg);
	} while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

	if (r == -1) {
		qDebug() << "[vokoscreen] CaptureThread::xioctl error " << errno << " " << strerror(errno);
		return;
	}
}


CaptureThread::CaptureThread()
{
  running = false;
}


// process video data
void CaptureThread::run() {
	while (devam) {
		mutex.lock();
		do {
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select(fd + 1, &fds, NULL, NULL, &tv);
		} while ((r == -1 && (errno = EINTR)));

		if (r == -1) {
			qDebug() << "[vokoscreen] CaptureThread::run select";
			quit();
			return;
		}

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		xioctl(fd, VIDIOC_DQBUF, &buf);

		if (v4lconvert_convert(v4lconvert_data,
					 &src_fmt,
					 &fmt,
					 (unsigned char*)buffers[buf.index].start, buf.bytesused,
					 dst_buf, fmt.fmt.pix.sizeimage) < 0) {
			if (errno != EAGAIN)
				qDebug() << "[vokoscreen] CaptureThread::run v4l_convert";
		}

		unsigned char* asil=(unsigned char*)malloc(fmt.fmt.pix.sizeimage+qstrlen(header));
		memmove(asil, dst_buf, fmt.fmt.pix.sizeimage);
		memmove(asil+qstrlen(header), asil, fmt.fmt.pix.sizeimage);
		memcpy(asil,header,qstrlen(header));

		QImage myImage;
		
		if( myImage.loadFromData( asil,fmt.fmt.pix.sizeimage+qstrlen( header ) ) )
                  emit newPicture( myImage ); 

		free(asil);
//		if (delay>0) {
//			this->msleep(delay);
//		}
		xioctl(fd, VIDIOC_QBUF, &buf);
		di++;
		mutex.unlock();
	}
}

int CaptureThread::stop()
{
  running = false;
  devam = false;
  mutex.lock();
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl( fd, VIDIOC_STREAMOFF, &type );
    for ( unsigned int i = 0; i < n_buffers; ++i )
      v4l2_munmap( buffers[i].start, buffers[i].length );

    v4l2_close( fd );
    fd = -1;
  mutex.unlock();
  quit();
  return 0;
}
  

QString CaptureThread::getNameFromDevice( QString device )
{
	char dummy[256];
	QString dev_name = device;
	int fd = -1;

	fd = v4l2_open(dev_name.toStdString().c_str(), O_RDWR | O_NONBLOCK, 0);
	
	qDebug();
	if ( -1 == fd )
	{
		fprintf( stderr,"[vokoscreen] CaptureThread::getNameFromDevice open %s: %s\n", dev_name.toStdString().c_str(), strerror( errno ) );
		exit(1);
	};

	if ( -1 != v4l2_ioctl( fd,VIDIOC_QUERYCAP, dummy ) )
	{
		//printf( "[vokoscreen] v4l2 device info [%s]\n", dev_name.toStdString().c_str() );
		qDebug() << "[vokoscreen] v4l2 device info " << dev_name.toStdString().c_str();
		qDebug( " " );
	} else
	  {
		fprintf( stderr, "[vokoscreen] CaptureThread::getNameFromDevice %s: not an video4linux device\n", dev_name.toStdString().c_str() );
		exit( 1 );
          }
	
	struct v4l2_capability capability;
	memset( &capability, 0, sizeof( capability ) );
	if ( -1 == v4l2_ioctl( fd, VIDIOC_QUERYCAP, &capability ) )
	{
          v4l2_close( fd );
          return "-1";
	}
	QString str( ( char * ) capability.card );
        
        v4l2_close( fd );
	
	return str;
}


bool CaptureThread::busy( QString device )
{
        QString dev_name = device;
        int fd = -1;

	// vergeben eines Filedescriptor 
	fd = v4l2_open(dev_name.toStdString().c_str(), O_RDWR | O_NONBLOCK, 0);
	if ( fd < 0 )
	{
          qDebug() << "[vokoscreen] CaptureThread::busy cannot open device";
          quit();
	}

	//CLEAR( fmt );
	struct v4l2_format fmt;// neu
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 640;
	fmt.fmt.pix.height      = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	
	int r;
	do
	{
          r = v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt);
	}
	while ( r == -1 && ( ( errno == EINTR) || (errno == EAGAIN ) ) );

	v4l2_close( fd );

	if ( r == -1 )
          return true;
	else
	  return false;
}


int CaptureThread::start( QString device )
{
	wait();

	devam=false;
	fd = -1;

	// read config
	dev_name = device;
	width    = 640;
	height   = 480;
//	fps      = 25;
	
//	if ( fps > 0 )
//          delay = 1000/fps;
//	else 
//	  delay = 0;

	// open webcam device node
	fd = v4l2_open(dev_name.toStdString().c_str(), O_RDWR | O_NONBLOCK, 0);
//	fd = v4l2_open( dev_name.toStdString().c_str(), O_RDONLY, 0 );

	if (fd < 0) {
		qDebug() << "[vokoscreen] CaptureThread::start cannot open device";
		quit();
		return 1;
	}

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	xioctl(fd, VIDIOC_S_FMT, &fmt);
	if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
		qDebug() << "[vokoscreen] CaptureThread::start Libv4l didn't accept RGB24 format. Can't proceed.";
		quit();
		return 1;
	}
	//emit startedCapture(fmt.fmt.pix.width, fmt.fmt.pix.height);

	v4lconvert_data = v4lconvert_create(fd);
	if (v4lconvert_data == NULL)
		qDebug() << "[vokoscreen] CaptureThread::start v4lconvert_create";
	if (v4lconvert_try_format(v4lconvert_data, &fmt, &src_fmt) != 0)
		qDebug() << "[vokoscreen] CaptureThread::start v4lconvert_try_format";
	xioctl(fd, VIDIOC_S_FMT, &src_fmt);
	dst_buf = (unsigned char*)malloc(fmt.fmt.pix.sizeimage);

	CLEAR(req);
	req.count = 2;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	xioctl(fd, VIDIOC_REQBUFS, &req);

	buffers = (buffer*)calloc(req.count, sizeof(*buffers));
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		CLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;
		xioctl(fd, VIDIOC_QUERYBUF, &buf);

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = v4l2_mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start) {
			qDebug() << "[vokoscreen] CaptureThread::start mmap";
			quit();
			return 1;
		}
	}

	for (unsigned int i = 0; i < n_buffers; ++i) {
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		xioctl(fd, VIDIOC_QBUF, &buf);
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	xioctl(fd, VIDIOC_STREAMON, &type);

	di=0;
	sprintf(header,"P6\n%d %d 255\n",fmt.fmt.pix.width,fmt.fmt.pix.height);
	devam=true;

	// start processing video data
	running = true;
	QThread::start();
	return 0;
}
