#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include "../util/jpg.h"

#include <webrtcpp.h>


int main(void){
	// Create a video_in channel called 'camera'
	int camera = webrtcpp_video_in_create("camera", 0, 640,480);

	// Save all incoming images as JPEG
	webrtcpp_video_in_add_callback(camera, [&](const uint8_t* rgb, uint32_t w, uint32_t h) {
		static int i=0;
		printf("Recv frame %d (%dx%d)\n", i++, w, h);
		save_jpg(rgb, w,h, "out.jpg");
	});

	getchar();
	return 0;
}
