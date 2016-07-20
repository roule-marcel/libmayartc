/*
 * webrtcpp.cpp
 *
 *  Created on: 14 juil. 2016
 *      Author: jfellus
 */


#include "webrtcpp.h"
#include "api/RTCPeer.h"
#include "api/SignalingServer.h"

#define DEFAULT_PORT 10000

#define NB_MAX_CHANNEL_FDS 1024
#define NB_MAX_STREAMS_IN_FDS 1024
#define NB_MAX_STREAMS_OUT_FDS 1024

using namespace webrtcpp;


static SignalingWebSocketServer *server = NULL;


static int nbChannelsFd = 0;
static int nbStreamsInFd = 0;
static int nbStreamsOutFd = 0;
static RTCDataChannel* channels_fd[NB_MAX_CHANNEL_FDS];
static RTCVideoStreamIn* streams_in_fd[NB_MAX_STREAMS_IN_FDS];
static RTCVideoStreamOut* streams_out_fd[NB_MAX_STREAMS_OUT_FDS];


static void webrtcpp_init(int port = DEFAULT_PORT) {
	if(!server) {
		server = new SignalingWebSocketServer(port);
		server->start();
	}
}


////////////////////////////
// WEBRTCPP_DATA CHANNELS //
////////////////////////////

int webrtcpp_create(const char* channel) {
	webrtcpp_init();
	channels_fd[nbChannelsFd] = server->addDataChannel(channel);
	return nbChannelsFd++;
}

void webrtcpp_add_callback(int fd, webrtc_data_callback cb) {
	if(fd >= nbChannelsFd || !channels_fd[fd]) throw "No such channel";
	channels_fd[fd]->addCallback(cb);
}

size_t webrtcpp_write(int fd, void* data, size_t size) {
	if(fd >= nbChannelsFd || !channels_fd[fd]) throw "No such channel";
	return channels_fd[fd]->write((char*)data, size);
}

void webrtcpp_close(int fd) {
	if(fd >= nbChannelsFd || !channels_fd[fd]) throw "No such channel";
	delete channels_fd[fd];
	channels_fd[fd] = NULL;
}


////////////////////////////
// WEBRTCPP_VIDEO STREAMS //
////////////////////////////

int webrtcpp_video_in_create(const char* channel, int mode, uint32_t w, uint32_t h) {
	webrtcpp_init();
	streams_in_fd[nbStreamsInFd] = server->addVideoInStream(channel, w ,h);
	return nbStreamsInFd++;
}

void webrtcpp_video_in_add_callback(int fd, webrtc_stream_callback cb) {
	if(fd >= nbStreamsInFd || !streams_in_fd[fd]) throw "No such video stream";
	streams_in_fd[fd]->addCallback(cb);
}

void webrtcpp_video_in_close(int fd) {
	if(fd >= nbStreamsInFd || !streams_in_fd[fd]) throw "No such video stream";
	delete streams_in_fd[fd];
	streams_in_fd[fd] = NULL;
}


int webrtcpp_video_out_create(const char* channel, int mode, uint32_t w, uint32_t h) {
	webrtcpp_init();
	streams_out_fd[nbStreamsOutFd] = server->addVideoOutStream(channel, w, h);
	return nbStreamsOutFd++;
}

size_t webrtcpp_video_out_write(int fd, unsigned char* rgb) {
	if(fd >= nbStreamsOutFd || !streams_out_fd[fd]) throw "No such video stream";
	return streams_out_fd[fd]->write(rgb);
}

void webrtcpp_video_out_close(int fd) {
	if(fd >= nbStreamsOutFd || !streams_out_fd[fd]) throw "No such video stream";
	delete streams_out_fd[fd];
	streams_out_fd[fd] = NULL;
}





