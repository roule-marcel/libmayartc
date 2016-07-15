/*
 * MemoryCapturer.hpp
 *
 *  Created on: 07 mar 2016
 *      Author: jfellus
 */

#ifndef MEMORYCAPTURER_HPP_
#define MEMORYCAPTURER_HPP_

#include <webrtc/media/base/videocapturer.h>
#include <webrtc/media/base/videocapturerfactory.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>

namespace webrtcpp {

class MemoryCapturer : public cricket::VideoCapturer {
public:
	sigslot::signal1<MemoryCapturer*> SignalDestroyed;


public:
	MemoryCapturer(uint32_t w, uint32_t h);
	~MemoryCapturer();


	bool captureFrame(const unsigned char* rgb);


	virtual cricket::CaptureState Start(const cricket::VideoFormat& format);
	virtual void Stop();


	virtual bool IsRunning() { return running_; }


	void SetScreencast(bool is_screencast) { is_screencast_ = is_screencast; }
	virtual bool IsScreencast() const { return is_screencast_; }
	bool GetPreferredFourccs(std::vector<uint32_t>* fourccs);
	void SignalCapturedFrame(cricket::CapturedFrame* frame) { SignalFrameCaptured(this, frame); }
	void ResetSupportedFormats(const std::vector<cricket::VideoFormat>& formats) { SetSupportedFormats(formats); }
	void SetRotation(webrtc::VideoRotation rotation) { rotation_ = rotation; }
	webrtc::VideoRotation GetRotation() { return rotation_; }

private:
	uint32_t w,h;

	bool running_;
	int64_t initial_unix_timestamp_;
	int64_t next_timestamp_;
	bool is_screencast_;
	webrtc::VideoRotation rotation_;
};


}





#endif /* MEMORYCAPTURER_HPP_ */
