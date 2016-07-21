/*
 * utils.h
 *
 *  Created on: 5 août 2014
 *      Author: jfellus
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>


struct buffer { void   *start; size_t  length;};

static void errno_exit(const char *s) {
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg) {
        int r;
        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);
        return r;
}

static void yuv2rgb(int Y, int U, int V, unsigned char *rgb) {
   int c = Y-16, d = U - 128, e = V - 128;
   int r = (298 * c           + 409 * e + 128) >> 8;
   int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
   int b = (298 * c + 516 * d           + 128) >> 8;
   if (r > 255) r = 255;   if (g > 255) g = 255;   if (b > 255) b = 255;
   if (r < 0) r = 0;       if (g < 0) g = 0;       if (b < 0) b = 0;
   *(rgb++) = r;   *(rgb++) = g;  *(rgb)   = b;
}

static void yuyv2rgb(const void *img, int size, unsigned char* buf) {
	unsigned char* p = (unsigned char*)img;
	int y,u,y2,v;
	for(unsigned char* rgb=buf; size>0; size-=4) {
	    y=(int)*p++; u=(int)*p++; y2 = (int)*p++; v = (int)*p++;
	    yuv2rgb(y, u, v, rgb); rgb+=3;
	    yuv2rgb(y2, u, v, rgb); rgb+=3;
	}
}

#endif /* UTILS_H_ */
