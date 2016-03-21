/*
 * RTCStream.hpp
 *
 *  Created on: 07 march 2016
 *      Author: jfellus
 */

#ifndef RTCSTREAM_HPP_
#define RTCSTREAM_HPP_

#include "RTCStreamInterface.hpp"
#include "MemoryCapturer.hpp"
#include "webrtc/media/devices/devicemanager.h"
#include "webrtc/media/base/videocapturer.h"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/videosourceinterface.h"

namespace maya{

class RTCStream : public RTCStreamInterface {
	private:
		uint w,h;
		std::string name;
		MemoryCapturer* capturer;
		rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;

	public:
		RTCStream(const char* name, uint w, uint h);
		virtual ~RTCStream();

		void init(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory, rtc::Thread* thread);

		virtual rtc::scoped_refptr<webrtc::MediaStreamInterface> getStream() { return stream; }
		virtual void sendImage(const char* buffer, int w, int h);
};

}

#endif /* RTCSTREAM_HPP_ */
