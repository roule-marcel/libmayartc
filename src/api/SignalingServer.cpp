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

#include <libwebsockets.h>


#include "../util/websocket.h"
#include "RTCPeer.h"
#include "stream/RTCDataChannel.h"
#include "stream/RTCVideoStream.h"

namespace webrtcpp {

webrtc::PeerConnectionFactoryInterface* rtcPeerConnectionFactory = NULL;


void initRTC() {
//	rtc::InitializeSSL();

	rtc::Thread* sig_thread = rtc::ThreadManager::Instance()->WrapCurrentThread();
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
	return NULL;
}

RTCVideoStreamOut* SignalingWebSocketServer::addVideoOutStream(const char* name, uint16_t w, uint16_t h) {
	// TODO multiplexer on RTCVideoOutStream ???
	return NULL;
}

RTCVideoStreamIn* SignalingWebSocketServer::addVideoInStream(const char* name, uint16_t w, uint16_t h) {
	// TODO multiplexer on RTCVideoInStream ???
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


////////////////////////////
// SignalingWebSocketPeer //
////////////////////////////

SignalingWebSocketPeer::SignalingWebSocketPeer(SignalingWebSocketServer* server, struct libwebsocket *ws) {
	this->server = server;
	rtcPeer = NULL;
	printf("PEER CONNECTED %lu\n", (long)this);
}

SignalingWebSocketPeer::~SignalingWebSocketPeer() {
	printf("PEER DISCONNECTED %lu\n", (long)this);
}




// Message parsing

void parse_remote_ice_candidate(SignalingWebSocketPeer* peer, Json::Value message){
	std::string sdp_mid;
	int sdp_mlineindex;
	std::string sdp;

	Json::Value candidate;

	rtc::GetValueFromJsonObject(message, "candidate", &candidate);

	rtc::GetStringFromJsonObject(candidate, "sdpMid", &sdp_mid);
	rtc::GetIntFromJsonObject(candidate, "sdpMLineIndex", &sdp_mlineindex);
	rtc::GetStringFromJsonObject(candidate, "candidate", &sdp);

	peer->onRemoteICECandidate(sdp_mid, sdp_mlineindex, sdp);
}

void parse_connect(SignalingWebSocketPeer* peer, Json::Value message){
	Json::Value turn;
	std::string turn_url;
	std::string turn_username;
	std::string turn_password;

	rtc::GetValueFromJsonObject(message, "turn", &turn);
	rtc::GetStringFromJsonObject(turn, "url", &turn_url);
	rtc::GetStringFromJsonObject(turn, "username", &turn_username);
	rtc::GetStringFromJsonObject(turn, "password", &turn_password);

	peer->onConnection(turn_url, turn_username, turn_password);
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

	std::string cmd;
	rtc::GetStringFromJsonObject(jmessage,"cmd", &cmd);

	if(cmd.empty()) return ; // if no command specified, ignore message

	if(cmd.compare("Connect") == 0) parse_connect(peer, jmessage);
	else if(cmd.compare("ICECandidate") == 0) parse_remote_ice_candidate(peer, jmessage);
	else if(cmd.compare("Answer") == 0) parse_remote_sdp(peer, jmessage);
}


/////


void SignalingWebSocketPeer::onMessage(char* msg) {
	printf("MSG (%lu) : %s\n", (long)this, msg);
	parse_signaling_message(this, msg);
}



// WebRTC handshaking

void SignalingWebSocketPeer::onConnection(std::string turn_url, std::string turn_username, std::string turn_password) {
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	rtcPeer = new rtc::RefCountedObject<RTCPeer>(this);
	rtcPeer->open(rtcPeerConnectionFactory, turn_url, turn_username, turn_password);
}

void SignalingWebSocketPeer::onRemoteICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp) {
	if(sdp_mid == "" && sdp_mlineindex == 0 && sdp == "") return;
	webrtc::IceCandidateInterface *candidate = webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, NULL);
	if (!candidate) {
		std::cout << "[SIG] cannot parse candidate information" << std::endl;
		return;
	}
	rtcPeer->peer->AddIceCandidate(candidate);
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

	message["cmd"] = "RemoteOffer";
	message["type"] = type;
	message["sdp"] = sdp;

	std::string msg = writer.write(message);
	send(msg.c_str());
}

void SignalingWebSocketPeer::sendLocalICECandidate(std::string sdp_mid, int sdp_mlineindex, std::string sdp) {
	Json::StyledWriter writer;
	Json::Value message;
	Json::Value candidate;

	message["cmd"] = "RemoteICECandidate";

	candidate["sdpMid"] = sdp_mid;
	candidate["sdpMLineIndex"] = sdp_mlineindex;
	candidate["candidate"] = sdp;

	message["candidate"] = candidate;

	std::string msg = writer.write(message);
	send(msg.c_str());
}

}
