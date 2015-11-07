/*
 * MayaSignaling.hpp
 *
 *  Created on: 1 août 2014
 *      Author: sylvain
 */

#ifndef MAYASIGNALING_HPP_
#define MAYASIGNALING_HPP_

#include "RTCSignaling.hpp"


namespace maya{

class MayaSignalingInterface : public RTCSignalingChannel{
	public:
		static MayaSignalingInterface *create();
};

}


#endif /* MAYASIGNALING_HPP_ */
