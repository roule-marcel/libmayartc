
#include "MemoryRenderer.hpp"
#include "webrtc/media/base/mediachannel.h"
#include "webrtc/media/base/videocommon.h"
#include "webrtc/media/base/videoframe.h"


#include "../../util/jpg.h"

namespace webrtcpp {

MemoryRenderer::MemoryRenderer() {
	w = h = 0;
	this->argb = 0;
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

	if(!this->argb) this->argb = new unsigned char[4*w*h];

	static int i=0;

	rotatedFrame->ConvertToRgbBuffer(cricket::FOURCC_ARGB, argb, 4*w*h,0);
	if(frameObserver) frameObserver->onFrame(argb, w, h);
}



}
