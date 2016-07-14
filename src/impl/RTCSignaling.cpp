/*
 * WebRTCServer.cpp
 *
 *  Created on: 24 juil. 2014
 *      Author: sylvain
 */

#include "../api/SignalingInterface.hpp"

namespace webrtcpp {

void SignalingChannelInterface::onSignalingThreadStarted(){
	peer->onSignalingThreadStarted();
}

} //NAMESPACE MAYA

