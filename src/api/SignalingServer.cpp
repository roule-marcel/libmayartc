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
	RTCDataChannel* ch = new RTCDataChannel(name);
	dataChannels.push_back(ch);
	return ch;
}

RTCVideoStreamOut* SignalingWebSocketServer::addVideoOutStream(const char* name, uint16_t w, uint16_t h) {
	RTCVideoStreamOut* out = new RTCVideoStreamOut(name, w, h);
	videoOuts.push_back(out);
	return out;
}

RTCVideoStreamIn* SignalingWebSocketServer::addVideoInStream(const char* name, uint16_t w, uint16_t h) {
	RTCVideoStreamIn* in = new RTCVideoStreamIn(name);
	videoIns.push_back(in);
	return in;
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
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	rtcPeer = new rtc::RefCountedObject<RTCPeer>(this);
	rtcPeer->AddRef();
	rtcPeer->open(rtcPeerConnectionFactory);
}

SignalingWebSocketPeer::~SignalingWebSocketPeer() {
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

static void parse_stream_label(SignalingWebSocketPeer* peer, Json::Value message) {
	std::string label;
	std::string name;
	rtc::GetStringFromJsonObject(message, "streamId", &label);
	rtc::GetStringFromJsonObject(message, "name", &name);

	peer->declareStreamLabel(name, label);
}

static void parse_requested_channels(SignalingWebSocketPeer* peer, Json::Value message) {
	Json::Value value;
	std::vector<std::string> channels;
	std::vector<std::string> streams;
	rtc::GetValueFromJsonObject(message, "channels", &value);
	rtc::JsonArrayToStringVector(value, &channels);
	rtc::GetValueFromJsonObject(message, "streams", &value);
	rtc::JsonArrayToStringVector(value, &streams);

	for(auto ch : channels) peer->rtcPeer->requestChannel(ch);
	for(auto s : streams) peer->rtcPeer->requestVideoOut(s);
}



static void parse_signaling_message(SignalingWebSocketPeer* peer, const char * msg) {
	std::string message = std::string(msg, strlen(msg));

	Json::Reader reader;
	Json::Value jmessage;
	if (!reader.parse(message, jmessage)) {
		std::cerr << "[SIG] error : cannot parse JSON message" << std::endl;
		return;
	}

	if(jmessage.isMember("channels")) parse_requested_channels(peer, jmessage);
	else if(jmessage.isMember("streamId")) parse_stream_label(peer, jmessage);
	else if(jmessage.isMember("sdp")) parse_remote_sdp(peer, jmessage);
	else  parse_remote_ice_candidate(peer, jmessage);
}


/////


void SignalingWebSocketPeer::onMessage(char* msg) {
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


///////////////
// ACCESSORS //
///////////////

RTCDataChannel* SignalingWebSocketPeer::getDataChannel(std::string name) {
	for(auto ch : server->dataChannels) {
		if(ch->name == name) return ch;
	}
	return NULL;
}

RTCVideoStreamIn* SignalingWebSocketPeer::getVideoStreamIn(std::string name) {
	for(auto s : server->videoIns) {
		if(s->name == name) return s;
	}
	return NULL;
}

RTCVideoStreamIn* SignalingWebSocketPeer::getVideoStreamInByLabel(std::string label) {
	for(auto s : videoInLabels) {
		if(s.first == label) return getVideoStreamIn(s.second);
	}
	return NULL;
}


RTCVideoStreamOut* SignalingWebSocketPeer::getVideoStreamOut(std::string name) {
	for(auto s : server->videoOuts) {
		if(s->name == name) return s;
	}
	return NULL;
}


}
