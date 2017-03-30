/*
 * MemoryCapturer.cpp
 *
 *  Created on: 07 mar 2016
 *      Author: jfellus
 */

#include "MemoryCapturer.hpp"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"


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

	rtc::scoped_refptr<webrtc::I420Buffer> buffer = webrtc::I420Buffer::Create(w,h);
	webrtc::ConvertToI420(webrtc::kRGB24, rgb, 0,0, w,h, 0, rotation_, buffer);
	webrtc::VideoFrame frame(buffer, initial_unix_timestamp_ + next_timestamp_, 0, rotation_);
	next_timestamp_ += 33333333;

	this->OnFrame(frame, w,h);

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
