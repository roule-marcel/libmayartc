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


class RTCPeer : webrtc::PeerConnectionObserver, public webrtc::CreateSessionDescriptionObserver {
public:
	rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer;
	SignalingWebSocketPeer* signalingPeer;

	std::vector<std::string> requestedChannels;
	std::vector<std::string> requestedVideoOuts;

public:
	RTCPeer(SignalingWebSocketPeer* signalingPeer);
	virtual ~RTCPeer() {}

	void open(webrtc::PeerConnectionFactoryInterface* peerConnectionFactory);
	void close();

	void requestChannel(std::string name) { requestedChannels.push_back(name); }
	void requestVideoOut(std::string name) { requestedVideoOuts.push_back(name); }

	void realize();

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

	virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
	virtual void OnFailure(const std::string& error);

	//

	virtual void onRemoteIceCandidate(webrtc::IceCandidateInterface* candidate);
	virtual void onRemoteSDP(webrtc::SessionDescriptionInterface* sdp);


protected:

	void createOffer(std::string turn_url, std::string turn_username, std::string turn_password);

};


}


#endif /* SRC_IMPL_RTCPEER_H_ */
