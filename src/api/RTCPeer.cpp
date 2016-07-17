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


class DataChannelTestObserver : public webrtc::DataChannelObserver {
	rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel;
	int i;
public:
	DataChannelTestObserver(rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel) : dataChannel(dataChannel) {
		 i = 0;
	}
	virtual void OnStateChange() {
		if(dataChannel->state() == webrtc::DataChannelInterface::DataState::kOpen) {
			char buf[256];
			sprintf(buf, "MESSAGEMIGNON n%08d", i++);
			dataChannel->Send(webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buf, strlen(buf)),true));
		}
	}
	virtual void OnMessage(const webrtc::DataBuffer& buffer) {
		const char* r = (const char*) buffer.data.data();
		printf("Datachannel message : %s\n", r);
		char buf[256];
		sprintf(buf, "REPONSE %08d", i++);
		dataChannel->Send(webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buf, strlen(buf)),true));
	}
};



/////////////
// RTCPeer //
/////////////

RTCPeer::RTCPeer(SignalingWebSocketPeer* signalingPeer) : signalingPeer(signalingPeer) {
	if(!mediaConstraints) {
		mediaConstraints = new SimpleConstraints;
		mediaConstraints->AddMandatory("EnableDtlsSrtp", "false");
		mediaConstraints->AddMandatory("DtlsSrtpKeyAgreement", "false");
		mediaConstraints->AddMandatory("kEnableSctpDataChannels", "true");
		mediaConstraints->AddMandatory("OfferToReceiveAudio", "true");
		mediaConstraints->AddMandatory("OfferToReceiveVideo", "true");
	}


}

void RTCPeer::open(webrtc::PeerConnectionFactoryInterface* peerConnectionFactory) {
	webrtc::PeerConnectionInterface::RTCConfiguration config;
	peer = peerConnectionFactory->CreatePeerConnection(config, NULL, NULL, this);

	// Create datachannels
	rtc::scoped_refptr<webrtc::DataChannelInterface> ch = createDataChannel("prout", 1);
	ch->RegisterObserver(new DataChannelTestObserver(ch));

	// Create streams
	//createStream(...);

	peer->CreateOffer(this, NULL);
}

void RTCPeer::close() {
	peer->Close();
}


///////////////
// INTERNALS //
///////////////

// WebRTC Handshaking

void RTCPeer::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
	std::string sdp;
	peer->SetLocalDescription(DummySetSessionDescriptionObserver::Create(0), desc);
	if(!desc->ToString(&sdp)) return;

	printf("Send Local SDP : %s\n", sdp.c_str());

	signalingPeer->sendLocalSDP(desc->type(), sdp);
}

void RTCPeer::OnFailure(const std::string& error) {
	printf("Offer error\n");
}

void RTCPeer::onRemoteSDP(webrtc::SessionDescriptionInterface* sdp) {
	std::string s; sdp->ToString(&s);
	printf("Received Remote SDP : %s\n", s.c_str());
	peer->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(0),sdp);
}

void RTCPeer::onRemoteIceCandidate(webrtc::IceCandidateInterface* candidate) {
	std::string s;
	if (!candidate->ToString(&s)) return;
	printf("Received Remote ICE Candidate : %s\n", s.c_str());
	peer->AddIceCandidate(candidate);
}

void RTCPeer::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
	std::string sdp_mid = candidate->sdp_mid();
	int sdp_mlineindex = candidate->sdp_mline_index();
	std::string sdp;
	if (!candidate->ToString(&sdp)) return;
	printf("Send ICE Candidate %s\n", sdp.c_str());
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
