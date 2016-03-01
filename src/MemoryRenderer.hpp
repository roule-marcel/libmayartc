/*
 * MemoryRenderer.hpp
 *
 *  Created on: 28 fev 2016
 *      Author: jfellus
 */

#ifndef MEMORYRENDERER_HPP_
#define MEMORYRENDERER_HPP_

#include "webrtc/api/mediastreaminterface.h"


namespace maya {

class FrameObserver;

class MemoryRenderer : public webrtc::VideoRendererInterface {
public:
	int w,h;
	unsigned char* argb;
	FrameObserver* frameObserver;
public:
	MemoryRenderer();
	virtual ~MemoryRenderer();

	void setSize(int w, int h);
	void RenderFrame(const cricket::VideoFrame* frame);

	void setFrameObserver(FrameObserver* o);
};

}



#endif /* MEMORYRENDERER_HPP_ */
