/*
 * RTCPeer.cpp
 *
 *  Created on: 15 juil. 2016
 *      Author: jfellus
 */

#include <iostream>
#include "RTCPeer.h"
#include "../util/webrtc_helpers.h"
#include "stream/RTCVideoStream.h"
#include "SignalingServer.h"

namespace webrtcpp {




static SimpleConstraints* mediaConstraints = NULL;



/////////////
// RTCPeer //
/////////////

RTCPeer::RTCPeer(SignalingWebSocketPeer* signalingPeer) : signalingPeer(signalingPeer) {
	if(!mediaConstraints) {
		mediaConstraints = new SimpleConstraints;
		mediaConstraints->AddMandatory("EnableDtlsSrtp", "true");
		mediaConstraints->AddMandatory("DtlsSrtpKeyAgreement", "true");
		mediaConstraints->AddMandatory("kEnableSctpDataChannels", "true");
		mediaConstraints->AddMandatory("OfferToReceiveAudio", "true");
		mediaConstraints->AddMandatory("OfferToReceiveVideo", "true");
	}


}

void RTCPeer::open(webrtc::PeerConnectionFactoryInterface* peerConnectionFactory, std::string turn_url, std::string turn_username, std::string turn_password) {
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	this->peer = peerConnectionFactory->CreatePeerConnection(config, mediaConstraints, NULL, NULL, this);

	// Create datachannels
	//createDataChannel(...);
	// Create streams
	//createStream(...);

	webrtc::PeerConnectionInterface::IceServers servers;
	webrtc::PeerConnectionInterface::IceServer turnserver;
	turnserver.uri = turn_url;
	turnserver.username = turn_username;
	turnserver.password = turn_password;
	servers.push_back(turnserver);

	config.servers = servers;
	peer->SetConfiguration(config);

	peer->CreateOffer(this, mediaConstraints);
}

void RTCPeer::close() {
	peer->Close();
}


///////////////
// INTERNALS //
///////////////

// WebRTC Handshaking

void RTCPeer::onRemoteSDP(webrtc::SessionDescriptionInterface* sdp) {
	peer->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(0),sdp);
}


void RTCPeer::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
	std::string sdp_mid = candidate->sdp_mid();
	int sdp_mlineindex = candidate->sdp_mline_index();
	std::string sdp;
	if (!candidate->ToString(&sdp)) return;
	signalingPeer->sendLocalICECandidate(sdp_mid, sdp_mlineindex, sdp);
}

void RTCPeer::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state){
	if(new_state == webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected
		|| new_state == webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed){
	 // TODO : connection closed ?
	}
}

void RTCPeer::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
	std::string sdp;
	peer->SetLocalDescription(DummySetSessionDescriptionObserver::Create(0), desc);
	if(!desc->ToString(&sdp)) return;

	signalingPeer->sendLocalSDP(desc->type(), sdp);
}

void RTCPeer::OnFailure(const std::string& error) {
	std::cerr << "CREATE ANSWER ERROR: " << error << std::endl;
}


//////////////
// Channels //
///////////////


// Outgoing

rtc::scoped_refptr<webrtc::DataChannelInterface> RTCPeer::createDataChannel(const char* name, int reliable) {
	struct webrtc::DataChannelInit *init = new webrtc::DataChannelInit();
	if(!reliable) {
		init->maxRetransmits = 0;
		init->ordered = false;
	}
	return peer->CreateDataChannel(name, init);
}

RTCVideoStreamOut* RTCPeer::createStream(const char* name, uint32_t w, uint32_t h) {
	RTCVideoStreamOut* out = new RTCVideoStreamOut(name, w, h);
}


// Incoming

void RTCPeer::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
	RTCVideoStreamIn* in = new RTCVideoStreamIn(); // TODO Keep a register of declared streams and just lookup them
	in->setStream(stream);
}

void RTCPeer::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
	RTCDataChannel* ch = new RTCDataChannel(data_channel);
}



}
