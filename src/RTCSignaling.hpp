/*
 * WebRTCServer.hpp
 *
 *  Created on: 24 juil. 2014
 *      Author: sylvain
 */

#ifndef WEBRTCSERVER_HPP_
#define WEBRTCSERVER_HPP_

#include <string>
#include <vector>


namespace maya{


enum RTCSignalingChannelState{
	CONNECTING,
	CONNECTED,
	DISCONNECTED
};

class RTCSignalingChannelPeer{
	public:
		virtual ~RTCSignalingChannelPeer() {};

		virtual void disconnect() = 0;

		virtual void onSignalingThreadStarted() = 0;
		virtual void onSignalingThreadStopped() = 0;
		virtual void processMessages() = 0;

		virtual void onStateChanged(RTCSignalingChannelState state) = 0;
		virtual void onMessage(int peerid, const char * message, int length) = 0;
		virtual void onRemoteSDP(int peerid, std::string type, std::string sdp) = 0;
		virtual void onRemoteICECandidate(int peerid, std::string sdp_mid, int sdp_mlineindex, std::string sdp) = 0;
		virtual void onConnectionRequest(int peerid, std::vector<std::string> channels, std::string turn_url, std::string turn_username, std::string turn_password) = 0;

		// jfellus 26/02/2016
		virtual void addChannelStreamMapping(int peerid, std::string channel, std::string stream) = 0;
		//

		virtual std::vector<std::string> getChannelNames() = 0;
};

class RTCSignalingChannel {
	public:
		virtual ~RTCSignalingChannel(){}

		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void join() = 0;

		void onSignalingThreadStarted();

		virtual void sendMessage(int peerid, const char * message, int length) = 0;
		virtual void sendLocalSDP(int peerid, std::string type, std::string sdp) = 0;
		virtual void sendLocalICECandidate(int peerid, std::string sdp_mid, int sdp_mlineindex, std::string sdp) = 0;

		void setPeer(RTCSignalingChannelPeer *peer);
		RTCSignalingChannelPeer * getPeer();

	private:
		RTCSignalingChannelPeer* peer;
};

} //NAMESPACE MAYA



#endif /* WEBRTCSERVER_HPP_ */
