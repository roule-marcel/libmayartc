/*
 * RTCVideoStream.cpp
 *
 *  Created on: 15 juil. 2016
 *      Author: jfellus
 */




#include "RTCVideoStream.h"
#include "MemoryCapturer.hpp"
#include <webrtc/media/base/videocapturer.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/api/peerconnectioninterface.h>

namespace webrtcpp {

extern webrtc::PeerConnectionFactoryInterface* rtcPeerConnectionFactory;


///////////////////////
// RTCVideoStreamOut //
///////////////////////

RTCVideoStreamOut::RTCVideoStreamOut(const char* name, uint32_t w, uint32_t h) {
	capturer = new MemoryCapturer(w,h);
	rtc::scoped_refptr<webrtc::VideoTrackInterface> track(rtcPeerConnectionFactory->CreateVideoTrack(name,rtcPeerConnectionFactory->CreateVideoSource(capturer, NULL)));
	stream = rtcPeerConnectionFactory->CreateLocalMediaStream(name);
	stream->AddTrack(track);
}

bool RTCVideoStreamOut::write(const uint8_t* rgb) {
	if(!capturer) return false;
	capturer->captureFrame(rgb);
	return true;
}


//////////////////////
// RTCVideoStreamIn //
//////////////////////

RTCVideoStreamIn::RTCVideoStreamIn() {
	w = h = 0;
	renderer = new MemoryRenderer();
	renderer->setFrameObserver(this);
}

void RTCVideoStreamIn::setStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
	// TODO : Handle [0] out of bound
	stream->GetVideoTracks()[0]->AddOrUpdateSink(renderer, rtc::VideoSinkWants());
}

void RTCVideoStreamIn::setSize(uint32_t w, uint32_t h) {
	this->w = w;
	this->h = h;
	renderer->setSize(w,h);
}


bool RTCVideoStreamIn::read(unsigned char* argb) {
	// TODO semaphore stuff
	return false;
}

void RTCVideoStreamIn::onFrame(unsigned char* argb, uint32_t w, uint32_t h) {
	// TODO read stuff
	printf("FRAME : %ux%u\n", w,h);
}


}
