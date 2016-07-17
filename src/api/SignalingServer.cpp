#include "SignalingServer.h"

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <unordered_map>

#include <webrtc/base/json.h>
#include <webrtc/base/ssladapter.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <libwebsockets.h>


#include "../util/websocket.h"
#include "RTCPeer.h"
#include "stream/RTCDataChannel.h"
#include "stream/RTCVideoStream.h"

namespace webrtcpp {

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> rtcPeerConnectionFactory;
rtc::Thread* sig_thread = NULL;

void initRTC() {
	rtc::InitializeSSL();

	sig_thread = rtc::ThreadManager::Instance()->WrapCurrentThread();
	rtc::Thread* worker_thread = new rtc::Thread;
	worker_thread->Start();
	rtcPeerConnectionFactory = webrtc::CreatePeerConnectionFactory(worker_thread, sig_thread, NULL, NULL, NULL);
}


//////////////////////////////
// SignalingWebSocketServer //
//////////////////////////////

SignalingWebSocketServer::SignalingWebSocketServer(int port) : IWebSocketServer(port) {}


RTCDataChannel* SignalingWebSocketServer::addDataChannel(const char* name) {
	// TODO multiplexer on RTCDataChannel ???
	printf("Declare channel %s\n", name);
	return NULL;
}

RTCVideoStreamOut* SignalingWebSocketServer::addVideoOutStream(const char* name, uint16_t w, uint16_t h) {
	// TODO multiplexer on RTCVideoOutStream ???
	printf("Declare video out %s\n", name);
	return NULL;
}

RTCVideoStreamIn* SignalingWebSocketServer::addVideoInStream(const char* name, uint16_t w, uint16_t h) {
	// TODO multiplexer on RTCVideoInStream ???
	printf("Declare video in %s\n", name);
	return NULL;
}

void SignalingWebSocketServer::start() {
	th = std::thread([&](){
		printf("Started\n");

		initRTC();

		this->run();

		printf("Finished\n");
	});
}

void SignalingWebSocketServer::run() {
	while ( !bQuit ) {
		libwebsocket_service(context, 50);
		sig_thread->ProcessMessages(10);
	}
	usleep(10);
	libwebsocket_context_destroy(context);
}

////////////////////////////
// SignalingWebSocketPeer //
////////////////////////////

SignalingWebSocketPeer::SignalingWebSocketPeer(SignalingWebSocketServer* server, struct libwebsocket *ws) : IWebSocketPeer(ws) {
	this->server = server;
	printf("PEER CONNECTED %lu\n", (long)this);

	webrtc::PeerConnectionInterface::RTCConfiguration config;
	rtcPeer = new rtc::RefCountedObject<RTCPeer>(this);
	rtcPeer->AddRef();
	rtcPeer->open(rtcPeerConnectionFactory);
}

SignalingWebSocketPeer::~SignalingWebSocketPeer() {
	printf("PEER DISCONNECTED %lu\n", (long)this);
	if(rtcPeer) rtcPeer->close();
}




// Message parsing

void parse_remote_ice_candidate(SignalingWebSocketPeer* peer, Json::Value message){
	std::string sdp_mid;
	int sdp_mlineindex;
	std::string sdp;

	rtc::GetStringFromJsonObject(message, "sdpMid", &sdp_mid);
	rtc::GetIntFromJsonObject(message, "sdpMLineIndex", &sdp_mlineindex);
	rtc::GetStringFromJsonObject(message, "candidate", &sdp);

	peer->onRemoteICECandidate(sdp_mid, sdp_mlineindex, sdp);
}

static void parse_remote_sdp(SignalingWebSocketPeer* peer, Json::Value message) {
	std::string type;
	std::string sdp;

	rtc::GetStringFromJsonObject(message, "type", &type);
	rtc::GetStringFromJsonObject(message, "sdp", &sdp);

	peer->onRemoteSDP(type, sdp);
}


static void parse_signaling_message(SignalingWebSocketPeer* peer, const char * msg) {
	std::string message = std::string(msg, strlen(msg));

	Json::Reader reader;
	Json::Value jmessage;
	if (!reader.parse(message, jmessage)) {
		std::cerr << "[SIG] error : cannot parse JSON message" << std::endl;
		return;
	}

	if(jmessage.isMember("sdp")) parse_remote_sdp(peer, jmessage);
	else  parse_remote_ice_candidate(peer, jmessage);
}


/////


void SignalingWebSocketPeer::onMessage(char* msg) {
	printf("MSG (%lu) : %s\n", (long)this, msg);
	parse_signaling_message(this, msg);
}



// WebRTC handshaking

void SignalingWebSocketPeer::onRemoteICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp) {
	if(sdp_mid == "" && sdp_mlineindex == 0 && sdp == "") return;
	webrtc::IceCandidateInterface *candidate = webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, NULL);
	if (!candidate) {
		std::cout << "[SIG] cannot parse candidate information" << std::endl;
		return;
	}
	rtcPeer->onRemoteIceCandidate(candidate);
}

void SignalingWebSocketPeer::onRemoteSDP(std::string type, std::string sdp) {
	webrtc::SessionDescriptionInterface* session_description = webrtc::CreateSessionDescription(type, sdp, NULL);
	if (!session_description) {
		std::cerr << "[SIG] error : cannot parse SDP string" << std::endl;
		return;
	}
	rtcPeer->onRemoteSDP(session_description);
}

void SignalingWebSocketPeer::sendLocalSDP(std::string type, std::string sdp) {
	Json::StyledWriter writer;
	Json::Value message;

	message["type"] = type;
	message["sdp"] = sdp;

	std::string msg = writer.write(message);
	send(msg.c_str());
}

void SignalingWebSocketPeer::sendLocalICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp) {
	Json::StyledWriter writer;
	Json::Value message;

	message["sdpMid"] = sdp_mid;
	message["sdpMLineIndex"] = sdp_mlineindex;
	message["candidate"] = sdp;

	std::string msg = writer.write(message);
	send(msg.c_str());
}

}
