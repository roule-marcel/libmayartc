/*
 * RTCChannel.hpp
 *
 *  Created on: 6 août 2014
 *      Author: sylvain
 */

#ifndef RTCCHANNEL_HPP_
#define RTCCHANNEL_HPP_

#include <webrtc/api/datachannelinterface.h>
#include <webrtc/api/mediastreaminterface.h>



#include "../api/RTCChannelInterface.hpp"

namespace webrtcpp {

class MemoryRenderer;

class RTCChannel : public RTCChannelInterface, public webrtc::DataChannelObserver, public FrameObserver {

	private:
		webrtc::DataChannelInterface *channel;
		webrtc::DataChannelInterface *nextChannel;
		char *name;
		int reliable;
		ReceiveCallback recv_cb;
		void * recv_cb_data;
		StreamCallback stream_cb;
		void* stream_cb_data;
		char *negociationMessage;
		int negociationMessageSize;
		bool negociated;
		MemoryRenderer* memoryRenderer;

	public:

		RTCChannel(char * name, int reliable);
		virtual ~RTCChannel();

		char* getName();

		int isReliable();

		void unsetDataChannel();

		void setDataChannel(webrtc::DataChannelInterface *channel);
		void setStream(webrtc::MediaStreamInterface *stream);

		virtual void OnStateChange();
		virtual void OnMessage(const webrtc::DataBuffer& buffer);

		// A frame was successfully received from a Stream
		virtual void onFrame(unsigned char* argb, int w, int h);

		virtual bool isConnected();
		virtual void sendData(const char* buffer, int bufferSize);
		virtual void setNegociationMessage(char * buffer, int bufferSize);
		virtual void registerReceiveCallback(ReceiveCallback cb, void * userData);
		virtual void registerStreamCallback(StreamCallback cb, void * userData);

		virtual void close();


	private:
		void doSetDataChannel(webrtc::DataChannelInterface *channel);
};

}

#endif /* RTCCHANNEL_HPP_ */
