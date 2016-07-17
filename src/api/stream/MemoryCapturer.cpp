/*
 * MemoryCapturer.cpp
 *
 *  Created on: 07 mar 2016
 *      Author: jfellus
 */

#include "MemoryCapturer.hpp"




namespace webrtcpp {

MemoryCapturer::MemoryCapturer(uint32_t w, uint32_t h)
	: cricket::VideoCapturer(), running_(false),
	  initial_unix_timestamp_(time(NULL) * rtc::kNumNanosecsPerSec),
	  next_timestamp_(rtc::kNumNanosecsPerMillisec),
	  is_screencast_(false),
	  rotation_(webrtc::kVideoRotation_0), w(w), h(h) {
	std::vector<cricket::VideoFormat> formats;
	formats.push_back(cricket::VideoFormat(w, h, cricket::VideoFormat::FpsToInterval(30), cricket::FOURCC_ARGB));
	ResetSupportedFormats(formats);
}

MemoryCapturer::~MemoryCapturer() {
	SignalDestroyed(this);
}

bool MemoryCapturer::captureFrame(const unsigned char* rgb) {
	if (!GetCaptureFormat()) return false;
	if (!running_) return false;

	size_t size = w*h*4;

	cricket::CapturedFrame frame;
	frame.width = w;
	frame.height = h;
	frame.fourcc = cricket::FOURCC_ARGB;
	frame.data_size = size;
	frame.time_stamp = initial_unix_timestamp_ + next_timestamp_;
	next_timestamp_ += 33333333;

	char* argb = new char[size];
	for(uint y = 0; y<h; y++) {
		for(uint x=0; x<w; x++) {
			argb[(y*w+x)*4+2] = rgb[(y*w+x)*3];
			argb[(y*w+x)*4+1] = rgb[(y*w+x)*3+1];
			argb[(y*w+x)*4] = rgb[(y*w+x)*3+2];
		}
	}

	frame.data = argb;
	frame.rotation = rotation_;

	SignalFrameCaptured(this, &frame);

	delete argb; argb = NULL;
	return true;
}


cricket::CaptureState MemoryCapturer::Start(const cricket::VideoFormat& format) {
	cricket::VideoFormat supported;
	if (GetBestCaptureFormat(format, &supported)) SetCaptureFormat(&supported);
	running_ = true;
	SetCaptureState(cricket::CS_RUNNING);
	return cricket::CS_RUNNING;
}

void MemoryCapturer::Stop() {
	running_ = false;
	SetCaptureFormat(NULL);
	SetCaptureState(cricket::CS_STOPPED);
}

bool MemoryCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs) {
	fourccs->push_back(cricket::FOURCC_ARGB);
	return true;
}



}