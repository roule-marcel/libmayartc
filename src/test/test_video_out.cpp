#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include "../util/v4l2/V4LIn.h"

#include <webrtcpp.h>

// We use a simple video4linux camera input
V4LIn* v4lin = NULL;

// Handle SIGINT for graceful exit
static void signal_handler(int sig){  if(v4lin) delete v4lin; exit(0); }
static void init() {
	struct sigaction sigact;
	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);
}


// video_out webrtc channels require BGR image format...
static void convert_to_bgr(uint8_t* rgb, uint8_t* bgr, size_t w, size_t h) {
	for(int i=0; i<w*h; i++) {
		bgr[i*3] = rgb[i*3+2];
		bgr[i*3+1] = rgb[i*3+1];
		bgr[i*3+2] = rgb[i*3];
	}
}


int main(void){
	init();

	// Creates a video_out channel called 'camera'
	int fd = webrtcpp_video_out_create("camera", 0, 640, 480);

	// Capture images from the webcam
	V4LIn* v4lin = new V4LIn("/dev/video0");
	sleep(1);
	v4lin->start();

	// Send all captured frames to the channel
	uint8_t* bgr = new uint8_t[640*480*3];
	for(int z=0; ; z++) {
		v4lin->readFrame();
		convert_to_bgr(v4lin->data, bgr, 640,480); // Webrtc video_out channels requires BGR image formats
		webrtcpp_video_out_write(fd, bgr);
	}

	sleep(99999999);

	return 0;
}
