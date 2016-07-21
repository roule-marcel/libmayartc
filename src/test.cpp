#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include "webrtcpp.h"
#include "util/jpg.h"
#include "util/v4l2/V4LIn.h"

static bool stop = false;

V4LIn* v4lin = NULL;

void signal_handler(int sig){
	std::cout << "Caught signal" << std::endl;
	stop = true;
	if(v4lin) delete v4lin;
	exit(0);
}

int main(void){
	try {
	struct sigaction sigact;
	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);

	printf("Create Datachannel 'prout'\n");
	int prout = webrtcpp_create("prout");
	webrtcpp_add_callback(prout, [&](const char* buf, size_t size){
		printf("prout: %s\n", buf);
	});

	printf("Create Video In 'camera'\n");
	int camera = webrtcpp_video_in_create("camera", 0, 640,480);
	webrtcpp_video_in_add_callback(camera, [&](const uint8_t* rgb, uint32_t w, uint32_t h) {
		static char camera_out[256];
		static int i=0;
		printf("CAMERA %d\n", i);
//		sprintf(camera_out, "camera_%08d.jpg", i++);
//		save_jpg(rgb, w,h, camera_out);
	});

	printf("Create Video Out 'stuff'\n");
	int stuff = webrtcpp_video_out_create("stuff", 0, 640, 480);


	V4LIn* v4lin = new V4LIn("/dev/video0");
	sleep(1);
	v4lin->start();


	uint8_t* rgb = new uint8_t[640*480*3];
	for(int z=0; ; z++) {
		v4lin->readFrame();
		webrtcpp_video_out_write(stuff, v4lin->data);
	}


	getchar();
	exit(0);

	return 0;
	} catch(const char* error) { fprintf(stderr, "ERROR : %s\n", error); return 1; }
	catch(...) {fprintf(stderr, "error\n"); }
}
