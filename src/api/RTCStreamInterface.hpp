/*
 * RTCStreamInterface.hpp
 *
 *  Created on: 07 march 2016
 *      Author: jfellus
 */

#ifndef RTCSTREAMINTERFACE_HPP_
#define RTCSTREAMINTERFACE_HPP_

namespace webrtcpp {

class RTCStreamInterface {
	public:
		virtual ~RTCStreamInterface() {}
		virtual void sendImage(const char* buffer, int w, int h) = 0;
};

}

#endif /* RTCSTREAMINTERFACE_HPP_ */
