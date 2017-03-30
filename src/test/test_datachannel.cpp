#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <thread>

#include <webrtcpp.h>


int main(void){
	// Creates a webrtc channel called 'test'
	int fd = webrtcpp_create("test");

	// Prints all data from this channel to stdout
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

	return 0;
}
