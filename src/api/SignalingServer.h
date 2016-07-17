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
	std::map<std::string, std::string> videoInLabels;

public:
	SignalingWebSocketPeer(SignalingWebSocketServer* server, struct libwebsocket *ws);
	virtual ~SignalingWebSocketPeer();
	virtual void onMessage(char* msg);

	void onRemoteICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp);
	void onRemoteSDP(std::string type, std::string sdp);

	void sendLocalSDP(std::string type, std::string sdp);
	void sendLocalICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp);

	void declareStreamLabel(std::string name, std::string label) { videoInLabels[label] = name; }

	RTCDataChannel* getDataChannel(std::string name);
	RTCVideoStreamIn* getVideoStreamIn(std::string name);
	RTCVideoStreamIn* getVideoStreamInByLabel(std::string label);
	RTCVideoStreamOut* getVideoStreamOut(std::string name);

};




class SignalingWebSocketServer : public IWebSocketServer {

public:
	std::vector<RTCDataChannel*> dataChannels;
	std::vector<RTCVideoStreamIn*> videoIns;
	std::vector<RTCVideoStreamOut*> videoOuts;

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
