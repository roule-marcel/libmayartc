#include <iostream>

#include <signal.h>

#include "api/SignalingInterface.hpp"
#include "api/RTCPeerInterface.hpp"

using namespace webrtcpp;

SignalingChannelInterface *signaling;
RTCPeerInterface * peer;

void signal_handler(int sig){
	std::cout << "Caught signal" << std::endl;
	signaling->stop();
}

int main(void){

	struct sigaction sigact;

	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);






	initRTC();

	signaling = SignalingChannelInterface::create();
	signaling->start();

	peer = (RTCPeerInterface*) signaling->peer;
	peer->registerChannel("ctrl.info", 1);
	peer->join();

	return 0;
}
