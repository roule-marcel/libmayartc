
#include "MemoryRenderer.hpp"
#include "webrtc/media/base/mediachannel.h"
#include "webrtc/media/base/videocommon.h"
#include "webrtc/media/base/videoframe.h"


#include "../../util/jpg.h"

namespace webrtcpp {

MemoryRenderer::MemoryRenderer() {
	w = h = 0;
	this->argb = 0;
	this->rgb = 0;
	this->frameObserver = NULL;
}

MemoryRenderer::~MemoryRenderer() { }


void MemoryRenderer::setSize(int w, int h) {
	this->w = w; this->h = h;
}


void MemoryRenderer::setFrameObserver(FrameObserver* o) {
	this->frameObserver = o;
}



// VideoSinkInterface implementation

void MemoryRenderer::OnFrame(const cricket::VideoFrame& frame) {
	const cricket::VideoFrame* rotatedFrame = frame.GetCopyWithRotationApplied();
	setSize(static_cast<int>(rotatedFrame->width()), static_cast<int>(rotatedFrame->height()));

	if(!this->argb) {
		this->argb = new unsigned char[4*w*h];
		this->rgb = new unsigned char[3*w*h];
	}

	static int i=0;

	rotatedFrame->ConvertToRgbBuffer(cricket::FOURCC_ARGB, argb, 4*w*h,0);

	for(uint i = 0; i<h*w; i++) {
		rgb[i*3] = argb[i*4+2];
		rgb[i*3+1] = argb[i*4+1];
		rgb[i*3+2] = argb[i*4];
	}

	if(frameObserver) frameObserver->onFrame(rgb, w, h);
}



}
