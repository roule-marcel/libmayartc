/*
 * RTCConnection.hpp
 *
 *  Created on: 5 août 2014
 *      Author: sylvain
 */

#ifndef RTCCONNECTION_HPP_
#define RTCCONNECTION_HPP_

#include <ctime>
#include <webrtc/api/peerconnectioninterface.h>

#include "RTCCommon.hpp"

namespace webrtcpp {

class RTCPeer;

class RTCConnection : public webrtc::PeerConnectionObserver, webrtc::CreateSessionDescriptionObserver, public rtc::MessageHandler{

	private :
		int peerID;
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection;
		RTCPeer *peer;
		std::time_t createOfferTimestamp;


	public :
		RTCConnection(RTCPeer *peer, webrtc::PeerConnectionFactoryInterface * peerConnectionFactory,  int peerID);
		virtual ~RTCConnection();


		// PeerConnectionObserver Implementation

		virtual void OnError(){	std::cerr << "An Error occured !" << std::endl;}
		virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {}
		virtual void OnStateChange(StateType state_changed) {}
		virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream){}
		virtual void OnRenegotiationNeeded(){}
		virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state);
		virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {}
		virtual void OnIceComplete() {}
		virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel);
		virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
		virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);


		// CreateSessionDescriptionObserver implementation

		virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
		virtual void OnFailure(const std::string& error);



		void OnMessage(rtc::Message * msg);

		rtc::scoped_refptr<webrtc::DataChannelInterface> createDataChannel(char *name, int reliable);
		void createOffer(std::string turn_url, std::string turn_username, std::string turn_password);

		void setRemoteSessionDescription(webrtc::SessionDescriptionInterface* session_description);
		void addStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
		void addICECandidate(webrtc::IceCandidateInterface *candidate);
		SimpleConstraints * getMediaConstraints();

		void close();
		int getPeerID();
		bool hasTimeoutExpired();
};

}

#endif /* RTCCONNECTION_HPP_ */
