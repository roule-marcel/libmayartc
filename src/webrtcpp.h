/*
 * webrtcpp.h
 *
 *  Created on: 14 juil. 2016
 *      Author: jfellus
 */

#ifndef SRC_WEBRTCPP_H_
#define SRC_WEBRTCPP_H_


#define WEBRTCPP_INPUT 0
#define WEBRTCPP_OUTPUT 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>



/**
 * @param int mode : {WEBRTCPP_INPUT | WEBRTCPP_OUTPUT}
 * @return a fd
 * */
int webrtcpp_open(const char* channel, int mode);
size_t webrtcpp_read(int fd, void* data, size_t maxsize);
size_t webrtcpp_write(int fd, void* data, size_t size);
void webrtcpp_close(int fd);



int webrtcpp_video_in_open(const char* channel, int mode, uint32_t w, uint32_t h);
size_t webrtcpp_video_in_read(int fd, unsigned char* rgb);
void webrtcpp_video_in_close(int fd);

int webrtcpp_video_out_open(const char* channel, int mode, uint32_t w, uint32_t h);
size_t webrtcpp_video_out_write(int fd, unsigned char* rgb);
void webrtcpp_video_out_close(int fd);




#endif /* SRC_WEBRTCPP_H_ */

