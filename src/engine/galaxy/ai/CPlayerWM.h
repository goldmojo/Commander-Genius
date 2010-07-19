/*
 * CPlayer.h
 *
 *  Created on: 14.07.2010
 *      Author: gerstrong
 */

#ifndef CPLAYERWM_H_
#define CPLAYERWM_H_

#include "../../../common/CObject.h"

namespace galaxy {

class CPlayerWM : public CObject {
public:
	CPlayerWM(CMap *pmap, Uint32 x, Uint32 y);
	void process();
	void processWalking();

	void finishlevel(Uint16 object);

	void performWalkingAnimation(bool walking);

	virtual ~CPlayerWM();
private:
	Uint16 m_basesprite;
	direction_t m_looking_dir;

	Uint8 m_animation;
	Uint8 m_animation_time;
	Uint8 m_animation_ticker;
};

}

#endif /* CPLAYERWM_H_ */
