/*
 * MemoryRenderer.hpp
 *
 *  Created on: 28 fev 2016
 *      Author: jfellus
 */

#ifndef MEMORYRENDERER_HPP_
#define MEMORYRENDERER_HPP_

#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/media/base/videosinkinterface.h>


namespace webrtcpp {


class FrameObserver {
public:
	virtual ~FrameObserver() {}
	virtual void onFrame(unsigned char* rgb, uint32_t w, uint32_t h) = 0;
};



class MemoryRenderer : public rtc::VideoSinkInterface<cricket::VideoFrame> {
public:
	int w,h;
	unsigned char* argb;
	unsigned char* rgb;
	FrameObserver* frameObserver;

public:
	MemoryRenderer();
	virtual ~MemoryRenderer();

	void setSize(int w, int h);
	void setFrameObserver(FrameObserver* o);


	// VideoSinkInterface implementation

	void OnFrame(const cricket::VideoFrame& frame);

};

}



#endif /* MEMORYRENDERER_HPP_ */
