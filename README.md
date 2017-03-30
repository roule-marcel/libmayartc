# libwebrtcpp

A C++ WebRTC channel library forked from Partnering's libmayartc (thank you guys !)


# Examples

### Datachannels

````c++
int fd = webrtcpp_create("test");

webrtcpp_add_callback(fd, [&](const char* buf, size_t size){
		printf("> %s\n", buf);
});

// Send all lines from stdin to the channel
char* buf = 0;
size_t n = 0;
while(getline(&buf, &n, stdin)) {
	webrtcpp_write(fd, buf, n);
	buf = 0; n = 0;
}
````

### Video In

````c++
	int camera = webrtcpp_video_in_create("camera", 0, 640,480);

	webrtcpp_video_in_add_callback(camera, [&](const uint8_t* rgb, uint32_t w, uint32_t h) {
		static int i=0;
		printf("Recv frame %d (%dx%d)\n", i++, w, h);
		// ... process RGB frame here ...
	});
````

### Video out

````c++
	int fd = webrtcpp_video_out_create("camera", 0, 640, 480);

	uint8_t* bgr = new uint8_t[640*480*3];
	for(int z=0; ; z++) {
    		// ... write something to the BGR frame here ...
		webrtcpp_video_out_write(fd, bgr);
	}
````  

## Install dependencies

##### Builds
- 31/03/2017 : Builds with webrtc commit [f0e1f60](https://chromium.googlesource.com/external/webrtc.git/+/f0e1f60b0c253377be6cc7fa3745467503d2631b)

##### WebRTC source repository

https://webrtc.org/native-code/development/

##### Example code 
https://chromium.googlesource.com/external/webrtc/+/master/webrtc/examples/peerconnection
