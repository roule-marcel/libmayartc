#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "webrtcpp.h"

static bool stop = false;

void signal_handler(int sig){
	std::cout << "Caught signal" << std::endl;
	stop = true;
	exit(0);
}

int main(void){
	try {
	struct sigaction sigact;
	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);

	int fd = webrtcpp_open("prout", 0);

	for(int i=0; i<100 && !stop; i++) {
		//webrtcpp_write(fd, &i, sizeof(i));
		sleep(100);
	}

//	webrtcpp_close(fd);
	exit(0);

	return 0;
	} catch(const char* error) { fprintf(stderr, "ERROR : %s\n", error); return 1; }
}
