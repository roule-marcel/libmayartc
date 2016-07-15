/*
 * RTCVideoStream.h
 *
 *  Created on: 15 juil. 2016
 *      Author: jfellus
 */

#ifndef SRC_API_STREAM_RTCVIDEOSTREAM_H_
#define SRC_API_STREAM_RTCVIDEOSTREAM_H_

#include "MemoryCapturer.hpp"
#include "MemoryRenderer.hpp"

namespace webrtcpp {

class MemoryCapturer;
class MemoryRenderer;

class RTCVideoStreamOut {
public:
	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;
	MemoryCapturer* capturer;

public:

	RTCVideoStreamOut(const char* name, uint32_t w, uint32_t h);

	bool write(const uint8_t* rgb);
};



class RTCVideoStreamIn : public FrameObserver {
public:
	rtc::scoped_refptr<webrtc::MediaStreamInterface> stream;
	MemoryRenderer* renderer;
	uint32_t w,h;

public:
	RTCVideoStreamIn();
	virtual ~RTCVideoStreamIn() {}

	void setSize(uint32_t w, uint32_t h);
	void setStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);

	bool read(uint8_t* argb);

protected:
	virtual void onFrame(uint8_t* argb, uint32_t w, uint32_t h);

};



}



#endif /* SRC_API_STREAM_RTCVIDEOSTREAM_H_ */
