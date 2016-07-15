/*
 * RTCDataChannel.h
 *
 *  Created on: 15 juil. 2016
 *      Author: jfellus
 */

#ifndef SRC_API_STREAM_RTCDATACHANNEL_H_
#define SRC_API_STREAM_RTCDATACHANNEL_H_


#include <iostream>
#include <webrtc/api/datachannelinterface.h>

namespace webrtcpp {

class RTCDataChannel : public webrtc::DataChannelObserver {
	rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel;

public:
	RTCDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel) {
		this->dataChannel = dataChannel;
		dataChannel->RegisterObserver(this);
	}

	void close() {
		dataChannel->Close();
	}

	bool write(const char* buf, size_t size) {
		return dataChannel->Send(webrtc::DataBuffer(rtc::CopyOnWriteBuffer(buf, size),true));
	}

	size_t read(char* buf, size_t maxsize) {
		// TODO
		return 0;
	}

	void onData(const char* buf, size_t size) {
		// TODO
	}


	// DataChannelObserver implementation

	virtual void OnStateChange() { }

	virtual void OnMessage(const webrtc::DataBuffer& buffer){
		const char* buf = (const char*) buffer.data.data();
		onData(buf, buffer.data.size());
	}


};

}

#endif /* SRC_API_STREAM_RTCDATACHANNEL_H_ */
