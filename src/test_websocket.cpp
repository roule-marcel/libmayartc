#include <iostream>
#include <signal.h>

#include "util/websocket.h"

class WebSocketPeer : public IWebSocketPeer {
public:
	WebSocketPeer(struct libwebsocket *ws) {
		printf("PEER CONNECTED %lu\n", (long)this);
	}

	virtual ~WebSocketPeer() {
		printf("PEER DISCONNECTED %lu\n", (long)this);
	}

	virtual void onMessage(char* msg) {
		printf("MSG (%lu) : %s\n", (long)this, msg);
	}
};

class WebSocketServer : public IWebSocketServer {
public:
	WebSocketServer(int port) : IWebSocketServer(port) {}
	virtual IWebSocketPeer* createPeer(struct libwebsocket *ws) { return new WebSocketPeer(ws); }
};


WebSocketServer* server = NULL;

void signal_handler(int sig){
	std::cout << "Caught signal" << std::endl;
	delete server;
	server = NULL;
}

int main(void){
	struct sigaction sigact;
	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);

	server = new WebSocketServer(10000);
	server->run();

	return 0;
}
