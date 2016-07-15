/*
 * RTCPeer.h
 *
 *  Created on: 15 juil. 2016
 *      Author: jfellus
 */

#ifndef SRC_IMPL_RTCPEER_H_
#define SRC_IMPL_RTCPEER_H_

#include <webrtc/api/peerconnectioninterface.h>
#include "stream/RTCVideoStream.h"
#include "stream/RTCDataChannel.h"

namespace webrtcpp {

class SignalingWebSocketPeer;


class RTCPeer : public webrtc::PeerConnectionObserver, public webrtc::CreateSessionDescriptionObserver {
public:
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer;
	SignalingWebSocketPeer* signalingPeer;

public:
	RTCPeer(SignalingWebSocketPeer* signalingPeer);
	virtual ~RTCPeer() {}

	void open(webrtc::PeerConnectionFactoryInterface* peerConnectionFactory, std::string turn_url, std::string turn_username, std::string turn_password);
	void close();

	// PeerConnectionObserver implementation

	virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
	virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);
	virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {}

	virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
	virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state);
	virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {}
	virtual void OnRenegotiationNeeded() {}
	virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {}
	virtual void OnIceCandidatesRemoved(const std::vector<cricket::Candidate>& candidates) {}
	virtual void OnIceConnectionReceivingChange(bool receiving) {}


	// CreateSessionDescriptionObserver implementation

	void OnSuccess(webrtc::SessionDescriptionInterface* desc);
	void OnFailure(const std::string& error);

	//

	virtual void onRemoteSDP(webrtc::SessionDescriptionInterface* sdp);


protected:

	RTCVideoStreamOut* createStream(const char* name, uint32_t w, uint32_t h);
	rtc::scoped_refptr<webrtc::DataChannelInterface> createDataChannel(const char *name, int reliable);

	void createOffer(std::string turn_url, std::string turn_username, std::string turn_password);

};


}


#endif /* SRC_IMPL_RTCPEER_H_ */
