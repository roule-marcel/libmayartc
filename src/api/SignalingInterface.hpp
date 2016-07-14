#ifndef SIGNALINGINTERFACE_HPP_
#define SIGNALINGINTERFACE_HPP_

#include <string>
#include <vector>


namespace webrtcpp {


enum SignalingChannelState { CONNECTING, CONNECTED, DISCONNECTED };



class SignalingChannelPeerInterface;
class SignalingChannelInterface;


///////////////////////////////
// SignalingChannelInterface //
///////////////////////////////

class SignalingChannelInterface {
public:
	SignalingChannelPeerInterface* peer;

public:
	static SignalingChannelInterface *create();

	virtual ~SignalingChannelInterface(){}

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void join() = 0;

	void onSignalingThreadStarted();

	virtual void sendMessage(int peerid, const char * message, int length) = 0;
	virtual void sendLocalSDP(int peerid, std::string type, std::string sdp) = 0;
	virtual void sendLocalICECandidate(int peerid, std::string sdp_mid, int sdp_mlineindex, std::string sdp) = 0;

};



///////////////////////////////////
// SignalingChannelPeerInterface //
///////////////////////////////////


class SignalingChannelPeerInterface {
public:
	virtual ~SignalingChannelPeerInterface() {};

	virtual void disconnect() = 0;

	virtual void onSignalingThreadStarted() = 0;
	virtual void onSignalingThreadStopped() = 0;
	virtual void processMessages() = 0;

	virtual void onStateChanged(SignalingChannelState state) = 0;
	virtual void onMessage(int peerid, const char * message, int length) = 0;
	virtual void onRemoteSDP(int peerid, std::string type, std::string sdp) = 0;
	virtual void onRemoteICECandidate(int peerid, std::string sdp_mid, int sdp_mlineindex, std::string sdp) = 0;
	virtual void onConnectionRequest(int peerid, std::vector<std::string> channels, std::string turn_url, std::string turn_username, std::string turn_password) = 0;

	virtual void addChannelStreamMapping(int peerid, std::string channel, std::string stream) = 0;

	virtual std::vector<std::string> getChannelNames() = 0;
};


}


#endif /* SIGNALINGINTERFACE_HPP_ */
