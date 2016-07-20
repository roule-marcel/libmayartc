
//////////////////
// WEBRTCPP API //
//////////////////


function webrtcpp_open(ip, channel, ondatachannel) {
	var rtc = new RTCSignaling(ip);
	rtc.addChannel(channel);
	rtc.ondatachannel = function(name, datachannel) { ondatachannel(datachannel); };
	rtc.realize();
}

function webrtcpp_open_video_out(ip, channel, stream) {
	var rtc = new RTCSignaling(ip);
	rtc.addVideoOut(channel, stream);
	rtc.realize();
}

function webrtcpp_open_video_in(ip, channel, onaddstream) {
	var rtc = new RTCSignaling(ip);
	rtc.addVideoIn(channel);
	rtc.onaddstream = function(name, stream) { onaddstream(stream); };
	rtc.realize();
}



////////////////////////////////////
// INTERNALS


const RTCPeerConnection = webkitRTCPeerConnection;


function RTCSignaling(ip) {
	var that = this;
	this.ws = new WebSocket("ws://"+ip, "webrtc-pp");
	this.rtc = null;
	this.requestedChannels = [];
	this.requestedStreams = [];
	this.streamLabels = [];
	this.streams = [];

	this.ws.onopen = function() {
		that.onopen();
		that.rtc = new RTCPeerConnection(null);

		that.rtc.onicecandidate = function(e) {
			if(!e.candidate) return;
			that.ws.send(JSON.stringify(e.candidate));
		};

		that.rtc.ondatachannel = function(event) {
			that.ondatachannel(event.channel.name, event.channel);
		};
		that.rtc.onaddstream = function(event) {
			that.onaddstream(event.stream.id, event.stream);
		};

		if(that._torealize) that.realize();
	};

	this.ws.onclose = function() { that.onclose(); };

	this.ws.onmessage = function(msg) {
		var data = JSON.parse(msg.data);
	//	console.log(data);
		if(data.sdpPartial) {
			if(!that._sdpData) that._sdpData = {type: data.type, sdp:""};
			that._sdpData.sdp += data.sdpPartial;
		} else if(data.endSdp) {
			that.rtc.setRemoteDescription(new RTCSessionDescription(that._sdpData), function(){
				that._ready = true; that._createAnswerWhenReady();
			}, function(e){console.error(e);});
		} else if(data.sdp) {
			that.rtc.setRemoteDescription(new RTCSessionDescription(data), function(){
				that._ready = true; that._createAnswerWhenReady();
			}, function(){});
		} else {
			that.rtc.addIceCandidate(new RTCIceCandidate(data), function() {}, function() {});
		}
	};
}

RTCSignaling.prototype.addVideoIn = function(name) {
	this.requestedStreams.push(name);
}

RTCSignaling.prototype.addChannel = function(name) {
	this.requestedChannels.push(name);
}

RTCSignaling.prototype.addVideoOut = function(name, stream) {
	this.streamLabels.push({streamId:stream.id, name:name});
	this.streams.push(stream);
}

RTCSignaling.prototype.onopen = function() {
	$("#open").innerHTML = "OPEN";
}

RTCSignaling.prototype.onclose = function() {
	$("#open").innerHTML = "CLOSED";
}

RTCSignaling.prototype.realize = function() {
	var that = this;
	if(this._realized) throw "AlreadyRealized";
	if(this.ws.readyState != 1) return this._torealize = true;
	this._torealize = false;
	this._realized = true;

	if(this.requestedStreams) this.ws.send(JSON.stringify({streams: this.requestedStreams}));
	if(this.requestedChannels) this.ws.send(JSON.stringify({channels: this.requestedChannels}));
	this.streamLabels.forEach(function(sl) { that.ws.send(JSON.stringify(sl)); });
	this.streams.forEach(function(s) { that.rtc.addStream(s); });

	this.ws.send("realize");

	this._createAnswerWhenReady();
}

RTCSignaling.prototype.ondatachannel = function(name, channel) { /* TO OVERRIDE */ }
RTCSignaling.prototype.onaddstream = function(name, stream) { /* TO OVERRIDE */ }


RTCSignaling.prototype._createAnswerWhenReady = function() {
	var that = this;
	if(!this._ready || !this._realized) return;

	that.rtc.createAnswer(function(sdp) {
		that.rtc.setLocalDescription(sdp);
		that.ws.send(JSON.stringify(sdp));
	}, function() {});
}
