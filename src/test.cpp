#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "webrtcpp.h"

static bool stop = false;

void signal_handler(int sig){
	std::cout << "Caught signal" << std::endl;
	stop = true;
}

int main(void){
	struct sigaction sigact;
	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);

	int fd = webrtcpp_open("prout", 0);

	for(int i=0; i<100 && !stop; i++) {
		webrtcpp_write(fd, &i, sizeof(i));
		sleep(1);
	}

	webrtcpp_close(fd);

	return 0;
}
