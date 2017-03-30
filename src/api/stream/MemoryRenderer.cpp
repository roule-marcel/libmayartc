
#include "MemoryRenderer.hpp"
#include "webrtc/media/base/mediachannel.h"
#include "webrtc/media/base/videocommon.h"
#include "webrtc/api/video/video_frame.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

#include "../../util/jpg.h"

namespace webrtcpp {

MemoryRenderer::MemoryRenderer() {
	w = h = 0;
	this->rgb = 0;
	this->frameObserver = NULL;
}

MemoryRenderer::~MemoryRenderer() { }



void MemoryRenderer::setFrameObserver(FrameObserver* o) {
	this->frameObserver = o;
}



// VideoSinkInterface implementation

void MemoryRenderer::OnFrame(const webrtc::VideoFrame& frame) {
	if(!frame.width() || !frame.height()) return;
	if(!this->rgb) {
		w = frame.width();
		h = frame.height();
		this->rgb = new unsigned char[3*w*h];
	}
	const rtc::scoped_refptr<webrtc::VideoFrameBuffer> fb = frame.video_frame_buffer();
	webrtc::ConvertFromI420(frame, webrtc::kRGB24, 0, rgb);
	if(frameObserver) frameObserver->onFrame(rgb, w, h);
}



}
