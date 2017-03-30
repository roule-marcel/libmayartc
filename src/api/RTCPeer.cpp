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

void RTCPeer::open(webrtc::PeerConnectionFactoryInterface* peerConnectionFactory) {
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	peer = peerConnectionFactory->CreatePeerConnection(config, mediaConstraints, NULL, NULL, this);
}

void RTCPeer::close() {
	peer->Close();
}

void RTCPeer::realize() {
	// Create datachannels
	for(std::string name : requestedChannels) {
		RTCDataChannel* ch = signalingPeer->getDataChannel(name);
		if(!ch) continue;
		struct webrtc::DataChannelInit *init = new webrtc::DataChannelInit();
		rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel = peer->CreateDataChannel(name, init);
		dataChannel->RegisterObserver(ch);
		ch->add(dataChannel);
	}

	// Create streams
	for(std::string name : requestedVideoOuts) {
		RTCVideoStreamOut* out = signalingPeer->getVideoStreamOut(name);
		if(!out) continue;
		peer->AddStream(out->stream);
	}

	peer->CreateOffer(this, mediaConstraints);
}

///////////////
// INTERNALS //
///////////////

// WebRTC Handshaking

void RTCPeer::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
	std::string sdp;
	peer->SetLocalDescription(DummySetSessionDescriptionObserver::Create(0), desc);
	if(!desc->ToString(&sdp)) return;

	signalingPeer->sendLocalSDP(desc->type(), sdp);
}

void RTCPeer::OnFailure(const std::string& error) {
	printf("Offer error\n");
}

void RTCPeer::onRemoteSDP(webrtc::SessionDescriptionInterface* sdp) {
	std::string s; sdp->ToString(&s);
	peer->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(0),sdp);
}

void RTCPeer::onRemoteIceCandidate(webrtc::IceCandidateInterface* candidate) {
	std::string s;
	if (!candidate->ToString(&s)) return;
	peer->AddIceCandidate(candidate);
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




//////////////
// Channels //
//////////////


void RTCPeer::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
	RTCVideoStreamIn* in = signalingPeer->getVideoStreamInByLabel(stream->label());
	if(!in) return;
	in->setStream(stream);
}

void RTCPeer::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {
	// NOTE : Datachannel are created by the native server, not by the client browser !
}



}
