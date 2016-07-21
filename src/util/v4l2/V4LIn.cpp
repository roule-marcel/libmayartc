/*
 * main.cpp
 *
 *  Created on: 5 août 2014
 *      Author: jfellus
 */


#include "V4LIn.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "utils.h"
#include <pthread.h>

void* readframe_async(void* p);



V4LIn::V4LIn(const char* devname) {
	fd = -1; buffers = NULL; n_buffers = 0; w = h = 0; data = tmpdata = NULL; bStarted = false;
	sem_init(&sem, 0,0);
	pthread_mutex_init(&mut, NULL);
	open_device(devname);
}

V4LIn::~V4LIn() {
	if(bStarted) stop();
	for (unsigned int i = 0; i < n_buffers; i++)
		if (-1 == munmap(buffers[i].start, buffers[i].length)) errno_exit("munmap");
	delete[] buffers;
	delete[] data;
	delete[] tmpdata;
	if (close(fd) == -1) errno_exit("close");
	fd = -1;
}

void V4LIn::start() {
	for(unsigned int i = 0; i < n_buffers; i++) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) errno_exit("VIDIOC_QBUF");
	}
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMON, &type)) errno_exit("VIDIOC_STREAMON");

	bStarted = true;
	pthread_create(&th_async, NULL, readframe_async, this);
}

void V4LIn::stop() {
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) errno_exit("VIDIOC_STREAMOFF");
	bStarted = false;
}

void V4LIn::readFrame() {
	sem_wait(&sem);
	pthread_mutex_lock(&mut);
	pthread_mutex_unlock(&mut);
}

void V4LIn::_readFrame_async() {
	struct v4l2_buffer buf;
	for(;;) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		struct timeval tv = {2,0};
		int r = select(fd + 1, &fds, NULL, NULL, &tv);
		if (r == -1) {	if (EINTR == errno)	continue; errno_exit("select");	}
		else if (r == 0) { fprintf(stderr, "select timeout\n"); 	exit(EXIT_FAILURE);	}

		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
			if(errno == EAGAIN) continue;
			if(errno != EIO) errno_exit("VIDIOC_DQBUF");
		} else break;
	}

	assert(buf.index < n_buffers);

	pthread_mutex_lock(&mut);
	if(!data) {data = new unsigned char[w*h*3]; tmpdata = new unsigned char[w*h*3];}
	yuyv2rgb(buffers[buf.index].start, buf.bytesused, tmpdata);
	unsigned char* tmp = tmpdata;
	tmpdata = data;
	data = tmp;
	pthread_mutex_unlock(&mut);
	sem_post(&sem);


	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf)) errno_exit("VIDIOC_QBUF");
}

void V4LIn::open_device(const char* dev_name) {
	struct stat st;
	if (-1 == stat(dev_name, &st)) {	fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno)); exit(EXIT_FAILURE); }
	if (!S_ISCHR(st.st_mode)) { fprintf(stderr, "%s is no device\n", dev_name); exit(EXIT_FAILURE);	}

	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (-1 == fd) {			fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno)); exit(EXIT_FAILURE); }

	struct v4l2_capability cap;
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) errno_exit("VIDIOC_QUERYCAP");
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) { fprintf(stderr, "%s is no video capture device\n", dev_name); exit(EXIT_FAILURE); }
	if (!(cap.capabilities & V4L2_CAP_STREAMING)) { fprintf(stderr, "%s does not support streaming i/o\n", dev_name); exit(EXIT_FAILURE); }

	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt)) errno_exit("VIDIOC_G_FMT");
	unsigned int min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)	fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min) fmt.fmt.pix.sizeimage = min;

	w = fmt.fmt.pix.width;
	h = fmt.fmt.pix.height;
	if(!data) {data = new unsigned char[w*h*3]; tmpdata = new unsigned char[w*h*3];}

	// MMAP buffers

	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) { if (errno == EINVAL) fprintf(stderr, "%s does not support memory mapping\n", dev_name); errno_exit("VIDIOC_REQBUFS");	}
	if (req.count < 2) {fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name); exit(EXIT_FAILURE); }

	buffers = new buffer[req.count];
	if (!buffers)  { fprintf(stderr, "Out of memory\n"); exit(EXIT_FAILURE); }

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if (buffers[n_buffers].start == MAP_FAILED)	errno_exit("mmap");
	}
}


void* readframe_async(void* p) {
	V4LIn* v = (V4LIn*)p;
	while(v->bStarted) {
		v->_readFrame_async();
	}
	return 0;
}



