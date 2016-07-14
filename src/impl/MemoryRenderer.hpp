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

class FrameObserver;

class MemoryRenderer : public rtc::VideoSinkInterface<cricket::VideoFrame> {
public:
	int w,h;
	unsigned char* argb;
	FrameObserver* frameObserver;

public:
	MemoryRenderer();
	virtual ~MemoryRenderer();

	void setSize(int w, int h);
	void OnFrame(const cricket::VideoFrame& frame);

	void setFrameObserver(FrameObserver* o);
};

}



#endif /* MEMORYRENDERER_HPP_ */
