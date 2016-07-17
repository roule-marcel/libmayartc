#ifndef SIGNALINGINTERFACE_HPP_
#define SIGNALINGINTERFACE_HPP_

#include <string>
#include <vector>
#include <thread>
#include "../util/websocket.h"
#include "RTCPeer.h"

namespace webrtcpp {


class SignalingWebSocketServer;


class SignalingWebSocketPeer : public IWebSocketPeer {
public:
	SignalingWebSocketServer* server;
	RTCPeer* rtcPeer;

public:
	SignalingWebSocketPeer(SignalingWebSocketServer* server, struct libwebsocket *ws);
	virtual ~SignalingWebSocketPeer();
	virtual void onMessage(char* msg);

	void onRemoteICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp);
	void onRemoteSDP(std::string type, std::string sdp);

	void sendLocalSDP(std::string type, std::string sdp);
	void sendLocalICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp);
};




class SignalingWebSocketServer : public IWebSocketServer {
private:
	std::thread th;
public:
	SignalingWebSocketServer(int port);

	void start();
	virtual IWebSocketPeer* createPeer(struct libwebsocket *ws) { return new SignalingWebSocketPeer(this, ws); }
	virtual void run();

	RTCDataChannel* addDataChannel(const char* name);
	RTCVideoStreamIn* addVideoInStream(const char* name, uint16_t w, uint16_t h);
	RTCVideoStreamOut* addVideoOutStream(const char* name, uint16_t w, uint16_t h);

};




}


#endif /* SIGNALINGINTERFACE_HPP_ */
