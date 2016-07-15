/*
 * webrtc_helpers.h
 *
 *  Created on: 15 juil. 2016
 *      Author: jfellus
 */

#ifndef SRC_UTIL_WEBRTC_HELPERS_H_
#define SRC_UTIL_WEBRTC_HELPERS_H_


#include <webrtc/api/mediaconstraintsinterface.h>




////////////////////
// HELPER CLASSES //
////////////////////

class SimpleConstraints : public webrtc::MediaConstraintsInterface {
	public:
		SimpleConstraints() {}
		virtual ~SimpleConstraints() {}
		virtual const Constraints& GetMandatory() const { return mandatory_; }
		virtual const Constraints& GetOptional() const { return optional_; }
		template <class T> void AddMandatory(const char* key, const T& value) { mandatory_.push_back(Constraint(key, rtc::ToString<T>(value))); }
		template <class T> void AddOptional(const char* key, const T& value) { optional_.push_back(Constraint(key, rtc::ToString<T>(value))); }
	private:
		Constraints mandatory_;
		Constraints optional_;
};




class DummySetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver {
	public:
		static DummySetSessionDescriptionObserver* Create(int verbose) {
			return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>(verbose);
		}
		virtual void OnSuccess() {if(verbose) std::cout <<"SetSessionDescription Succeeded"<<std::endl;}
		virtual void OnFailure(const std::string& error) {if(verbose) std::cout<<"SetSessionDescription Failed"<<std::endl;}

	protected:
		DummySetSessionDescriptionObserver(int verbose) : verbose(verbose){}
		~DummySetSessionDescriptionObserver() {}
		int verbose;
};



#endif /* SRC_UTIL_WEBRTC_HELPERS_H_ */
