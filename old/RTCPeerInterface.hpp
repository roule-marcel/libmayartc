/*
 * RTCPeerInterface.hpp
 *
 *  Created on: 6 août 2014
 *      Author: sylvain
 */

#ifndef RTCPEERINTERFACE_HPP_
#define RTCPEERINTERFACE_HPP_

#include "RTCChannelInterface.hpp"
#include "RTCStreamInterface.hpp"
#include "SignalingServer.h"

namespace oldwebrtcpp {

	class RTCPeerInterface : public SignalingChannelPeerInterface {
		public :
			virtual ~RTCPeerInterface(){}

			static RTCPeerInterface * create(SignalingChannelInterface *signalingChannel);

			virtual RTCChannelInterface* registerChannel(const char* name, int reliable) = 0;
			virtual RTCStreamInterface* registerStream(const char* name, uint32_t w, uint32_t h) = 0;

			virtual void join() = 0;
	};


	void initRTC();
	void destroyRTC();
}

#endif /* RTCPEERINTERFACE_HPP_ */
