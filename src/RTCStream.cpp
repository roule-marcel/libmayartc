/*
 * RTCStream.cpp
 *
 *  Created on: 07 march 2016
 *      Author: jfellus
 */

#include "RTCStream.hpp"
#include "webrtc/api/mediastreaminterface.h"
#include "webrtc/api/videosourceinterface.h"

namespace maya{

RTCStream::RTCStream(const char* name, uint w, uint h) {
	this->w = w;
	this->h = h;
	this->name = name;
	this->capturer = NULL;
}

void RTCStream::init(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peerConnectionFactory, rtc::Thread* thread) {
	if(this->capturer) return;

	this->capturer = new MemoryCapturer(thread, w,h);
	rtc::scoped_refptr<webrtc::VideoSourceInterface> source(peerConnectionFactory->CreateVideoSource(capturer, NULL));
	rtc::scoped_refptr<webrtc::VideoTrackInterface> track(peerConnectionFactory->CreateVideoTrack(name,source));
	stream = peerConnectionFactory->CreateLocalMediaStream(name);
	stream->AddTrack(track);
}

RTCStream::~RTCStream() {
}

void RTCStream::sendImage(const char* buffer, int w, int h) {
	if(!capturer) return;

	this->w = w; this->h = h;
	capturer->CaptureFrame(buffer, w,h);
}

}
