/*
 * RTCDataChannel.h
 *
 *  Created on: 15 juil. 2016
 *      Author: jfellus
 */

#ifndef SRC_API_STREAM_RTCDATACHANNEL_H_
#define SRC_API_STREAM_RTCDATACHANNEL_H_

#include <vector>
#include <iostream>
#include <webrtc/api/datachannelinterface.h>

namespace webrtcpp {

class RTCDataChannel : public webrtc::DataChannelObserver {
public:

	typedef void(*Callback)(const char* buf, size_t size);

	std::string name;
	std::vector<rtc::scoped_refptr<webrtc::DataChannelInterface>> channels;
	std::vector<Callback> callbacks;

public:

	RTCDataChannel(const char* name) {
		this->name = name;
	}

	void addCallback(Callback cb) { callbacks.push_back(cb); }

	void add(rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel) {
		channels.push_back(dataChannel);
		dataChannel->RegisterObserver(this);
	}

	void close() {
		for(auto ch : channels) ch->Close();
	}

	bool write(const char* buf, size_t size) {
		webrtc::DataBuffer dataBuffer(rtc::CopyOnWriteBuffer(buf, size),true);
		for(auto ch : channels) ch->Send(dataBuffer);
		return true;
	}


	// DataChannelObserver implementation

	virtual void OnStateChange() { }

	virtual void OnMessage(const webrtc::DataBuffer& buffer){
		const char* buf = (const char*) buffer.data.data();
		onData(buf, buffer.data.size());
	}


protected:

	void onData(const char* buf, size_t size) {
		for(auto cb : callbacks) cb(buf, size);
	}


};

}

#endif /* SRC_API_STREAM_RTCDATACHANNEL_H_ */
