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



typedef void(*webrtc_data_callback)(const char* buf, size_t size);
typedef void(*webrtc_stream_callback)(const uint8_t* rgb, uint32_t w, uint32_t h);


/**
 * @param int mode : {WEBRTCPP_INPUT | WEBRTCPP_OUTPUT}
 * @return a fd
 * */
int webrtcpp_create(const char* channel);
void webrtcpp_add_callback(int fd, webrtc_data_callback cb);
size_t webrtcpp_write(int fd, void* data, size_t size);
void webrtcpp_close(int fd);



int webrtcpp_video_in_create(const char* channel, int mode, uint32_t w, uint32_t h);
void webrtcpp_video_in_add_callback(int fd, webrtc_stream_callback cb);
void webrtcpp_video_in_close(int fd);

int webrtcpp_video_out_create(const char* channel, int mode, uint32_t w, uint32_t h);
size_t webrtcpp_video_out_write(int fd, unsigned char* rgb);
void webrtcpp_video_out_close(int fd);




#endif /* SRC_WEBRTCPP_H_ */

